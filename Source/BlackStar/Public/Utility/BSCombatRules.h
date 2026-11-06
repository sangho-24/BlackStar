#pragma once

#include "AbilitySystemComponent.h"
#include "Utility/BSGameplayTags.h"

namespace BSCombatRules
{
	inline bool CanReceiveDamage(const UAbilitySystemComponent* TargetASC, const FGameplayTag& DamageType)
	{
		if (!TargetASC)
		{
			return false;
		}

		if (TargetASC->HasMatchingGameplayTag(BSGameplayTags::State_Dead) || TargetASC->HasMatchingGameplayTag(BSGameplayTags::State_Invulnerable))
		{
			return false;
		}

		if (DamageType.MatchesTag(BSGameplayTags::DamageType_Melee) && TargetASC->HasMatchingGameplayTag(BSGameplayTags::State_Immune_Melee))
		{
			return false;
		}

		if (DamageType.MatchesTag(BSGameplayTags::DamageType_Fire) && TargetASC->HasMatchingGameplayTag(BSGameplayTags::State_Immune_Fire))
		{
			return false;
		}

		if (DamageType.MatchesTag(BSGameplayTags::DamageType_Ice) && TargetASC->HasMatchingGameplayTag(BSGameplayTags::State_Immune_Ice))
		{
			return false;
		}

		// 슈퍼아머는 피해를 차단하지 않음.
		return true;
	}
}