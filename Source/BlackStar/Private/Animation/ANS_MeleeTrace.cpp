#include "Animation/ANS_MeleeTrace.h"


#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interface/IAbilityAnimationInterface.h"
#include "Utility/BSGameplayTags.h"

void UANS_MeleeTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
	{
		return;
	}

	if (IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(Owner))
	{
		FMeleeTraceData Data;
		Data.StartSocketName = StartSocketName;
		Data.EndSocketName = EndSocketName;
		Data.TraceRadius = TraceRadius;
		Data.ExtraLength = ExtraLength;
		Data.DamageMultiplier = DamageMultiplier;
		Data.HitCueTag = HitCueTag;
		Data.HitReactionData = HitReactionData;
		AnimChar->SetMeleeTraceData(Data);
	}

	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, BSGameplayTags::Event_MeleeTrace_Start, EventData);
}

void UANS_MeleeTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
	{
		return;
	}

	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, BSGameplayTags::Event_MeleeTrace_End, EventData);
}
