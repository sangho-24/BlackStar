#include "Animation/AN_SpawnProjectile.h"

#include "Ability/BSAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interface/IAbilityAnimationInterface.h"
#include "Utility/BSGameplayTags.h"

void UAN_SpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
	{
		return;
	}

	if (IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(Owner))
	{
		FProjectileData Data;
		Data.SpawnSocketName = SpawnSocketName;
		Data.ProjectileClass = ProjectileClass;
		Data.DamageMultiplier = DamageMultiplier;
		AnimChar->SetProjectileData(Data);
	}

	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, BSGameplayTags::Event_SpawnProjectile, EventData);
}
