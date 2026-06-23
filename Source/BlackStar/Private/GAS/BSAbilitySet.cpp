#include "GAS/BSAbilitySet.h"

#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"

void UBSAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, TArray<FGameplayAbilitySpecHandle>* OutGrantedHandles) const
{
	if (!ASC)
		return;

	for (const FBSAbilityGrantInfo& AbilityInfo : GrantedAbilities)
	{
		if (!AbilityInfo.AbilityClass)
			continue;

		FGameplayAbilitySpec AbilitySpec(AbilityInfo.AbilityClass, AbilityInfo.AbilityLevel);
		if (AbilityInfo.InputTag.IsValid())
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityInfo.InputTag);

		const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
		if (OutGrantedHandles)
			OutGrantedHandles->Add(Handle);
	}
}
