#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"

#include "BSAIStateTreeTasks.generated.h"

class ABSBaseCharacter;
class ABSEnemyCharacter;
class AAIController;
class UBSAbilitySystemComponent;

// ===== 어빌리티 발동!! =====
USTRUCT()
struct FSTTask_ActivateAbilityByTagInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABSBaseCharacter> Character = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag AbilityTag;

	// 해당 어빌리티가 이미 실행중이면 실패. 혹은 갱신(?)
	// UPROPERTY(EditAnywhere, Category = "Parameter")
	// bool bFailIfAlreadyActive = false;

	// 어빌리티 실패가 스테이트 트리 실패인지 여부
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bTreatCancelledAsFailure = true;

	// 현재 바인딩한 ASC와 핸들들, Task가 끝날 때 델리게이트 해제하려고 들고 있음.
	TWeakObjectPtr<UBSAbilitySystemComponent> BoundASC;
	FDelegateHandle AbilityEndedHandle;
	FGameplayAbilitySpecHandle ActivatedAbilityHandle;
};

USTRUCT(meta = (DisplayName = "Activate Ability By Tag", Category = "BlackStar|GAS"))
struct FSTTask_ActivateAbilityByTag : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTTask_ActivateAbilityByTag();

	using FInstanceDataType = FSTTask_ActivateAbilityByTagInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	static FGameplayAbilitySpec* FindAbilitySpecByTag(UBSAbilitySystemComponent* BSASC, const FGameplayTag& AbilityTag);

	static void UnbindAbilityEndedDelegate(FInstanceDataType& InstanceData);
	

};

// ===== 타겟에게 이동!! =====
USTRUCT()
struct FSTTask_MoveToLastKnownTargetLocationInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABSEnemyCharacter> EnemyCharacter = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float MovementSpeed = 200.0f;
	
	// 도착 판정 거리
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 120.0f;

	TWeakObjectPtr<AActor> LastRequestedTarget = nullptr;
	FVector LastRequestedMoveLocation = FVector::ZeroVector;
	bool bLastRequestWasActor = false;
};

USTRUCT(meta = (DisplayName = "Move To Last Known Target Location", Category = "BlackStar|AI"))
struct FSTTask_MoveToLastKnownTargetLocation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTTask_MoveToLastKnownTargetLocation();

	using FInstanceDataType = FSTTask_MoveToLastKnownTargetLocationInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
private:
	static EStateTreeRunStatus RequestMove(FInstanceDataType& InstanceData);
	static EStateTreeRunStatus RequestMoveToActor(FInstanceDataType& InstanceData, AActor* TargetActor);
	static EStateTreeRunStatus RequestMoveToLocation(FInstanceDataType& InstanceData, const FVector& GoalLocation);
	static bool HasReachedLocation(const FInstanceDataType& InstanceData, const FVector& GoalLocation);
	

};

// ===== 잠시 대기!! ======
USTRUCT()
struct FSTTask_WaitRandomInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MinWaitTime = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MaxWaitTime = 1.2f;

	float ElapsedTime = 0.0f;
	float TargetWaitTime = 0.0f;
};

USTRUCT(meta = (DisplayName = "Wait Random", Category = "BlackStar|AI"))
struct FSTTask_WaitRandom : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTTask_WaitRandom();

	using FInstanceDataType = FSTTask_WaitRandomInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	

};

// ===== 랜덤 패트롤 =====
USTRUCT()
struct FSTTask_RandomPatrolInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABSEnemyCharacter> EnemyCharacter = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float PatrolSpeed = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float PatrolRadius = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float MinimumMoveDistance = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float AcceptanceRadius = 80.0f;

	FVector PatrolDestination = FVector::ZeroVector;
	bool bHasPatrolDestination = false;
};

USTRUCT(meta = (DisplayName = "RandomPatrol", Category = "BlackStar|AI"))
struct FSTTask_RandomPatrol : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTTask_RandomPatrol();

	using FInstanceDataType = FSTTask_RandomPatrolInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	

};

// ===== 루트 패트롤 =====
USTRUCT()
struct FSTTask_RoutePatrolInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABSEnemyCharacter> EnemyCharacter = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float PatrolSpeed = 120.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float AcceptanceRadius = 80.0f;

	FVector PatrolDestination = FVector::ZeroVector;

	bool bMoveRequested = false;
};

USTRUCT(meta = (DisplayName = "Route Patrol", Category = "BlackStar|AI"))
struct FSTTask_RoutePatrol : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTTask_RoutePatrol();

	using FInstanceDataType = FSTTask_RoutePatrolInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};