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

ABSBaseCharacter::ABSBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UBSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UBSBaseAttributeSet>(TEXT("AttributeSet"));
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

FAbilitySkillData ABSBaseCharacter::GetSkillDataForAbility(FGameplayTag AbilityTag)
{
	if (const FAbilitySkillData* FoundData = AbilitySkillDataMap.Find(AbilityTag))
	{
		return *FoundData;
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
	
	OnDeathStarted(DeathKiller.Get());
	DisableCharacterOnDeath();
	PlayDeathMontage();
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
