
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Utility/BSTeam.h"
#include "BSAIController.generated.h"

class UStateTreeAIComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

UCLASS()
class BLACKSTAR_API ABSAIController : public AAIController
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStateTreeAIComponent *StateTreeAI;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Team")
	uint8 DefaultTeamId = BSTeam::Enemy;
	
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> BSPerceptionComponent;

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
	
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Reaction")
	float StimulusTurnSpeed = 360.0f;
	
	// FTimerHandle UpdateLastKnownTargetLocationTimerHandle;
	TWeakObjectPtr<AActor> VisibleTarget;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
	float SightTargetTraceTime = 2.0f;
	
	FTimerHandle SightTargetTraceTimerHandle;
	TWeakObjectPtr<AActor> LostSightTarget;


public:
	ABSAIController();
	void SendDeathStateTreeEvent();
	AActor* GetVisibleTarget() const { return VisibleTarget.Get(); }
	
	// virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	virtual void OnPossess(APawn *InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor *Actor, FAIStimulus Stimulus);
	
	UFUNCTION()
	// void OnTargetPerceptionForgotten(AActor* Actor);
	
	void TurnTowardsStimulus(ABSEnemyCharacter* EnemyCharacter, const FVector& StimulusLocation);
	
	void StartSightTargetTraceTimer(AActor* TraceTarget);
	void CancelSightTargetTraceTimer();
	void FinishSightTargetTraceTimer();
private:
	// void StartLastKnownTargetLocationUpdate(AActor* TargetActor);
	// void StopLastKnownTargetLocationUpdate();
	// void UpdateLastKnownTargetLocation();
};
