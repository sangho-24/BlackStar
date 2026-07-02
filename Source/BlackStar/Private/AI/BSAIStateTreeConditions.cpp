#include "AI/BSAIStateTreeConditions.h"

#include "Character/BSEnemyCharacter.h"
#include "Controller/BSAIController.h"
#include "StateTreeExecutionContext.h"
#include "Components/CapsuleComponent.h"

// ===== 시야에 있는지 =====
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
	return FText::FromString(TEXT("타겟이 시야 안에 있음?"));
}
#endif

// ===== 범위 안에 있는지 =====
bool FSTCondition_IsCombatTargetInRange::TestCondition(FStateTreeExecutionContext& Context) const
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

	if (const ICombatInterface* CombatTargetInterface = Cast<ICombatInterface>(CombatTarget))
	{
		if (CombatTargetInterface->IsDead())
		{
			return false;
		}
	}

	float EffectiveRange = InstanceData.Range;

	if (InstanceData.bIncludeCapsuleRadius)
	{
		if (const UCapsuleComponent* EnemyCapsule = EnemyCharacter->GetCapsuleComponent())
		{
			EffectiveRange += EnemyCapsule->GetScaledCapsuleRadius();
		}

		if (const APawn* TargetPawn = Cast<APawn>(CombatTarget))
		{
			if (const UCapsuleComponent* TargetCapsule = TargetPawn->FindComponentByClass<UCapsuleComponent>())
			{
				EffectiveRange += TargetCapsule->GetScaledCapsuleRadius();
			}
		}
	}

	const float DistanceSq = FVector::DistSquared2D(
		EnemyCharacter->GetActorLocation(),
		CombatTarget->GetActorLocation());

	return DistanceSq <= FMath::Square(EffectiveRange);
}

#if WITH_EDITOR
FText FSTCondition_IsCombatTargetInRange::GetDescription(
	const FGuid& ID,
	FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup,
	EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("타겟이 범위 안에 있음?"));
}
#endif
