// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlayerEvade.generated.h"

/**
 * 
 */
UCLASS()
class BLACKSTAR_API UGA_PlayerEvade : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_PlayerEvade();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade|Setup")
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade|Setup")
	TSubclassOf<UGameplayEffect> StaminaCostEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade|Cost")
	float BaseStaminaCost = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade|Movement")
	float EvadeStrength = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade|Movement")
	float EvadeDuration = 0.25f;
	
	
private:
	bool bSpentStamina = false;
	FTimerHandle EvadeTimerHandle;
	
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
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	
	FVector GetEvadeDirection(ACharacter* Character) const;
	float GetStaminaCost(const FGameplayAbilityActorInfo* ActorInfo) const;
	bool HasEnoughStamina(const FGameplayAbilityActorInfo* ActorInfo) const;
	void ApplyStaminaCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const;
	void FinishEvade();
	
	// UFUNCTION()
	// void OnMontageCompleted();
	//
	// UFUNCTION()
	// void OnMontageCancelled();
};
