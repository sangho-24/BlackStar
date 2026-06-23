
#include "GAS/BSBaseAttributeSet.h"
#include "GameplayEffectExtension.h"

UBSBaseAttributeSet::UBSBaseAttributeSet()
{
	InitMaxHP(100.0f);
	InitCurrentHP(100.0f);
	InitMaxMP(100.0f);
	InitCurrentMP(100.0f);
	InitMagicPower(0.0f);
	InitAttackPower(0.0f);
	InitDefense(0.0f);
}

void UBSBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetCurrentHPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHP());
	}
	else if (Attribute == GetCurrentMPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMP());
	}
	else if (Attribute == GetMaxHPAttribute() || Attribute == GetMaxMPAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UBSBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetCurrentHPAttribute())
	{
		SetCurrentHP(FMath::Clamp(GetCurrentHP(), 0.0f, GetMaxHP()));
	}
	else if (Data.EvaluatedData.Attribute == GetCurrentMPAttribute())
	{
		SetCurrentMP(FMath::Clamp(GetCurrentMP(), 0.0f, GetMaxMP()));
	}
}

void UBSBaseAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHPAttribute())
	{
		SetCurrentHP(FMath::Clamp(GetCurrentHP(), 0.0f, GetMaxHP()));
	}
	else if (Attribute == GetMaxMPAttribute())
	{
		SetCurrentMP(FMath::Clamp(GetCurrentMP(), 0.0f, GetMaxMP()));
	}
}
