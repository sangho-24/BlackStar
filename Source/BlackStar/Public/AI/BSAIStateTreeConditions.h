// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"

#include "BSAIStateTreeConditions.generated.h"

class ABSEnemyCharacter;

USTRUCT()
struct FSTCondition_HasVisibleCombatTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABSEnemyCharacter> EnemyCharacter = nullptr;
};

USTRUCT(meta = (DisplayName = "Has Visible Combat Target", Category = "BlackStar|AI"))
struct BLACKSTAR_API FSTCondition_HasVisibleCombatTarget : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTCondition_HasVisibleCombatTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(
		const FGuid& ID,
		FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};
