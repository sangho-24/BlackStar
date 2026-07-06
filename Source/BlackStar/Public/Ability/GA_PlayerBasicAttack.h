#pragma once

#include "CoreMinimal.h"
#include "Ability/GA_AttackBase.h"
#include "GA_PlayerBasicAttack.generated.h"

UCLASS()
class BLACKSTAR_API UGA_PlayerBasicAttack : public UGA_AttackBase
{
	GENERATED_BODY()

public:
	UGA_PlayerBasicAttack();

protected:
	bool bSaveCombo = false;
	bool bIsComboWindowOpen = false;
	bool bComboTransitioning = false;
	
	// 한 번의 공격에서 허용하는 최대 회전각
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Rotation", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxAttackTurnAngle = 60.0f;

	// 초당 회전 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Rotation", meta = (ClampMin = "0.0"))
	float AttackTurnSpeed = 720.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Setup")
	TSubclassOf<UGameplayEffect> StaminaCostEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Setup")
	float BaseStaminaCost = 15.0f;

private:
	bool bSpentStamina = false;

protected:
	virtual void EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled) override;
	
	float GetStaminaCost(const FGameplayAbilityActorInfo* ActorInfo) const;
	bool HasEnoughStamina(const FGameplayAbilityActorInfo* ActorInfo) const;
	void ApplyStaminaCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const;
	
	virtual bool StartAttack(const FGameplayEventData* TriggerEventData) override;
	virtual void FaceAttackDirection() override;
	void RegisterComboEventTasks();
	void PlayNextCombo();

	UFUNCTION() void OnComboInput(FGameplayEventData Payload);
	UFUNCTION() void OnComboWindowOpen(FGameplayEventData Payload);
	UFUNCTION() void OnComboWindowClose(FGameplayEventData Payload);

	virtual void OnMontageCancelled() override;
};
