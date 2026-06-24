
#include "GAS/GC_LoopingEffectBase.h"

#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AGC_LoopingEffectBase::AGC_LoopingEffectBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// 동일한 Target에 Cue가 다시 적용될 때 기존 Cue Actor를 재사용한다.
	bUniqueInstancePerInstigator = false;
	bUniqueInstancePerSourceObject = false;

	// 즉시 파괴되지 않고 0.5초 뒤에 제거(이펙트 마무리할 시간을 줌)
	bAutoDestroyOnRemove = true;
	AutoDestroyDelay = 0.5f;
}

bool AGC_LoopingEffectBase::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!IsValid(MyTarget))
	{
		return false;
	}

	StartEffects(MyTarget);
	return true;
}

bool AGC_LoopingEffectBase::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!IsValid(MyTarget))
	{
		return false;
	}
	
	// 복제 또는 예측 순서에 따라 WhileActive가 먼저 도착해도 이펙트가 정상적으로 존재하도록 보완한다.
	StartEffects(MyTarget);
	return true;
}

bool AGC_LoopingEffectBase::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	StopEffects();
	return true;
}

void AGC_LoopingEffectBase::StartEffects(AActor* MyTarget)
{
	if (!IsValid(MyTarget))
	{
		return;
	}

	USceneComponent* AttachComponent = MyTarget->GetRootComponent();

	if (const ACharacter* Character = Cast<ACharacter>(MyTarget))
	{
		if (Character->GetMesh())
		{
			AttachComponent = Character->GetMesh();
		}
	}

	if (!IsValid(AttachComponent))
	{
		return;
	}

	if (LoopingEffect && !IsValid(SpawnedNiagaraComponent))
	{
		SpawnedNiagaraComponent =
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				LoopingEffect,
				AttachComponent,
				AttachSocketName,
				LocationOffset,
				RotationOffset,
				EffectScale,
				EAttachLocation::KeepRelativeOffset,
				true,	// TODO, 이거 나이아가라 컴포넌트 제거 관련 테스트 필요
				ENCPoolMethod::None,
				true);
	}

	if (LoopingSound && !IsValid(SpawnedAudioComponent))
	{
		SpawnedAudioComponent =
			UGameplayStatics::SpawnSoundAttached(
				LoopingSound,
				AttachComponent,
				AttachSocketName,
				LocationOffset,
				RotationOffset,
				EAttachLocation::KeepRelativeOffset,
				false,
				1.0f,
				1.0f,
				0.0f,
				nullptr,
				nullptr,
				false);
	}
}

void AGC_LoopingEffectBase::StopEffects()
{
	if (IsValid(SpawnedNiagaraComponent))
	{
		if (bAllowEffectToFinish)
		{
			SpawnedNiagaraComponent->Deactivate();
		}
		else
		{
			SpawnedNiagaraComponent->DestroyComponent();
		}

		SpawnedNiagaraComponent = nullptr;
	}

	if (IsValid(SpawnedAudioComponent))
	{
		if (SoundFadeOutDuration > 0.0f)
		{
			SpawnedAudioComponent->FadeOut(
				SoundFadeOutDuration,
				0.0f);
		}
		else
		{
			SpawnedAudioComponent->Stop();
		}

		SpawnedAudioComponent = nullptr;
	}
}