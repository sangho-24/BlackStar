#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "BSDamageExecution.generated.h"

UCLASS()
class BLACKSTAR_API UBSDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UBSDamageExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};