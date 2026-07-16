#include "AI/BSAIStateTreeTasks.h"

#include "GAS/BSAbilitySystemComponent.h"
#include "Character/BSBaseCharacter.h"
#include "Character/BSEnemyCharacter.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeExecutionTypes.h"
#include "StateTreeAsyncExecutionContext.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// ===== 어빌리티 발동!! =====
FSTTask_ActivateAbilityByTag::FSTTask_ActivateAbilityByTag()
{
	bShouldCallTick = false;
	// 갱신 시 Enter/Exit를 다시 호출할지 여부
	bShouldStateChangeOnReselect = false;
}

EStateTreeRunStatus FSTTask_ActivateAbilityByTag::EnterState(
	FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.Character || !InstanceData.AbilityTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	UBSAbilitySystemComponent* BSASC = Cast<UBSAbilitySystemComponent>(
		InstanceData.Character->GetAbilitySystemComponent());
	if (!BSASC)
	{
		return EStateTreeRunStatus::Failed;
	}
	// 스펙으로 찾아서 발동시키기 위함
	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecByTag(BSASC, InstanceData.AbilityTag);
	if (!AbilitySpec)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (/*InstanceData.bFailIfAlreadyActive && */AbilitySpec->IsActive())
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.BoundASC = BSASC;
	InstanceData.ActivatedAbilityHandle = AbilitySpec->Handle;
	
	InstanceData.AbilityEndedHandle = BSASC->OnAbilityEnded.AddLambda(
		[
			// 나중에 ST를 완료시키기 위한 약한 실행 컨텍스트.
			WeakContext = Context.MakeWeakExecutionContext(),
			ExpectedHandle = InstanceData.ActivatedAbilityHandle,
			AbilityTag = InstanceData.AbilityTag,
			bTreatCancelledAsFailure = InstanceData.bTreatCancelledAsFailure
		](const FAbilityEndedData& EndedData)
		{
			// 얼리리턴의 경우 성공/실패 판단이 아니라 무시 (기다리는 어빌리티 아님)
			if (EndedData.AbilitySpecHandle != ExpectedHandle)
			{
				return;
			}
			if (!EndedData.AbilityThatEnded)
			{
				return;
			}
			if (!EndedData.AbilityThatEnded->GetAssetTags().HasTagExact(AbilityTag))
			{
				return;
			}

			const EStateTreeFinishTaskType FinishType =
				EndedData.bWasCancelled && bTreatCancelledAsFailure
					? EStateTreeFinishTaskType::Failed
					: EStateTreeFinishTaskType::Succeeded;

			WeakContext.FinishTask(FinishType);
		});

	const bool bActivated = BSASC->TryActivateAbility(AbilitySpec->Handle);
	if (!bActivated)
	{
		UnbindAbilityEndedDelegate(InstanceData);
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

void FSTTask_ActivateAbilityByTag::ExitState(
	FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	UnbindAbilityEndedDelegate(InstanceData);
}

FGameplayAbilitySpec* FSTTask_ActivateAbilityByTag::FindAbilitySpecByTag(
	UBSAbilitySystemComponent* BSASC, const FGameplayTag& AbilityTag)
{
	if (!BSASC || !AbilityTag.IsValid())
	{
		return nullptr;
	}

	for (FGameplayAbilitySpec& Spec : BSASC->GetActivatableAbilities())
	{
		const UGameplayAbility* Ability = Spec.Ability;
		if (!Ability)
		{
			continue;
		}

		if (Ability->GetAssetTags().HasTagExact(AbilityTag))
		{
			return &Spec;
		}
	}

	return nullptr;
}

void FSTTask_ActivateAbilityByTag::UnbindAbilityEndedDelegate(FInstanceDataType& InstanceData)
{
	if (UBSAbilitySystemComponent* BSASC = InstanceData.BoundASC.Get())
	{
		if (InstanceData.AbilityEndedHandle.IsValid())
		{
			BSASC->OnAbilityEnded.Remove(InstanceData.AbilityEndedHandle);
		}
	}

	InstanceData.BoundASC.Reset();
	InstanceData.AbilityEndedHandle.Reset();
	InstanceData.ActivatedAbilityHandle = FGameplayAbilitySpecHandle();
}




// ===== 퍼셉션 위치로 이동!! =====
FSTTask_MoveToLastKnownTargetLocation::FSTTask_MoveToLastKnownTargetLocation()
{
	bShouldCallTick = true;
	bShouldStateChangeOnReselect = false;
}

EStateTreeRunStatus FSTTask_MoveToLastKnownTargetLocation::EnterState(
	FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.EnemyCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}
	if (UCharacterMovementComponent* Movement = EnemyCharacter->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = InstanceData.MovementSpeed;
	}
	
	
	InstanceData.LastRequestedTarget.Reset();
	InstanceData.LastRequestedMoveLocation = FVector::ZeroVector;
	InstanceData.bLastRequestWasActor = false;
	
	return RequestMove(InstanceData);
}

EStateTreeRunStatus FSTTask_MoveToLastKnownTargetLocation::Tick(
	FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.EnemyCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	return RequestMove(InstanceData);
}

void FSTTask_MoveToLastKnownTargetLocation::ExitState(
	FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.EnemyCharacter)
	{
		return;
	}

	if (AAIController* AIController = Cast<AAIController>(InstanceData.EnemyCharacter->GetController()))
	{
		AIController->StopMovement();
	}

	InstanceData.LastRequestedTarget.Reset();
	InstanceData.LastRequestedMoveLocation = FVector::ZeroVector;
	InstanceData.bLastRequestWasActor = false;
}

EStateTreeRunStatus FSTTask_MoveToLastKnownTargetLocation::RequestMove(FInstanceDataType& InstanceData)
{
	ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());
	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	if (AActor* CombatTarget = EnemyCharacter->GetCombatTarget())
	{
		if (ICombatInterface* CombatTargetInterface = Cast<ICombatInterface>(CombatTarget))
		{
			if (CombatTargetInterface->IsDead())
			{
				return EStateTreeRunStatus::Failed;
			}
		}

		if (InstanceData.bLastRequestWasActor && InstanceData.LastRequestedTarget.Get() == CombatTarget)
		{
			const EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();
			if (MoveStatus == EPathFollowingStatus::Moving || MoveStatus == EPathFollowingStatus::Waiting)
			{
				return EStateTreeRunStatus::Running;
			}
		}
		
		return RequestMoveToActor(InstanceData, CombatTarget);
	}

	if (!EnemyCharacter->HasLastKnownTargetLocation())
	{
		return EStateTreeRunStatus::Failed;
	}
	const FVector GoalLocation = EnemyCharacter->GetLastKnownTargetLocation();

	if (HasReachedLocation(InstanceData, GoalLocation))
	{
		EnemyCharacter->ClearLastKnownTargetLocation();
		return EStateTreeRunStatus::Succeeded;
	}

	if (!InstanceData.bLastRequestWasActor && InstanceData.LastRequestedMoveLocation.Equals(GoalLocation, 1.0f))
	{
		return EStateTreeRunStatus::Running;
	}

	return RequestMoveToLocation(InstanceData, GoalLocation);
}

EStateTreeRunStatus FSTTask_MoveToLastKnownTargetLocation::RequestMoveToActor(
	FInstanceDataType& InstanceData, AActor* TargetActor)
{
	ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter || !TargetActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());
	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	const EPathFollowingRequestResult::Type MoveResult =
		AIController->MoveToActor(
			TargetActor,
			InstanceData.AcceptanceRadius,
			true,
			true,
			true,
			nullptr,
			true);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.LastRequestedTarget = TargetActor;
	InstanceData.LastRequestedMoveLocation = FVector::ZeroVector;
	InstanceData.bLastRequestWasActor = true;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTTask_MoveToLastKnownTargetLocation::RequestMoveToLocation(
	FInstanceDataType& InstanceData, const FVector& GoalLocation)
{
	ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());
	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	const EPathFollowingRequestResult::Type MoveResult =
		AIController->MoveToLocation(
			GoalLocation,
			0.0f,
			true,
			true,
			true,
			false,
			nullptr,
			true);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.LastRequestedTarget.Reset();
	InstanceData.LastRequestedMoveLocation = GoalLocation;
	InstanceData.bLastRequestWasActor = false;

	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		EnemyCharacter->ClearLastKnownTargetLocation();
		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Running;
}

bool FSTTask_MoveToLastKnownTargetLocation::HasReachedLocation(const FInstanceDataType& InstanceData, const FVector& GoalLocation)
{
	const ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter)
	{
		return false;
	}
	const FVector CurrentLocation = EnemyCharacter->GetActorLocation();
	return FVector::DistSquared2D(CurrentLocation, GoalLocation) <= FMath::Square(InstanceData.AcceptanceRadius);
}



// ===== 잠시 대기!! ======
FSTTask_WaitRandom::FSTTask_WaitRandom()
{
	bShouldCallTick = true;
	bShouldStateChangeOnReselect = false;
}

EStateTreeRunStatus FSTTask_WaitRandom::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const float MinTime = FMath::Max(0.0f, InstanceData.MinWaitTime);
	const float MaxTime = FMath::Max(MinTime, InstanceData.MaxWaitTime);

	InstanceData.ElapsedTime = 0.0f;
	InstanceData.TargetWaitTime = FMath::RandRange(MinTime, MaxTime);

	return InstanceData.TargetWaitTime <= 0.0f ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTTask_WaitRandom::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.ElapsedTime += DeltaTime;
	return InstanceData.ElapsedTime >= InstanceData.TargetWaitTime ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}



// ===== 패트롤 =====
// ===== 랜덤 패트롤 =====
FSTTask_RandomPatrol::FSTTask_RandomPatrol()
{
	bShouldCallTick = true;
	bShouldStateChangeOnReselect = false;
}

EStateTreeRunStatus FSTTask_RandomPatrol::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
    if (!EnemyCharacter || EnemyCharacter->IsDead())
    {
        return EStateTreeRunStatus::Failed;
    }

    AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());
    if (!AIController)
    {
        return EStateTreeRunStatus::Failed;
    }

    UCharacterMovementComponent* Movement = EnemyCharacter->GetCharacterMovement();
    if (!Movement)
    {
        return EStateTreeRunStatus::Failed;
    }

    // 이전 ChaseRun 속도를 Patrol 속도로 덮어씀
    Movement->MaxWalkSpeed = InstanceData.PatrolSpeed;

    UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(EnemyCharacter->GetWorld());
    if (!NavigationSystem)
    {
        return EStateTreeRunStatus::Failed;
    }

    InstanceData.bHasPatrolDestination = false;
    const FVector Origin = EnemyCharacter->GetPatrolOrigin();

    // 너무 가까운 점이 뽑히면 몇 번 다시 시도
    constexpr int32 MaxAttempts = 8;

    for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
    {
        FNavLocation RandomLocation;
        const bool bFoundLocation = NavigationSystem->GetRandomReachablePointInRadius(Origin, InstanceData.PatrolRadius, RandomLocation);
        if (!bFoundLocation)
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared2D(EnemyCharacter->GetActorLocation(), RandomLocation.Location);
        if (DistanceSquared < FMath::Square(InstanceData.MinimumMoveDistance))
        {
            continue;
        }

        InstanceData.PatrolDestination = RandomLocation.Location;
        InstanceData.bHasPatrolDestination = true;
        break;
    }

    if (!InstanceData.bHasPatrolDestination)
    {
        return EStateTreeRunStatus::Failed;
    }

    const EPathFollowingRequestResult::Type Result =
        AIController->MoveToLocation(
            InstanceData.PatrolDestination,
            InstanceData.AcceptanceRadius,
            true,
            true,
            true,
            false,
            nullptr,
            true);

    if (Result == EPathFollowingRequestResult::Failed)
    {
        return EStateTreeRunStatus::Failed;
    }
    if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        return EStateTreeRunStatus::Succeeded;
    }
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTTask_RandomPatrol::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter || EnemyCharacter->IsDead() || !InstanceData.bHasPatrolDestination)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());
	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	const float DistanceSquared = FVector::DistSquared2D(EnemyCharacter->GetActorLocation(), InstanceData.PatrolDestination);
	if (DistanceSquared <= FMath::Square(InstanceData.AcceptanceRadius))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	const EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();
	if (MoveStatus == EPathFollowingStatus::Moving || MoveStatus == EPathFollowingStatus::Waiting)
	{
		return EStateTreeRunStatus::Running;
	}
	
	return EStateTreeRunStatus::Failed;
}

void FSTTask_RandomPatrol::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.EnemyCharacter)
	{
		if (AAIController* AIController = Cast<AAIController>(InstanceData.EnemyCharacter->GetController()))
		{
			AIController->StopMovement();
		}
	}
	InstanceData.PatrolDestination =FVector::ZeroVector;
	InstanceData.bHasPatrolDestination = false;
}


// ===== 루트 패트롤 =====
FSTTask_RoutePatrol::FSTTask_RoutePatrol()
{
	bShouldCallTick = true;
	bShouldStateChangeOnReselect = false;
}

EStateTreeRunStatus FSTTask_RoutePatrol::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter || EnemyCharacter->IsDead() || !EnemyCharacter->HasPatrolRoute())
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());
	UCharacterMovementComponent* Movement = EnemyCharacter->GetCharacterMovement();
	if (!AIController || !Movement)
	{
		return EStateTreeRunStatus::Failed;
	}

	Movement->MaxWalkSpeed = FMath::Max(0.0f, InstanceData.PatrolSpeed);
	InstanceData.PatrolDestination = EnemyCharacter->GetCurrentPatrolPointLocation();
	InstanceData.bMoveRequested = false;

	const EPathFollowingRequestResult::Type Result =
		AIController->MoveToLocation(
			InstanceData.PatrolDestination,
			InstanceData.AcceptanceRadius,
			false,
			true,
			true,
			false,
			nullptr,
			true);

	if (Result == EPathFollowingRequestResult::Failed)
	{
		return EStateTreeRunStatus::Failed;
	}
	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		EnemyCharacter->AdvancePatrolPoint();
		return EStateTreeRunStatus::Succeeded;
	}

	InstanceData.bMoveRequested = true;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTTask_RoutePatrol::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ABSEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter || EnemyCharacter->IsDead() || !EnemyCharacter->HasPatrolRoute() || !InstanceData.bMoveRequested)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());
	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	const float DistanceSquared = FVector::DistSquared2D(EnemyCharacter->GetActorLocation(), InstanceData.PatrolDestination);
	if (DistanceSquared <= FMath::Square(InstanceData.AcceptanceRadius))
	{
		EnemyCharacter->AdvancePatrolPoint();
		return EStateTreeRunStatus::Succeeded;
	}
	
	const EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();
	if (MoveStatus == EPathFollowingStatus::Moving || MoveStatus == EPathFollowingStatus::Waiting)
	{
		return EStateTreeRunStatus::Running;
	}
	
	return EStateTreeRunStatus::Failed;
}

void FSTTask_RoutePatrol::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.EnemyCharacter)
	{
		if (AAIController* AIController = Cast<AAIController>(InstanceData.EnemyCharacter->GetController()))
		{
			AIController->StopMovement();
		}
	}
	InstanceData.PatrolDestination = FVector::ZeroVector;
	InstanceData.bMoveRequested = false;
}