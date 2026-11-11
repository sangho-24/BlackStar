#include "AI/BSAIStateTreeTasks.h"

#include "GAS/BSAbilitySystemComponent.h"
#include "Character/BSBaseCharacter.h"
#include "Character/BSEnemyCharacter.h"
#include "Character/BSNormalEnemyCharacter.h"
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

	ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
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
	ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
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
	ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
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
	ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
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
	const ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
	if (!EnemyCharacter)
	{
		return false;
	}
	const FVector CurrentLocation = EnemyCharacter->GetActorLocation();
	return FVector::DistSquared2D(CurrentLocation, GoalLocation) <= FMath::Square(InstanceData.AcceptanceRadius);
}



// ===== 랜덤 대기!! ======
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

// ===== 태그 없어질 때까지 대기 =====
FSTTask_WaitForGameplayTagRemoved::FSTTask_WaitForGameplayTagRemoved()
{
	bShouldCallTick = true;
	bShouldStateChangeOnReselect = false;
}

EStateTreeRunStatus FSTTask_WaitForGameplayTagRemoved::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.EnemyCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	const UAbilitySystemComponent* ASC = InstanceData.EnemyCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	return ASC->HasMatchingGameplayTag(InstanceData.GameplayTag) ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FSTTask_WaitForGameplayTagRemoved::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.EnemyCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	const UAbilitySystemComponent* ASC = InstanceData.EnemyCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	return ASC->HasMatchingGameplayTag(InstanceData.GameplayTag) ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
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
    ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
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

	AIController->StopMovement();
	EnemyCharacter->StopTurning();
	InstanceData.bMoveRequested = false;
	InstanceData.TargetYaw = 0.0f;
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

	FVector Direction = InstanceData.PatrolDestination - EnemyCharacter->GetActorLocation();
	Direction.Z = 0.0f;
	if (Direction.IsNearlyZero())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	InstanceData.TargetYaw = Direction.Rotation().Yaw;
	const float RemainingAngle = FMath::Abs(FMath::FindDeltaAngleDegrees(EnemyCharacter->GetActorRotation().Yaw, InstanceData.TargetYaw));
	// 이미 목적지를 바라보고 있다면 바로 이동.
	if (FMath::IsNearlyZero(RemainingAngle))
	{
		return StartPatrolMove(InstanceData);
	}
	// 현재 StartTurning은 속도가 0 이하면 순간회전하므로 방지.
	if (InstanceData.TurnSpeed <= 0.0f)
	{
		return EStateTreeRunStatus::Failed;
	}
	EnemyCharacter->StartTurning(FRotator(0.0f, InstanceData.TargetYaw, 0.0f), InstanceData.TurnSpeed);
	// 피격 등으로 회전을 시작하지 못했다면 실패.
	return EnemyCharacter->IsTurning() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FSTTask_RandomPatrol::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;

	if (!EnemyCharacter || EnemyCharacter->IsDead() || !InstanceData.bHasPatrolDestination)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());

	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	// ===== 회전 단계
	if (!InstanceData.bMoveRequested)
	{
		const float RemainingAngle = FMath::Abs(FMath::FindDeltaAngleDegrees(EnemyCharacter->GetActorRotation().Yaw, InstanceData.TargetYaw));
		if (FMath::IsNearlyZero(RemainingAngle))
		{
			return StartPatrolMove(InstanceData);
		}
		// 목표 방향에 도달하기 전에 회전이 취소된 경우.
		if (!EnemyCharacter->IsTurning())
		{
			return EStateTreeRunStatus::Failed;
		}
		return EStateTreeRunStatus::Running;
	}

	// ===== 이동 단계
	const float DistanceSquared = FVector::DistSquared2D(EnemyCharacter->GetActorLocation(), InstanceData.PatrolDestination);

	if (DistanceSquared <= FMath::Square(InstanceData.AcceptanceRadius))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	const EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();

	if (MoveStatus == EPathFollowingStatus::Moving || MoveStatus == EPathFollowingStatus::Waiting || MoveStatus == EPathFollowingStatus::Paused)
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
		InstanceData.EnemyCharacter->StopTurning();
		if (AAIController* AIController = Cast<AAIController>(InstanceData.EnemyCharacter->GetController()))
		{
			AIController->StopMovement();
		}
	}
	InstanceData.PatrolDestination = FVector::ZeroVector;
	InstanceData.bHasPatrolDestination = false;
}

EStateTreeRunStatus FSTTask_RandomPatrol::StartPatrolMove(FInstanceDataType& InstanceData)
{
	ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;

	if (!EnemyCharacter || EnemyCharacter->IsDead())
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());

	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 이동 중에는 수동 회전이 이동 회전과 겹치지 않도록 종료.
	EnemyCharacter->StopTurning();

	const EPathFollowingRequestResult::Type Result =
		AIController->MoveToLocation(
			InstanceData.PatrolDestination,
			InstanceData.AcceptanceRadius,
			false,  // 아래 Tick의 거리 판정과 맞추기 위해 반경 추가 제외.
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

	InstanceData.bMoveRequested = true;

	return EStateTreeRunStatus::Running;
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
	ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;
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

	AIController->StopMovement();
	EnemyCharacter->StopTurning();
	InstanceData.TargetYaw = 0.0f;
	Movement->MaxWalkSpeed = FMath::Max(0.0f, InstanceData.PatrolSpeed);
	InstanceData.PatrolDestination = EnemyCharacter->GetCurrentPatrolPointLocation();
	InstanceData.bMoveRequested = false;

	const float DistanceSquared = FVector::DistSquared2D(EnemyCharacter->GetActorLocation(), InstanceData.PatrolDestination);

	if (DistanceSquared <= FMath::Square(InstanceData.AcceptanceRadius))
	{
		EnemyCharacter->AdvancePatrolPoint();
		return EStateTreeRunStatus::Succeeded;
	}

	FVector Direction = InstanceData.PatrolDestination - EnemyCharacter->GetActorLocation();

	Direction.Z = 0.0f;

	InstanceData.TargetYaw = Direction.Rotation().Yaw;

	const float RemainingAngle = FMath::Abs(FMath::FindDeltaAngleDegrees(EnemyCharacter->GetActorRotation().Yaw, InstanceData.TargetYaw));

	// 이미 목적지 방향을 바라보고 있다면 바로 이동.
	if (FMath::IsNearlyZero(RemainingAngle))
	{
		return StartPatrolMove(InstanceData);
	}

	// StartTurning에서 순간회전으로 처리되는 값은 사용하지 않음.
	if (InstanceData.TurnSpeed <= 0.0f)
	{
		return EStateTreeRunStatus::Failed;
	}

	EnemyCharacter->StartTurning(FRotator(0.0f, InstanceData.TargetYaw, 0.0f), InstanceData.TurnSpeed);

	return EnemyCharacter->IsTurning() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FSTTask_RoutePatrol::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;

	if (!EnemyCharacter || EnemyCharacter->IsDead() || !EnemyCharacter->HasPatrolRoute())
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());

	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	// ===== 회전 단계 =====
	if (!InstanceData.bMoveRequested)
	{
		const float RemainingAngle = FMath::Abs(FMath::FindDeltaAngleDegrees(EnemyCharacter->GetActorRotation().Yaw, InstanceData.TargetYaw));

		if (FMath::IsNearlyZero(RemainingAngle))
		{
			return StartPatrolMove(InstanceData);
		}

		// 목표 방향에 도달하기 전에 회전이 취소된 경우.
		if (!EnemyCharacter->IsTurning())
		{
			return EStateTreeRunStatus::Failed;
		}

		return EStateTreeRunStatus::Running;
	}

	// ===== 이동 단계 =====
	const float DistanceSquared = FVector::DistSquared2D(EnemyCharacter->GetActorLocation(), InstanceData.PatrolDestination);

	if (DistanceSquared <= FMath::Square(InstanceData.AcceptanceRadius))
	{
		EnemyCharacter->AdvancePatrolPoint();
		return EStateTreeRunStatus::Succeeded;
	}

	const EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();

	if (MoveStatus == EPathFollowingStatus::Moving || MoveStatus == EPathFollowingStatus::Waiting || MoveStatus == EPathFollowingStatus::Paused)
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
		InstanceData.EnemyCharacter->StopTurning();
		if (AAIController* AIController = Cast<AAIController>(InstanceData.EnemyCharacter->GetController()))
		{
			AIController->StopMovement();
		}
	}
	InstanceData.PatrolDestination = FVector::ZeroVector;
	InstanceData.TargetYaw = 0.0f;
	InstanceData.bMoveRequested = false;
}

EStateTreeRunStatus FSTTask_RoutePatrol::StartPatrolMove(FInstanceDataType& InstanceData)
{
	ABSNormalEnemyCharacter* EnemyCharacter = InstanceData.EnemyCharacter;

	if (!EnemyCharacter || EnemyCharacter->IsDead() || !EnemyCharacter->HasPatrolRoute())
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());

	if (!AIController)
	{
		return EStateTreeRunStatus::Failed;
	}

	EnemyCharacter->StopTurning();

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

// 대상 위치로 회전
FSTTask_RotateToLastKnownTargetLocation::FSTTask_RotateToLastKnownTargetLocation()
{
	bShouldCallTick = true;
	bShouldStateChangeOnReselect = false;
}

EStateTreeRunStatus FSTTask_RotateToLastKnownTargetLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	ABSNormalEnemyCharacter* EnemyCharacter = Data.EnemyCharacter;
	if (!EnemyCharacter || EnemyCharacter->IsDead())
	{
		return EStateTreeRunStatus::Failed;
	}
	FVector TargetLocation = FVector::ZeroVector;
	if (AActor* CombatTarget = EnemyCharacter->GetCombatTarget())
	{
		TargetLocation = CombatTarget->GetActorLocation();
	}
	else if (EnemyCharacter->HasLastKnownTargetLocation())
	{
		TargetLocation = EnemyCharacter->GetLastKnownTargetLocation();
	}
	else
	{
		return EStateTreeRunStatus::Failed;
	}
	
	FVector Direction = TargetLocation - EnemyCharacter->GetActorLocation();
	Direction.Z = 0.0f;
	if (Direction.IsNearlyZero())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	EnemyCharacter->StartTurning(Direction.Rotation(), Data.TurnSpeed);
	return EnemyCharacter->IsTurning() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FSTTask_RotateToLastKnownTargetLocation::Tick(FStateTreeExecutionContext& Context, float DeltaTime) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.EnemyCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	return Data.EnemyCharacter->IsTurning() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

void FSTTask_RotateToLastKnownTargetLocation::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (Data.EnemyCharacter)
	{
		Data.EnemyCharacter->StopTurning();
	}
}