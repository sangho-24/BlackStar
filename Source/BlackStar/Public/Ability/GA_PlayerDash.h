// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlayerDash.generated.h"

/**
 * 
 */
UCLASS()
class BLACKSTAR_API UGA_PlayerDash : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade|Setup")
	FGameplayTag AbilityTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Cost")
	TSubclassOf<UGameplayEffect> StaminaCostEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Cost", meta = (ClampMin = "0.0"))
	float MinActivationStamina = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Cost", meta = (ClampMin = "0.0"))
	float StaminaDrainPerSecond = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Cost", meta = (ClampMin = "0.016"))
	float DrainTickRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Movement", meta = (ClampMin = "0.0"))
	float DashWalkSpeed = 1000.0f;

	FTimerHandle DrainTimerHandle;
	float OriginalWalkSpeed = 0.0f;
	bool bDashMovementActive = false;
	
private:
	bool bSpentStamina = false;
public:
	UGA_PlayerDash();
	
protected:
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	float GetCurrentStamina(const FGameplayAbilityActorInfo* ActorInfo) const;
	bool HasMoveInput(const FGameplayAbilityActorInfo* ActorInfo) const;

	void TickDash();
	void StartDashMovement();
	void StopDashMovement();

	void ApplyStaminaDrain(float DrainAmount);
};
