#pragma once

#include "CoreMinimal.h"
#include "Character/BSBaseCharacter.h"
#include "BSEnemyCharacter.generated.h"

UCLASS(Abstract)
class BLACKSTAR_API ABSEnemyCharacter : public ABSBaseCharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FString EnemyName = TEXT("Enemy");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	float CorpseLifeTime = 5.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<ABSBaseCharacter> CombatTarget = nullptr;

public:
	virtual AActor* GetCombatTarget() const override;
	virtual void SetCombatTarget(AActor* NewTarget) override;
	virtual void ClearCombatTarget() override;

	virtual void ApplyHitReaction(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData) override;

protected:
	virtual void OnDeathStarted(AActor* Killer) override;
	virtual void OnDeathFinished(AActor* Killer) override;
};