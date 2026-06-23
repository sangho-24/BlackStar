#include "Ability/GA_EnemyBasicAttack.h"

#include "Utility/BSGameplayTags.h"

UGA_EnemyBasicAttack::UGA_EnemyBasicAttack()
{
	AbilityTag = BSGameplayTags::Ability_Enemy_BasicAttack;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(AbilityTag);
	SetAssetTags(AssetTags);
}
