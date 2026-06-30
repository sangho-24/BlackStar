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

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context, 
		const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(
		const FGuid& ID,
		FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

private:
	static FGameplayAbilitySpec* FindAbilitySpecByTag(
		UBSAbilitySystemComponent* BSASC, const FGameplayTag& AbilityTag);

	static void UnbindAbilityEndedDelegate(FInstanceDataType& InstanceData);
};

USTRUCT()
struct FSTTask_MoveToLastKnownTargetLocationInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABSEnemyCharacter> EnemyCharacter = nullptr;

	// 도착 판정 거리
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 120.0f;

	// 목표가 이만큼 이동하면 갱신
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float RepathDistance = 100.0f;

	// 마지막으로 요청한 이동 위치, RepathDistance에서 사용
	FVector LastRequestedMoveLocation = FVector::ZeroVector;
};

USTRUCT(meta = (DisplayName = "Move To Last Known Target Location", Category = "BlackStar|AI"))
struct FSTTask_MoveToLastKnownTargetLocation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTTask_MoveToLastKnownTargetLocation();

	using FInstanceDataType = FSTTask_MoveToLastKnownTargetLocationInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
	
#if WITH_EDITOR
	virtual FText GetDescription(
		const FGuid& ID,
		FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

private:
	static EStateTreeRunStatus RequestMove(FInstanceDataType& InstanceData);
};