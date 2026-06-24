#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GC_LoopingEffectBase.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UAudioComponent;
class USoundBase;

UCLASS()
class BLACKSTAR_API AGC_LoopingEffectBase : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Looping")
	TObjectPtr<UNiagaraSystem> LoopingEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Looping")
	TObjectPtr<USoundBase> LoopingSound;

	// 이펙트를 부착할 Target의 소켓 또는 본 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Looping")
	FName AttachSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Looping")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Looping")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Looping")
	FVector EffectScale = FVector::OneVector;

	// 제거 시 Niagara를 즉시 파괴하지 않고 자연스럽게 종료할지 여부
	// 방어막이 깨지거나 효과를 즉시 지워야 할 때 false
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Looping")
	bool bAllowEffectToFinish = true;

	// 제거 시 사운드 Fade Out 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Cue|Looping", meta = (ClampMin = "0.0"))
	float SoundFadeOutDuration = 0.2f;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> SpawnedNiagaraComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> SpawnedAudioComponent;
	
public:
	AGC_LoopingEffectBase();
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

private:
	void StartEffects(AActor* MyTarget);
	void StopEffects();
};
