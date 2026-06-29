#include "Animation/ANS_RotateToTarget.h"

#include "Interface/ICombatInterface.h"

void UANS_RotateToTarget::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float TotalDuration, const FAnimNotifyEventReference &EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	CachedOwner = nullptr;
	CachedTarget = nullptr;

	AActor *Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	ICombatInterface *CombatChar = Cast<ICombatInterface>(Owner);
	if (!Owner || !CombatChar)
	{
		return;
	}
	CachedOwner = Owner;
	CachedTarget = CombatChar->GetCombatTarget();
}

void UANS_RotateToTarget::NotifyTick(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
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
