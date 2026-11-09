#pragma once

#include "CoreMinimal.h"
#include "Controller/BSEnemyAIControllerBase.h"
#include "BSBossAIController.generated.h"

UCLASS()
class BLACKSTAR_API ABSBossAIController : public ABSEnemyAIControllerBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	bool StartCombat(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void EndCombat();

	UFUNCTION(BlueprintPure, Category = "Boss|Combat")
	bool IsCombatActive() const
	{
		return bCombatActive;
	}

	virtual void HandleEnemyDeath() override;

protected:
	virtual void OnUnPossess() override;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Boss|Combat")
	bool bCombatActive = false;
};