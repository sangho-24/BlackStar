
#include "Animation/AN_DeathFinished.h"
#include "Character/BSBaseCharacter.h"

void UAN_DeathFinished::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (ABSBaseCharacter* Character = Cast<ABSBaseCharacter>(MeshComp->GetOwner()))
	{
		Character->NotifyDeathAnimationFinished();
	}
}