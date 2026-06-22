#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BSBaseAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class BLACKSTAR_API UBSBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats")
	FGameplayAttributeData CurrentHP;
	ATTRIBUTE_ACCESSORS(UBSBaseAttributeSet, CurrentHP)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats")
	FGameplayAttributeData MaxHP;
	ATTRIBUTE_ACCESSORS(UBSBaseAttributeSet, MaxHP)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats")
	FGameplayAttributeData CurrentMP;
	ATTRIBUTE_ACCESSORS(UBSBaseAttributeSet, CurrentMP)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats")
	FGameplayAttributeData MaxMP;
	ATTRIBUTE_ACCESSORS(UBSBaseAttributeSet, MaxMP)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats")
	FGameplayAttributeData MagicPower;
	ATTRIBUTE_ACCESSORS(UBSBaseAttributeSet, MagicPower)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UBSBaseAttributeSet, Defense)

private:
	float CachedHPPercent = 1.0f;

public:
	UBSBaseAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
};
