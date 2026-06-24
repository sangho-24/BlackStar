
#include "GAS/GC_ImpactBase.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

bool UGC_ImpactBase::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!IsValid(MyTarget))
	{
		return false;
	}

	UWorld* World = MyTarget->GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector ImpactLocation = ResolveImpactLocation(MyTarget, Parameters);
	const FVector ImpactNormal = ResolveImpactNormal(Parameters);
	const FRotator ImpactRotation = ImpactNormal.Rotation() + RotationOffset;
	bool bPlayedAnything = false;

	if (ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			ImpactEffect,
			ImpactLocation,
			ImpactRotation,
			ImpactEffectScale,
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
		
		bPlayedAnything = true;
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,
			ImpactSound,
			ImpactLocation);
		
		bPlayedAnything = true;
	}
	
	return bPlayedAnything;
}

FVector UGC_ImpactBase::ResolveImpactLocation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (const FHitResult* HitResult =
		Parameters.EffectContext.GetHitResult())
	{
		if (!HitResult->ImpactPoint.IsNearlyZero())
		{
			return HitResult->ImpactPoint;
		}
	}

	if (!Parameters.Location.IsNearlyZero())
	{
		return Parameters.Location;
	}

	if (bUseTargetLocationAsFallback && IsValid(MyTarget))
	{
		return MyTarget->GetActorLocation();
	}

	return FVector::ZeroVector;
}

FVector UGC_ImpactBase::ResolveImpactNormal(const FGameplayCueParameters& Parameters) const
{
	if (const FHitResult* HitResult =
		Parameters.EffectContext.GetHitResult())
	{
		if (!HitResult->ImpactNormal.IsNearlyZero())
		{
			return HitResult->ImpactNormal.GetSafeNormal();
		}
	}

	if (!Parameters.Normal.IsNearlyZero())
	{
		return Parameters.Normal.GetSafeNormal();
	}

	return FVector::UpVector;
}