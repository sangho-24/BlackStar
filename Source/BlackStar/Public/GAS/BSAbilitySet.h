#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "BSAbilitySet.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FBSAbilityGrantInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS", meta = (ClampMin = "1"))
	int32 AbilityLevel = 1;
};

UCLASS(BlueprintType)
class BLACKSTAR_API UBSAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<FBSAbilityGrantInfo> GrantedAbilities;

	void GiveToAbilitySystem(UAbilitySystemComponent* ASC, TArray<FGameplayAbilitySpecHandle>* OutGrantedHandles = nullptr) const;
};
