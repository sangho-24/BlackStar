#include "GAS/BSAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayAbilitySpec.h"
#include "Utility/BSGameplayTags.h"

void UBSAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (!Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		AbilitySpecInputPressed(Spec);

		if (Spec.IsActive())
		{
			if (InputTag == BSGameplayTags::Input_BasicAttack)
			{
				FGameplayEventData EventData;
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), BSGameplayTags::Event_BasicAttack_ComboInput, EventData);
			}
			continue;
		}

		TryActivateAbility(Spec.Handle);
	}
}

void UBSAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(Spec);
		}
	}
}
