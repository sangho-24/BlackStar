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
