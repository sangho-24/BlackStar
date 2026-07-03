// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GC_ImpactBase.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS()
class BLACKSTAR_API UGC_ImpactBase : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Impact")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Impact")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Impact")
	FVector ImpactEffectScale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Impact")
	FRotator RotationOffset = FRotator::ZeroRotator;
	
	// Parameters.Location이 유효하지 않을 때 Target 위치를 대신 사용할지 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Impact")
	bool bUseTargetLocationAsFallback = true;
	
	// ===== 카메라 셰이크 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Camera Shake")
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Camera Shake", meta = (ClampMin = "0.0"))
	float CameraShakeScale = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Camera Shake", meta = (ClampMin = "0.0"))
	float CameraShakeInnerRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Camera Shake", meta = (ClampMin = "0.0"))
	float CameraShakeOuterRadius = 0.0f;

	// ===== 히트 스탑 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Hit Stop", meta = (ClampMin = "0.0"))
	float HitStopDuration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Hit Stop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HitStopTimeDilation = 0.0f;
	
public:
	virtual bool OnExecute_Implementation(
		AActor* MyTarget,
		const FGameplayCueParameters& Parameters) const override;
	
protected:
	FVector ResolveImpactLocation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const;

	FVector ResolveImpactNormal(const FGameplayCueParameters& Parameters) const;
	
};
