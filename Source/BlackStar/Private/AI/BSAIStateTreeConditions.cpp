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



// ===== 개편 =====
// 타겟이 있는가?
bool FSTCondition_HasCombatTarget::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;

	if (!EnemyCharacter || EnemyCharacter->IsDead())
	{
		return false;
	}

	const AActor* CombatTarget = EnemyCharacter->GetCombatTarget();

	if (!IsValid(CombatTarget))
	{
		return false;
	}

	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(CombatTarget);
	if (CombatInterface && CombatInterface->IsDead())
	{
		return false;
	}
	
	return true;
}


// ===== 마지막 위치 아는지? =====
bool FSTCondition_HasLastKnownTargetLocation::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;

	return EnemyCharacter 
		&& !EnemyCharacter->IsDead() 
		&& EnemyCharacter->HasLastKnownTargetLocation();
}


// ===== 거리가 멀어졌어요 =====
bool FSTCondition_IsCombatTargetFartherThan::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter || EnemyCharacter->IsDead())
	{
		return false;
	}

	const AActor* CombatTarget = EnemyCharacter->GetCombatTarget();
	if (!IsValid(CombatTarget))
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared2D(EnemyCharacter->GetActorLocation(), CombatTarget->GetActorLocation());
	return DistanceSquared > FMath::Square(InstanceData.Distance);
}

// ===== 정찰용 스플라인 있음? =====
bool FSTCondition_HasPatrolRoute::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	return InstanceData.EnemyCharacter && InstanceData.EnemyCharacter->HasPatrolRoute();
}