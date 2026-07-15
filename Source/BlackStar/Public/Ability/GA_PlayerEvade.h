// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlayerEvade.generated.h"

class UAnimMontage;

UENUM()
enum class EPlayerEvadeDirection : uint8
{
	Front,
	Back,
	Left,
	Right
};

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
	float EvadeDistance = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade|Movement")
	float EvadeDuration = 0.25f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade|Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EvadeControlLockRatio = 0.75f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade|Movement|Setup")
	TObjectPtr<UCurveFloat> EvadeMovementCurve;
	
private:
	bool bSpentStamina = false;
	FTimerHandle EvadeTimerHandle;
	uint16 EvadeRootMotionSourceID = 0;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveEvadeMontage;
	
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
	
	FVector GetEvadeMoveDirection(const class ABSPlayerCharacter* PlayerCharacter) const;
	EPlayerEvadeDirection GetEvadeAnimDirection(const class ABSPlayerCharacter* PlayerCharacter) const;
	FName GetEvadeSectionName(EPlayerEvadeDirection Direction) const;
	float GetMontagePlayRateForDuration(UAnimMontage* Montage, FName SectionName) const;
	
	void FaceEvadeDirectionIfNeeded(class ABSPlayerCharacter* PlayerCharacter, const FVector& Direction) const;
	
	// FVector GetEvadeDirection(ACharacter* Character) const;
	void StartEvadeMovement(ACharacter* Character, const FVector& Direction);
	void StopEvadeMovement(ACharacter* Character);
	
	float GetStaminaCost(const FGameplayAbilityActorInfo* ActorInfo) const;
	bool HasEnoughStamina(const FGameplayAbilityActorInfo* ActorInfo) const;
	void ApplyStaminaCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const;
	void FinishEvade();
	
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();
	
};
