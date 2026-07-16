// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"

#include "BSAIStateTreeConditions.generated.h"

class ABSEnemyCharacter;

// ===== 시야에 있는지 =====
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


};

// ===== 범위 안에 있는지 =====
USTRUCT()
struct FSTCondition_IsCombatTargetInRangeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABSEnemyCharacter> EnemyCharacter = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float Range = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bIncludeCapsuleRadius = true;
};

USTRUCT(meta = (DisplayName = "Is Combat Target In Range", Category = "BlackStar|AI"))
struct BLACKSTAR_API FSTCondition_IsCombatTargetInRange : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTCondition_IsCombatTargetInRangeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;


};

// ===== 개편 =====

// ===== 타겟이 있는지? =====
USTRUCT()
struct FSTCondition_HasCombatTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABSEnemyCharacter> EnemyCharacter = nullptr;
};

USTRUCT(meta = (DisplayName = "Has Combat Target", Category = "BlackStar|AI"))
struct BLACKSTAR_API FSTCondition_HasCombatTarget : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTCondition_HasCombatTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	
};

// ===== 마지막 위치 아는지? =====
USTRUCT()
struct FSTCondition_HasLastKnownTargetLocationInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABSEnemyCharacter> EnemyCharacter = nullptr;
};

USTRUCT(meta = (DisplayName = "Has Last Known Target Location", Category = "BlackStar|AI"))
struct BLACKSTAR_API FSTCondition_HasLastKnownTargetLocation : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTCondition_HasLastKnownTargetLocationInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	
};

// ===== 거리가 멀어졌어요 =====
USTRUCT()
struct FSTCondition_IsCombatTargetFartherThanInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context") 
	TObjectPtr<ABSEnemyCharacter> EnemyCharacter = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float Distance = 800.0f;
};

USTRUCT(meta = (DisplayName = "Is Combat Target Farther Than", Category = "BlackStar|AI"))
struct BLACKSTAR_API
FSTCondition_IsCombatTargetFartherThan : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTCondition_IsCombatTargetFartherThanInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	
};

// ===== 순찰용 스플라인 있음? =====
USTRUCT()
struct FSTCondition_HasPatrolRouteInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABSEnemyCharacter> EnemyCharacter = nullptr;
};

USTRUCT(meta = (DisplayName = "Has Patrol Route", Category = "BlackStar|AI"))
struct FSTCondition_HasPatrolRoute : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTCondition_HasPatrolRouteInstanceData;
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};