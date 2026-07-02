#include "AI/BSAIStateTreeConditions.h"

#include "Character/BSEnemyCharacter.h"
#include "Controller/BSAIController.h"
#include "StateTreeExecutionContext.h"

bool FSTCondition_HasVisibleCombatTarget::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter || EnemyCharacter->IsDead())
	{
		return false;
	}

	AActor* CombatTarget = EnemyCharacter->GetCombatTarget();
	if (!CombatTarget)
	{
		return false;
	}

	const ABSAIController* AIController = Cast<ABSAIController>(EnemyCharacter->GetController());
	if (!AIController)
	{
		return false;
	}

	return AIController->GetVisibleTarget() == CombatTarget;
}

#if WITH_EDITOR
FText FSTCondition_HasVisibleCombatTarget::GetDescription(
	const FGuid& ID,
	FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup,
	EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("대상이 시야에 있는가?"));
}
#endif
