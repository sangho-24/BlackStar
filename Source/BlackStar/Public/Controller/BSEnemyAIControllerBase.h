#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "Utility/BSTeam.h"
#include "BSEnemyAIControllerBase.generated.h"

class ABSEnemyCharacter;
class UStateTreeAIComponent;

UCLASS(Abstract)
class BLACKSTAR_API ABSEnemyAIControllerBase : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeAI;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Team")
	uint8 DefaultTeamId = BSTeam::Enemy;

public:
	ABSEnemyAIControllerBase();

	ABSEnemyCharacter* GetEnemyCharacter() const;

	void SendStateTreeEvent(FGameplayTag EventTag);

	virtual void HandleEnemyHitReaction();
	virtual void HandleEnemyDeath();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	bool StartEnemyLogic();
	void StopEnemyLogic(const FString& Reason);
};