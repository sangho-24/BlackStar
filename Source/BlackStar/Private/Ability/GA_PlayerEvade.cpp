// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GA_PlayerEvade.h"
#include "Character/BSPlayerCharacter.h"
#include "GAS/BSBaseAttributeSet.h"
#include "Utility/BSGameplayTags.h"


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
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!Character || !HasEnoughStamina(ActorInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	ApplyStaminaCost(Handle, ActorInfo, ActivationInfo);
	bSpentStamina = true;
	
	const FVector EvadeDirection = GetEvadeDirection(Character);
	if (EvadeDirection.IsNearlyZero())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Character->LaunchCharacter(EvadeDirection * EvadeStrength, true, true);

	if (UWorld* World = Character->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			EvadeTimerHandle,
			this,
			&UGA_PlayerEvade::FinishEvade,
			EvadeDuration,
			false);
	}
}

void UGA_PlayerEvade::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(EvadeTimerHandle);
		}
		
		if (ABSPlayerCharacter* PlayerCharacter = Cast<ABSPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (bSpentStamina)
			{
			PlayerCharacter->ApplyStaminaRegenDelay();
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// GetCurrentMoveInput이 플레이어 캐릭터에만 있슴!
FVector UGA_PlayerEvade::GetEvadeDirection(ACharacter* Character) const
{
	const ABSPlayerCharacter* PlayerCharacter = Cast<ABSPlayerCharacter>(Character);
	const FVector2D MoveInput = PlayerCharacter ? PlayerCharacter->GetCurrentMoveInput() : FVector2D::ZeroVector;

	if (MoveInput.IsNearlyZero())
	{
		return -Character->GetActorForwardVector();
	}

	const FRotator ControlRotation = Character->GetController()
		? Character->GetController()->GetControlRotation()
		: Character->GetActorRotation();

	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	return (ForwardDirection * MoveInput.Y + RightDirection * MoveInput.X).GetSafeNormal2D();
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

// void UGA_PlayerEvade::OnMontageCompleted()
// {
// }
//
// void UGA_PlayerEvade::OnMontageCancelled()
// {
// }
