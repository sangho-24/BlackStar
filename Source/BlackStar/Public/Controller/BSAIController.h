#pragma once

#include "CoreMinimal.h"
#include "Controller/BSEnemyAIControllerBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "BSAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

UCLASS()
class BLACKSTAR_API ABSAIController
	: public ABSEnemyAIControllerBase
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> BSPerceptionComponent;

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
	float SightTargetTraceTime = 5.0f;

	TWeakObjectPtr<AActor> VisibleTarget;
	TWeakObjectPtr<AActor> LostSightTarget;

	FTimerHandle SightTargetTraceTimerHandle;
	FTimerHandle StartLogicTimerHandle;

public:
	ABSAIController();

	AActor* GetVisibleTarget() const
	{
		return VisibleTarget.Get();
	}

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void StartSightTargetTraceTimer(AActor* TraceTarget);
	void CancelSightTargetTraceTimer();
	void FinishSightTargetTraceTimer();

private:
	void StartNormalEnemyLogic();
};