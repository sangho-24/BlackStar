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

protected:
	virtual bool StartAttack(const FGameplayEventData* TriggerEventData) override;
	virtual void FaceAttackDirection() override;
	void RegisterComboEventTasks();
	void PlayNextCombo();

	UFUNCTION() void OnComboInput(FGameplayEventData Payload);
	UFUNCTION() void OnComboWindowOpen(FGameplayEventData Payload);
	UFUNCTION() void OnComboWindowClose(FGameplayEventData Payload);

	virtual void OnMontageCancelled() override;
};
