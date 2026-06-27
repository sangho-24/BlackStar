#include "AI/BSAIStateTreeTasks.h"

#include "GAS/BSAbilitySystemComponent.h"
#include "Character/BSBaseCharacter.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeExecutionTypes.h"
#include "StateTreeAsyncExecutionContext.h"

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
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
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

#if WITH_EDITOR
FText FSTTask_ActivateAbilityByTag::GetDescription(
	const FGuid& ID,
	FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup,
	EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("태그로 GAS어빌리티 발동!!"));
}
#endif