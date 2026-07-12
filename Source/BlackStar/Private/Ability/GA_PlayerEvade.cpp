// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GA_PlayerEvade.h"
#include "Character/BSPlayerCharacter.h"
#include "GAS/BSBaseAttributeSet.h"
#include "Utility/BSGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/RootMotionSource.h"
#include "GameFramework/CharacterMovementComponent.h"


UGA_PlayerEvade::UGA_PlayerEvade()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AbilityTag = BSGameplayTags::Ability_Player_Evade;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(AbilityTag);
	SetAssetTags(AssetTags);
	
	ActivationOwnedTags.AddTag(BSGameplayTags::State_Evading);
	
	ActivationBlockedTags.AddTag(BSGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(BSGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(BSGameplayTags::State_Evading);
}

bool UGA_PlayerEvade::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                   FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	return HasEnoughStamina(ActorInfo);
}


void UGA_PlayerEvade::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	bSpentStamina = false;
	EvadeRootMotionSourceID = 0;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ABSPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABSPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!PlayerCharacter || !HasEnoughStamina(ActorInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAnimMontage* EvadeMontage = PlayerCharacter->GetEvadeMontage();
	if (!EvadeMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const FVector EvadeMoveDirection = GetEvadeMoveDirection(PlayerCharacter);
	if (EvadeMoveDirection.IsNearlyZero())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const EPlayerEvadeDirection AnimDirection = GetEvadeAnimDirection(PlayerCharacter);
	const FName SectionName = GetEvadeSectionName(AnimDirection);
	
	ApplyStaminaCost(Handle, ActorInfo, ActivationInfo);
	bSpentStamina = true;

	FaceEvadeDirectionIfNeeded(PlayerCharacter, EvadeMoveDirection);
	StartEvadeMovement(PlayerCharacter, EvadeMoveDirection);

	const float MontagePlayRate = GetMontagePlayRateForDuration(EvadeMontage, SectionName);
	// 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			EvadeMontage,
			MontagePlayRate,
			SectionName,
			false);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGA_PlayerEvade::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_PlayerEvade::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_PlayerEvade::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_PlayerEvade::OnMontageInterrupted);
		MontageTask->ReadyForActivation();
	}
	
		UE_LOG(LogTemp, Warning, TEXT("Evade Section: %s, Index: %d, PlayRate: %.2f"),
    		*SectionName.ToString(),
    		EvadeMontage->GetSectionIndex(SectionName),
    		MontagePlayRate);
    	// if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    	// {
    	// 	ActorInfo->AbilitySystemComponent->CurrentMontageJumpToSection(SectionName);
    	// }
    	
	const float AbilityEndTime = EvadeDuration * EvadeControlLockRatio;
	
	if (UWorld* World = PlayerCharacter->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			EvadeTimerHandle,
			this,
			&UGA_PlayerEvade::FinishEvade,
			AbilityEndTime,
			false);
	}
}

void UGA_PlayerEvade::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ABSPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABSPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	float BlendOutTime = 0.08f;
	if (PlayerCharacter)
	{
		if (UWorld* World = PlayerCharacter->GetWorld())
		{
			World->GetTimerManager().ClearTimer(EvadeTimerHandle);
		}

		StopEvadeMovement(PlayerCharacter);
		BlendOutTime = PlayerCharacter->GetEvadeMontageBlendOutTime();

		if (bSpentStamina)
		{
			PlayerCharacter->ApplyStaminaRegenDelay();
		}
	}

	if (bWasCancelled && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		UAnimMontage* PlayingMontage = ActorInfo->AbilitySystemComponent->GetCurrentMontage();
		UAnimMontage* EvadeMontage = PlayerCharacter ? PlayerCharacter->GetEvadeMontage() : nullptr;

		if (PlayingMontage && PlayingMontage == EvadeMontage)
		{
			ActorInfo->AbilitySystemComponent->CurrentMontageStop(BlendOutTime);
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// GetCurrentMoveInput이 플레이어 캐릭터에만 있슴!
FVector UGA_PlayerEvade::GetEvadeMoveDirection(const ABSPlayerCharacter* PlayerCharacter) const
{
	if (!PlayerCharacter)
	{
		return FVector::ZeroVector;
	}

	const FVector2D MoveInput = PlayerCharacter->GetCurrentMoveInput();

	if (MoveInput.IsNearlyZero())
	{
		return -PlayerCharacter->GetActorForwardVector();
	}

	const bool bLockedOn = PlayerCharacter->GetCombatTarget() != nullptr;

	if (bLockedOn)
	{
		const FVector Forward = PlayerCharacter->GetActorForwardVector();
		const FVector Right = PlayerCharacter->GetActorRightVector();
		return (Forward * MoveInput.Y + Right * MoveInput.X).GetSafeNormal2D();
	}

	const FRotator ControlRotation = PlayerCharacter->GetController()
		? PlayerCharacter->GetController()->GetControlRotation() : PlayerCharacter->GetActorRotation();

	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	return (Forward * MoveInput.Y + Right * MoveInput.X).GetSafeNormal2D();
}

EPlayerEvadeDirection UGA_PlayerEvade::GetEvadeAnimDirection(const class ABSPlayerCharacter* PlayerCharacter) const
{
	if (!PlayerCharacter)
	{
		return EPlayerEvadeDirection::Back;
	}

	const FVector2D MoveInput = PlayerCharacter->GetCurrentMoveInput();

	if (MoveInput.IsNearlyZero())
	{
		return EPlayerEvadeDirection::Back;
	}

	const bool bLockedOn = PlayerCharacter->GetCombatTarget() != nullptr;

	if (!bLockedOn)
	{
		return EPlayerEvadeDirection::Front;
	}

	if (FMath::Abs(MoveInput.X) > FMath::Abs(MoveInput.Y))
	{
		return MoveInput.X > 0.0f
			? EPlayerEvadeDirection::Right : EPlayerEvadeDirection::Left;
	}

	return MoveInput.Y > 0.0f
		? EPlayerEvadeDirection::Front : EPlayerEvadeDirection::Back;
}

FName UGA_PlayerEvade::GetEvadeSectionName(EPlayerEvadeDirection Direction) const
{
	switch (Direction)
	{
	case EPlayerEvadeDirection::Front:
		return TEXT("Front");
	case EPlayerEvadeDirection::Back:
		return TEXT("Back");
	case EPlayerEvadeDirection::Left:
		return TEXT("Left");
	case EPlayerEvadeDirection::Right:
		return TEXT("Right");
	default:
		return TEXT("Back");
	}
}

float UGA_PlayerEvade::GetMontagePlayRateForDuration(UAnimMontage* Montage, FName SectionName) const
{
	if (!Montage || EvadeDuration <= KINDA_SMALL_NUMBER)
	{
		return 1.0f;
	}

	const int32 SectionIndex = Montage->GetSectionIndex(SectionName);
	if (SectionIndex == INDEX_NONE)
	{
		return 1.0f;
	}

	const float SectionLength = Montage->GetSectionLength(SectionIndex);
	if (SectionLength <= KINDA_SMALL_NUMBER)
	{
		return 1.0f;
	}

	return SectionLength / EvadeDuration;
}

void UGA_PlayerEvade::FaceEvadeDirectionIfNeeded(class ABSPlayerCharacter* PlayerCharacter,const FVector& Direction) const
{
	if (!PlayerCharacter || PlayerCharacter->GetCombatTarget())
	{
		return;
	}

	if (PlayerCharacter->GetCurrentMoveInput().IsNearlyZero())
	{
		return;
	}
	
	if (!Direction.IsNearlyZero())
	{
		PlayerCharacter->SetActorRotation(Direction.Rotation());
	}
}

void UGA_PlayerEvade::StartEvadeMovement(ACharacter* Character, const FVector& Direction)
{
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	const FVector MoveDirection = Direction.GetSafeNormal2D();
	const FVector StartLocation = Character->GetActorLocation();
	const FVector TargetLocation = StartLocation + MoveDirection * EvadeDistance;

	TSharedPtr<FRootMotionSource_MoveToDynamicForce> MoveSource = MakeShared<FRootMotionSource_MoveToDynamicForce>();
	MoveSource->InstanceName = TEXT("PlayerEvade");
	MoveSource->AccumulateMode = ERootMotionAccumulateMode::Override;
	MoveSource->Priority = 500;
	MoveSource->Duration = EvadeDuration;
	MoveSource->StartLocation = StartLocation;
	MoveSource->TargetLocation = TargetLocation;
	MoveSource->bRestrictSpeedToExpected = true;
	
	const bool bIsInAir = MovementComponent->IsFalling();
	if (!bIsInAir)
	{
		MoveSource->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
	}

	if (EvadeMovementCurve)
	{
		MoveSource->TimeMappingCurve = EvadeMovementCurve;
	}

	EvadeRootMotionSourceID = MovementComponent->ApplyRootMotionSource(MoveSource);
}

void UGA_PlayerEvade::StopEvadeMovement(ACharacter* Character)
{
	if (!Character || EvadeRootMotionSourceID == 0)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->RemoveRootMotionSourceByID(EvadeRootMotionSourceID);
	}

	EvadeRootMotionSourceID = 0;
}

float UGA_PlayerEvade::GetStaminaCost(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const ABSPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABSPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;

	// TODO: 캐릭터에 넣고 버프/디버프 배율에 적용
	const float Multiplier = 1.0f;
	return BaseStaminaCost * Multiplier;
}

bool UGA_PlayerEvade::HasEnoughStamina(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const UBSBaseAttributeSet* AttributeSet = ASC ? ASC->GetSet<UBSBaseAttributeSet>() : nullptr;
	return AttributeSet && AttributeSet->GetCurrentStamina() >= GetStaminaCost(ActorInfo);
}

void UGA_PlayerEvade::ApplyStaminaCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC || !StaminaCostEffect)
	{
		return;
	}

	FGameplayEffectSpecHandle CostSpecHandle = MakeOutgoingGameplayEffectSpec(StaminaCostEffect, GetAbilityLevel(Handle, ActorInfo));
	if (!CostSpecHandle.IsValid())
	{
		return;
	}

	const float Cost = GetStaminaCost(ActorInfo);
	CostSpecHandle.Data->SetSetByCallerMagnitude(BSGameplayTags::Data_StaminaCost, -Cost);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CostSpecHandle);
}

void UGA_PlayerEvade::FinishEvade()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_PlayerEvade::OnMontageCompleted()
{

}

void UGA_PlayerEvade::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}


