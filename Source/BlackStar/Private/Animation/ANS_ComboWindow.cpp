#include "Animation/ANS_ComboWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interface/IAbilityAnimationInterface.h"
#include "Utility/BSGameplayTags.h"

void UANS_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
	{
		return;
	}

	if (IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(Owner))
	{
		AnimChar->SetNextComboMontage(NextComboMontage);
		AnimChar->SetNextComboSection(NextComboSection);
	}

	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, BSGameplayTags::Event_Combo_WindowOpen, EventData);
}

void UANS_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
	{
		return;
	}

	if (IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(Owner))
	{
		AnimChar->SetNextComboMontage(nullptr);
		AnimChar->SetNextComboSection(NAME_None);
	}

	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, BSGameplayTags::Event_Combo_WindowClose, EventData);
}
