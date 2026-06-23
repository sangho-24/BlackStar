#include "Animation/ANS_RotateToTarget.h"

#include "Interface/IAbilityAnimationInterface.h"

void UANS_RotateToTarget::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	CachedOwner = nullptr;
	CachedTarget = nullptr;

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(Owner);
	if (!Owner || !AnimChar)
	{
		return;
	}

	CachedOwner = Owner;
	CachedTarget = AnimChar->GetLockedOnTarget();
	if (!CachedTarget.IsValid())
	{
		CachedTarget = AnimChar->GetNearestTarget();
	}
}

void UANS_RotateToTarget::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!CachedOwner.IsValid() || !CachedTarget.IsValid())
	{
		return;
	}

	FVector ToTarget = CachedTarget->GetActorLocation() - CachedOwner->GetActorLocation();
	ToTarget.Z = 0.0f;
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator NewRotation = FMath::RInterpTo(CachedOwner->GetActorRotation(), ToTarget.Rotation(), FrameDeltaTime, RotationSpeed);
	CachedOwner->SetActorRotation(NewRotation);
}
