
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BSAIController.generated.h"

class UStateTreeAIComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class BLACKSTAR_API ABSAIController : public AAIController
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStateTreeAIComponent *StateTreeAI;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionComponent> BSPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
	
	FTimerHandle UpdateLastKnownTargetLocationTimerHandle;
	TWeakObjectPtr<AActor> VisibleTarget;
	


public:
	ABSAIController();
	void SendDeathStateTreeEvent();
	AActor* GetVisibleTarget() const { return VisibleTarget.Get(); }

protected:
	virtual void OnPossess(APawn *InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor *Actor, FAIStimulus Stimulus);
	
	UFUNCTION()
	void OnTargetPerceptionForgotten(AActor* Actor);
	
private:
	void StartLastKnownTargetLocationUpdate(AActor* TargetActor);
	void StopLastKnownTargetLocationUpdate();
	void UpdateLastKnownTargetLocation();
};
