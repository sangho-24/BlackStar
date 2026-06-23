#include "Ability/GA_AttackBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Actor/BSProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GAS/BSBaseAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/IAbilityAnimationInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/BSGameplayTags.h"

UGA_AttackBase::UGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationOwnedTags.AddTag(BSGameplayTags::State_Attacking);
}

void UGA_AttackBase::PostInitProperties()
{
	Super::PostInitProperties();

	if (AbilityTag.IsValid())
	{
		FGameplayTagContainer AssetTags;
		AssetTags.AddTag(AbilityTag);
		SetAssetTags(AssetTags);
	}
}

void UGA_AttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!StartAttack(TriggerEventData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

bool UGA_AttackBase::StartAttack(const FGameplayEventData* TriggerEventData)
{
	if (!CurrentActorInfo)
	{
		return false;
	}

	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(AvatarActor);
	if (!AvatarActor || !AnimChar)
	{
		return false;
	}

	const FAbilitySkillData SkillData = AnimChar->GetSkillDataForAbility(AbilityTag);
	if (!SkillData.Montage)
	{
		return false;
	}

	AttackTarget = ResolveAttackTarget();
	if (AttackTarget)
	{
		FaceTarget(AttackTarget);
	}
	else
	{
		FaceAttackDirection();
	}
	HitActors.Empty();
	PlayMontage(SkillData.Montage, SkillData.StartSection);
	RegisterCommonEventTasks();

	return true;
}

void UGA_AttackBase::RegisterCommonEventTasks()
{
	UAbilityTask_WaitGameplayEvent* SpawnTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BSGameplayTags::Event_SpawnProjectile);
	SpawnTask->EventReceived.AddDynamic(this, &UGA_AttackBase::OnSpawnProjectile);
	SpawnTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* MeleeStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BSGameplayTags::Event_MeleeTrace_Start, nullptr, false, true);
	MeleeStartTask->EventReceived.AddDynamic(this, &UGA_AttackBase::OnMeleeTraceStart);
	MeleeStartTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* MeleeEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BSGameplayTags::Event_MeleeTrace_End, nullptr, false, true);
	MeleeEndTask->EventReceived.AddDynamic(this, &UGA_AttackBase::OnMeleeTraceEnd);
	MeleeEndTask->ReadyForActivation();
}

void UGA_AttackBase::PlayMontage(UAnimMontage* Montage, FName StartSection)
{
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.0f, StartSection);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_AttackBase::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_AttackBase::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_AttackBase::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_AttackBase::StartMeleeTrace()
{
	UE_LOG(LogTemp, Log, TEXT("Starting Melee Trace"));
	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	UWorld* World = CurrentActorInfo->AvatarActor->GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(TraceTimerHandle);
	World->GetTimerManager().SetTimer(TraceTimerHandle, this, &UGA_AttackBase::DoMeleeTrace, TraceTickRate, true, 0.0f);
}

void UGA_AttackBase::StopMeleeTrace()
{
	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	if (UWorld* World = CurrentActorInfo->AvatarActor->GetWorld())
	{
		World->GetTimerManager().ClearTimer(TraceTimerHandle);
	}
}

void UGA_AttackBase::DoMeleeTrace()
{
	if (!CurrentActorInfo)
	{
		return;
	}

	AActor* Avatar = CurrentActorInfo->AvatarActor.Get();
	if (!Avatar)
	{
		return;
	}
	USkeletalMeshComponent* Mesh = Avatar->FindComponentByClass<USkeletalMeshComponent>();
	UWorld* World = Avatar->GetWorld();
	if (!Mesh || !World)
	{
		return;
	}

	FVector TraceStart = Avatar->GetActorLocation();
	FVector TraceEnd = Avatar->GetActorLocation();
	
	if (!ActiveTraceData.StartSocketName.IsNone() && Mesh->DoesSocketExist(ActiveTraceData.StartSocketName))
	{
		TraceStart = Mesh->GetSocketLocation(ActiveTraceData.StartSocketName);
	}
	if (!ActiveTraceData.EndSocketName.IsNone() && Mesh->DoesSocketExist(ActiveTraceData.EndSocketName))
	{
		TraceEnd = Mesh->GetSocketLocation(ActiveTraceData.EndSocketName);
	}
	if (ActiveTraceData.ExtraLength > 0.0f)
	{
		FVector Direction = TraceEnd - TraceStart;
		Direction = Direction.IsNearlyZero() ? Avatar->GetActorForwardVector() : Direction.GetSafeNormal();
		TraceEnd += Direction * ActiveTraceData.ExtraLength;
	}
	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);

	const bool bHit = World->SweepMultiByChannel(
		Hits,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		DamageTraceChannel,
		FCollisionShape::MakeSphere(ActiveTraceData.TraceRadius),
		Params);

	if (bDrawDebugTrace)
	{
		const FColor Color = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(World, TraceStart, TraceEnd, Color, false, TraceTickRate * 2.0f, 0, 2.0f);
		DrawDebugSphere(World, TraceStart, ActiveTraceData.TraceRadius, 8, Color, false, TraceTickRate * 2.0f);
		DrawDebugSphere(World, TraceEnd, ActiveTraceData.TraceRadius, 8, Color, false, TraceTickRate * 2.0f);
	}

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == Avatar)
		{
			continue;
		}

		if (HitActors.ContainsByPredicate([HitActor](const TWeakObjectPtr<AActor>& ExistingActor) { return ExistingActor.Get() == HitActor; }))
		{
			continue;
		}

		HitActors.Add(HitActor);
		ApplyMeleeDamage(HitActor, Hit);
	}
}

void UGA_AttackBase::ApplyMeleeDamage(AActor* TargetActor, const FHitResult& HitResult)
{
	if (!TargetActor || !MeleeDamageEffect || !CurrentActorInfo)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = CurrentActorInfo->AbilitySystemComponent.Get();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!SourceASC || !TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddHitResult(HitResult);
	ContextHandle.AddInstigator(CurrentActorInfo->AvatarActor.Get(), CurrentActorInfo->AvatarActor.Get());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(MeleeDamageEffect, 1.0f, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	float FinalDamage = MeleeBaseDamage;
	if (const UBSBaseAttributeSet* SourceAttributeSet = SourceASC->GetSet<UBSBaseAttributeSet>())
	{
		FinalDamage *= 1.0f + SourceAttributeSet->GetAttackPower() / 100.0f;
	}
	FinalDamage *= ActiveTraceData.DamageMultiplier;

	SpecHandle.Data->SetSetByCallerMagnitude(BSGameplayTags::Data_Damage, -FinalDamage);
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	if (ActiveTraceData.HitCueTag.IsValid())
	{
		FGameplayCueParameters CueParameters;
		CueParameters.Location = HitResult.ImpactPoint;
		CueParameters.Normal = HitResult.ImpactNormal;
		CueParameters.PhysicalMaterial = HitResult.PhysMaterial;
		CueParameters.EffectCauser = CurrentActorInfo->AvatarActor.Get();
		CueParameters.Instigator = CurrentActorInfo->AvatarActor.Get();
		CueParameters.SourceObject = TargetActor;
		SourceASC->ExecuteGameplayCue(ActiveTraceData.HitCueTag, CueParameters);
	}
}

void UGA_AttackBase::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_AttackBase::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_AttackBase::OnMeleeTraceStart(FGameplayEventData Payload)
{
	AActor* AvatarActor = CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;
	IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(AvatarActor);
	if (!AnimChar)
	{
		return;
	}

	ActiveTraceData = AnimChar->GetMeleeTraceData();
	HitActors.Empty();
	StartMeleeTrace();
}

void UGA_AttackBase::OnMeleeTraceEnd(FGameplayEventData Payload)
{
	StopMeleeTrace();
	HitActors.Empty();
}

void UGA_AttackBase::OnSpawnProjectile(FGameplayEventData Payload)
{
	AActor* AvatarActor = CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;
	IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(AvatarActor);
	if (!AvatarActor || !AnimChar)
	{
		return;
	}

	ActiveProjectileData = AnimChar->GetProjectileData();
	if (!ActiveProjectileData.ProjectileClass)
	{
		return;
	}

	FVector SpawnLocation = AvatarActor->GetActorLocation();
	FRotator SpawnRotation = AvatarActor->GetActorRotation();

	if (USkeletalMeshComponent* Mesh = AvatarActor->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (!ActiveProjectileData.SpawnSocketName.IsNone() && Mesh->DoesSocketExist(ActiveProjectileData.SpawnSocketName))
		{
			SpawnLocation = Mesh->GetSocketLocation(ActiveProjectileData.SpawnSocketName);
		}
	}

	AActor* Target = const_cast<AActor*>(Payload.Target.Get());
	if (!Target)
	{
		Target = AttackTarget;
	}
	if (Target)
	{
		SpawnRotation = (Target->GetActorLocation() - SpawnLocation).Rotation();
	}

	ABSProjectile* Projectile = AvatarActor->GetWorld()->SpawnActorDeferred<ABSProjectile>(
		ActiveProjectileData.ProjectileClass,
		FTransform(SpawnRotation, SpawnLocation),
		AvatarActor,
		Cast<APawn>(AvatarActor),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (Projectile)
	{
		Projectile->InitProjectile(CurrentActorInfo->AbilitySystemComponent.Get(), ActiveProjectileData.DamageMultiplier);
		UGameplayStatics::FinishSpawningActor(Projectile, FTransform(SpawnRotation, SpawnLocation));
	}
}

AActor* UGA_AttackBase::ResolveAttackTarget() const
{
	AActor* AvatarActor = CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;
	const IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(AvatarActor);
	if (!AnimChar)
	{
		return nullptr;
	}
	return AnimChar->GetCombatTarget();
}

void UGA_AttackBase::FaceTarget(AActor* TargetActor)
{
	ACharacter* Character = CurrentActorInfo ? Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	if (!Character || !TargetActor)
	{
		return;
	}

	FVector ToTarget = TargetActor->GetActorLocation() - Character->GetActorLocation();
	ToTarget.Z = 0.0f;
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	FRotator NewRotation = Character->GetActorRotation();
	NewRotation.Yaw = ToTarget.Rotation().Yaw;
	Character->SetActorRotation(NewRotation);
}

void UGA_AttackBase::FaceAttackDirection()
{
}


void UGA_AttackBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	StopMeleeTrace();
	HitActors.Empty();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
