#include "Character/BSBaseCharacter.h"
#include "GAS/BSAbilitySystemComponent.h"
#include "GAS/BSAbilitySet.h"
#include "GAS/BSBaseAttributeSet.h"
#include "GameplayAbilitySpec.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "UI/FloatingDamageActor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Utility/BSGameplayTags.h"
#include "Component/BSHitReactionComponent.h"
#include "Component/BSWeaponComponent.h"
#include "Data/BSWeaponMotionSet.h"

ABSBaseCharacter::ABSBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UBSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UBSBaseAttributeSet>(TEXT("AttributeSet"));
	
	HitReactionComponent = CreateDefaultSubobject<UBSHitReactionComponent>(TEXT("HitReactionComponent"));
}

void ABSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		if (HasAuthority())
		{
			for (const UBSAbilitySet* AbilitySet : DefaultAbilitySets)
				if (AbilitySet)
					AbilitySet->GiveToAbilitySystem(AbilitySystemComponent);

			for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
				if (AbilityClass)
					AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		}
	}
}

void ABSBaseCharacter::UpdateTurning(float DeltaTime)
{
	if (!bIsTurning)
		return;
	
	const FRotator NewRotation = FMath::RInterpConstantTo(
	GetActorRotation(),
	TurnTargetRotation,
	DeltaTime,
	CurrentTurnSpeed);
	SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
	
	const float RemainingAngle = FMath::Abs(
	FMath::FindDeltaAngleDegrees(
		NewRotation.Yaw,
		TurnTargetRotation.Yaw));
	if (RemainingAngle <= TurnTolerance)
	{
		SetActorRotation(TurnTargetRotation);
		StopTurning();
	}
}

void ABSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTurning(DeltaTime);

}

void ABSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABSBaseCharacter::StartTurning(const FRotator& TargetRotation, float TurnSpeed)
{
	TurnTargetRotation = FRotator(0.0f, TargetRotation.Yaw, 0.0f);
	CurrentTurnSpeed = TurnSpeed;

	if (CurrentTurnSpeed <= 0.0f)
	{
		SetActorRotation(TurnTargetRotation);
		bIsTurning = false;
		return;
	}
	bIsTurning = true;
}

void ABSBaseCharacter::StopTurning()
{
	bIsTurning = false;
}

void ABSBaseCharacter::NotifyDeathAnimationFinished()
{
	UE_LOG(LogTemp, Log, TEXT("NotifyDeathAnimationFinished 확인로그"));
	if (!bIsDead) // 죽은척 방지
	{
		return;
	}
	FinishDeath();
}

UAbilitySystemComponent* ABSBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

UBSWeaponComponent* ABSBaseCharacter::GetWeaponComponent() const
{
	return FindComponentByClass<UBSWeaponComponent>();
}

FBSWeaponMovementSet ABSBaseCharacter::GetCurrentMovementSet() const
{
	if (const UBSWeaponComponent* WeaponComponent = GetWeaponComponent())
	{
		if (const UBSWeaponMotionSet* MotionSet = WeaponComponent->GetMotionSet())
		{
			return MotionSet->MovementSet;
		}
	}
	return DefaultMovementSet;
}

FAbilitySkillData ABSBaseCharacter::GetSkillDataForAbility(FGameplayTag AbilityTag)
{
	if (const UBSWeaponComponent* WeaponComponent = GetWeaponComponent())
	{
		if (const UBSWeaponMotionSet* MotionSet = WeaponComponent->GetMotionSet())
		{
			if (const FAbilitySkillData* WeaponSkillData = MotionSet->SkillDataMap.Find(AbilityTag))
			{
				return *WeaponSkillData;
			}
		}
	}
	// 무기가 없거나 무기 전용 모션이 없으면 캐릭터 기본값
	if (const FAbilitySkillData* CharacterSkillData = AbilitySkillDataMap.Find(AbilityTag))
	{
		return *CharacterSkillData;
	}
	return FAbilitySkillData();
}

void ABSBaseCharacter::SetProjectileData(const FProjectileData& Data)
{
	CachedProjectileData = Data;
}

FProjectileData ABSBaseCharacter::GetProjectileData() const
{
	return CachedProjectileData;
}

void ABSBaseCharacter::SetMeleeTraceData(const FMeleeTraceData& Data)
{
	CachedMeleeTraceData = Data;
}

FMeleeTraceData ABSBaseCharacter::GetMeleeTraceData() const
{
	return CachedMeleeTraceData;
}

void ABSBaseCharacter::SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical)
{
	if (!FloatingDamageActorClass || Amount <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, CapsuleHalfHeight);

	SpawnLocation.X += FMath::RandRange(-FloatingDamageRandomRadius,FloatingDamageRandomRadius);

	SpawnLocation.Y += FMath::RandRange(-FloatingDamageRandomRadius,FloatingDamageRandomRadius);

	AFloatingDamageActor* DamageActor = World->SpawnActor<AFloatingDamageActor>(
			FloatingDamageActorClass, SpawnLocation,FRotator::ZeroRotator);

	if (DamageActor)
	{
		DamageActor->Initialize(Amount, bIsHeal, bIsCritical);
	}
}

void ABSBaseCharacter::Death(AActor* Killer)
{
	if (bIsDead)
		return;
	bIsDead = true;
	bDeathFinished = false;
	DeathKiller = Killer;
	
	AbilitySystemComponent->AddLooseGameplayTag(BSGameplayTags::State_Dead);
	OnDeathStarted(DeathKiller.Get());
	DisableCharacterOnDeath();
	PlayDeathMontage();
}

void ABSBaseCharacter::ApplyHitReaction(AActor* Attacker, const FHitResult& HitResult,
	const FBSHitReactionData& ReactionData)
{
	if (HitReactionComponent)
	{
		HitReactionComponent->ApplyHitReaction(Attacker, HitResult, ReactionData);
	}
}

FGenericTeamId ABSBaseCharacter::GetGenericTeamId() const
{
	const IGenericTeamAgentInterface* ControllerTeam = Cast<IGenericTeamAgentInterface>(GetController());
	return ControllerTeam ? ControllerTeam->GetGenericTeamId() : FGenericTeamId::NoTeam;
}

void ABSBaseCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	IGenericTeamAgentInterface* ControllerTeam = Cast<IGenericTeamAgentInterface>(GetController());
	if (ControllerTeam)
	{
		ControllerTeam->SetGenericTeamId(NewTeamID);
	}
}

void ABSBaseCharacter::DisableCharacterOnDeath()
{
	StopTurning();

	// 현재 실행 중인 공격/스킬 종료
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}

	// 이동 중지
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	// AI나 플레이어 컨트롤러가 이동 명령을 내리고 있었다면 중단
	if (Controller)
	{
		Controller->StopMovement();
	}
}


void ABSBaseCharacter::PlayDeathMontage()
{
	UAnimMontage* SelectedMontage = SelectDeathMontage(DeathKiller.Get());
	if (!SelectedMontage)
	{
		FinishDeath();
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		FinishDeath();
		return;
	}

	// 다른 몽타주 재생중지
	AnimInstance->StopAllMontages(0.1f);

	const float Duration = AnimInstance->Montage_Play(SelectedMontage);

	if (Duration <= 0.0f)
	{
		FinishDeath();
		return;
	}
}


void ABSBaseCharacter::FinishDeath()
{
	if (bDeathFinished)
		return;
	bDeathFinished = true;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OnDeathFinished(DeathKiller.Get());
}

UAnimMontage* ABSBaseCharacter::SelectDeathMontage(AActor* Killer) const
{
	return DeathMontage;
}

void ABSBaseCharacter::OnDeathStarted(AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("OnDeathStarted 확인로그. Killer: %s"), *GetNameSafe(Killer));
}

void ABSBaseCharacter::OnDeathFinished(AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("OnDeathFinished 확인로그. Killer: %s"), *GetNameSafe(Killer));
}
