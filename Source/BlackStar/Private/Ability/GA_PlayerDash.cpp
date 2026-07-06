// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GA_PlayerDash.h"
#include "Utility/BSGameplayTags.h"
#include "GAS/BSBaseAttributeSet.h"
#include "Character/BSPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


UGA_PlayerDash::UGA_PlayerDash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AbilityTag = BSGameplayTags::Ability_Player_Dash;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(AbilityTag);
	SetAssetTags(AssetTags);
	// ActivationOwnedTags.AddTag(BSGameplayTags::State_Dashing);
	ActivationBlockedTags.AddTag(BSGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(BSGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(BSGameplayTags::State_Evading);
}

bool UGA_PlayerDash::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return GetCurrentStamina(ActorInfo) >= MinActivationStamina;
}

void UGA_PlayerDash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bSpentStamina = false;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid() || GetCurrentStamina(ActorInfo) < MinActivationStamina)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DrainTimerHandle,
			this,
			&UGA_PlayerDash::TickDash,
			DrainTickRate,
			true,
			0.0f);
	}
}

void UGA_PlayerDash::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_PlayerDash::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(DrainTimerHandle);
		}
		if (ABSPlayerCharacter* PlayerCharacter = Cast<ABSPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (bSpentStamina)
			{
				PlayerCharacter->ApplyStaminaRegenDelay();
			}
		}
	}
	StopDashMovement();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

float UGA_PlayerDash::GetCurrentStamina(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const UBSBaseAttributeSet* AttributeSet = ASC ? ASC->GetSet<UBSBaseAttributeSet>() : nullptr;

	return AttributeSet ? AttributeSet->GetCurrentStamina() : 0.0f;
}

bool UGA_PlayerDash::HasMoveInput(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const ABSPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABSPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;

	if (!PlayerCharacter)
	{
		return false;
	}

	return !PlayerCharacter->GetCurrentMoveInput().IsNearlyZero();
}

void UGA_PlayerDash::TickDash()
{
	if (!CurrentActorInfo)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!HasMoveInput(CurrentActorInfo))
	{
		StopDashMovement();
		return;
	}

	const float CurrentStamina = GetCurrentStamina(CurrentActorInfo);
	if (CurrentStamina <= 0.0f)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	StartDashMovement();

	const float DrainAmount = StaminaDrainPerSecond * DrainTickRate;
	const float ActualDrainAmount = FMath::Min(CurrentStamina, DrainAmount);

	ApplyStaminaDrain(ActualDrainAmount);
	bSpentStamina = true;
}

void UGA_PlayerDash::StartDashMovement()
{
	if (bDashMovementActive || !CurrentActorInfo)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();

	if (!Movement || !ASC)
	{
		return;
	}

	OriginalWalkSpeed = Movement->MaxWalkSpeed;
	Movement->MaxWalkSpeed = DashWalkSpeed;

	ASC->AddLooseGameplayTag(BSGameplayTags::State_Dashing);

	bDashMovementActive = true;
}

void UGA_PlayerDash::StopDashMovement()
{
	if (!bDashMovementActive || !CurrentActorInfo)
	{
		return;
	}

	ABSPlayerCharacter* PlayerCharacter = Cast<ABSPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	UCharacterMovementComponent* Movement = PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr;
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	
	if (PlayerCharacter)
	{
		PlayerCharacter->ApplyStaminaRegenDelay();
	}
	
	if (Movement)
	{
		Movement->MaxWalkSpeed = OriginalWalkSpeed;
	}

	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(BSGameplayTags::State_Dashing);
	}

	bDashMovementActive = false;
}

void UGA_PlayerDash::ApplyStaminaDrain(float DrainAmount)
{
	UE_LOG(LogTemp, Log, TEXT("Dash drain: %.2f, Effect: %s"),
	DrainAmount,
	*GetNameSafe(StaminaCostEffect));
	if (!CurrentActorInfo || !StaminaCostEffect || DrainAmount <= 0.0f)
	{
		return;
	}

	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return;
	}

	FGameplayEffectSpecHandle CostSpecHandle = MakeOutgoingGameplayEffectSpec(StaminaCostEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));

	if (!CostSpecHandle.IsValid())
	{
		return;
	}

	CostSpecHandle.Data->SetSetByCallerMagnitude(BSGameplayTags::Data_StaminaCost, -DrainAmount);
	ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,CostSpecHandle);
	UE_LOG(LogTemp, Log, TEXT("Dash cost applied: %s"), CostSpecHandle.IsValid() ? TEXT("true") : TEXT("false"));
}