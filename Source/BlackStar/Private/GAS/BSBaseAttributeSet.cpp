
#include "GAS/BSBaseAttributeSet.h"

UBSBaseAttributeSet::UBSBaseAttributeSet()
{
	InitMaxHP(100.0f);
	InitCurrentHP(100.0f);
	InitMaxMP(100.0f);
	InitCurrentMP(100.0f);
	InitMagicPower(0.0f);
	InitDefense(0.0f);
}

void UBSBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UBSBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UBSBaseAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}
