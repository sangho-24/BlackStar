#include "Character/BSEnemyCharacter.h"
#include "Controller/BSEnemyAIControllerBase.h"

AActor* ABSEnemyCharacter::GetCombatTarget() const
{
	return IsValid(CombatTarget) ? CombatTarget.Get() : nullptr;
}

void ABSEnemyCharacter::SetCombatTarget(AActor* NewTarget)
{
	CombatTarget = Cast<ABSBaseCharacter>(NewTarget);
}

void ABSEnemyCharacter::ClearCombatTarget()
{
	CombatTarget = nullptr;
}

void ABSEnemyCharacter::ApplyHitReaction(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData)
{
	if (IsDead())
	{
		return;
	}

	Super::ApplyHitReaction(Attacker, HitResult, ReactionData);

	if (auto* AI = Cast<ABSEnemyAIControllerBase>(GetController()))
	{
		AI->HandleEnemyHitReaction();
	}
}

void ABSEnemyCharacter::OnDeathStarted(AActor* Killer)
{
	Super::OnDeathStarted(Killer);

	if (auto* AI = Cast<ABSEnemyAIControllerBase>(GetController()))
	{
		AI->HandleEnemyDeath();
	}

	ClearCombatTarget();
}

void ABSEnemyCharacter::OnDeathFinished(AActor* Killer)
{
	Super::OnDeathFinished(Killer);

	if (CorpseLifeTime > 0.0f)
	{
		SetLifeSpan(CorpseLifeTime);
	}
}