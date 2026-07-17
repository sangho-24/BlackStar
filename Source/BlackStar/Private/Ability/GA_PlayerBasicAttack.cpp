#include "Ability/GA_PlayerBasicAttack.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interface/IAbilityAnimationInterface.h"
#include "Utility/BSGameplayTags.h"
#include "GameFramework/Character.h"
#include "Character/BSPlayerCharacter.h"
#include "TimerManager.h"
#include "GAS/BSBaseAttributeSet.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"


UGA_PlayerBasicAttack::UGA_PlayerBasicAttack()
{
	AbilityTag = BSGameplayTags::Ability_Player_BasicAttack;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(AbilityTag);
	SetAssetTags(AssetTags);
	
	ActivationBlockedTags.AddTag(BSGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(BSGameplayTags::State_Evading);
}

void UGA_PlayerBasicAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bSpentStamina && ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (ABSPlayerCharacter* PlayerCharacter = Cast<ABSPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->ApplyStaminaRegenDelay();
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

float UGA_PlayerBasicAttack::GetStaminaCost(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const ABSPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABSPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;

	// TODO: 캐릭터에 넣고 버프/디버프 배율에 적용
	const float Multiplier = 1.0f;
	return BaseStaminaCost * Multiplier;
}

bool UGA_PlayerBasicAttack::HasEnoughStamina(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const UBSBaseAttributeSet* AttributeSet = ASC ? ASC->GetSet<UBSBaseAttributeSet>() : nullptr;
	return AttributeSet && AttributeSet->GetCurrentStamina() >= GetStaminaCost(ActorInfo);
}

void UGA_PlayerBasicAttack::ApplyStaminaCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC || !StaminaCostEffect)
	{
		return;
	}

	FGameplayEffectSpecHandle CostSpecHandle = MakeOutgoingGameplayEffectSpec(StaminaCostEffect, GetAbilityLevel(Handle, ActorInfo));
	if (!CostSpecHandle.IsValid())
	{
		return;
	}

	const float Cost = GetStaminaCost(ActorInfo);
	CostSpecHandle.Data->SetSetByCallerMagnitude(BSGameplayTags::Data_StaminaCost, -Cost);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CostSpecHandle);
}

bool UGA_PlayerBasicAttack::StartAttack(const FGameplayEventData* TriggerEventData)
{
	bSaveCombo = false;
	bIsComboWindowOpen = false;
	bComboTransitioning = false;
	bSpentStamina = false;

	if (!HasEnoughStamina(CurrentActorInfo))
	{
		return false;
	}
	
	const bool bStarted = Super::StartAttack(TriggerEventData);
	if (!bStarted)
	{
		return false;
	}

	ApplyStaminaCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	bSpentStamina = true;

	RegisterComboEventTasks();
	return true;
}

void UGA_PlayerBasicAttack::FaceAttackDirection()
{
	ABSPlayerCharacter* Character = CurrentActorInfo ? Cast<ABSPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	if (!Character)
	{
		return;
	}

	AController* Controller = Character->GetController();
	if (!Controller)
	{
		return;
	}

	const FVector2D MoveInput = Character->GetCurrentMoveInput();
	if (MoveInput.IsNearlyZero())
	{
		// StopAttackRotation();
		return;
	}
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator CameraYawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	
	const FVector CameraForward =
	FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::X);

	const FVector CameraRight =
		FRotationMatrix(CameraYawRotation).GetUnitAxis(EAxis::Y);
	
	// 정규화 전(1,0,-1)
	FVector InputDirection =
	CameraForward * MoveInput.Y +
	CameraRight * MoveInput.X;

	InputDirection.Z = 0.0f;
	InputDirection.Normalize();
	
	const float CurrentYaw = Character->GetActorRotation().Yaw;
	const float DesiredYaw = InputDirection.Rotation().Yaw;
	
	// 현재 방향에서 목표 방향까지의 최단 부호 각도
	const float DesiredYawDelta =
		FMath::FindDeltaAngleDegrees(CurrentYaw, DesiredYaw);
	// 한 번에 돌 수 있는 총 회전각을 제한
	const float ClampedYawDelta = FMath::Clamp(
		DesiredYawDelta,
		-MaxAttackTurnAngle,
		MaxAttackTurnAngle);

	FRotator AttackTargetRotation = FRotator(
		0.0f,
		CurrentYaw + ClampedYawDelta,
		0.0f);
	
	Character->StartTurning(AttackTargetRotation,AttackTurnSpeed);
}

void UGA_PlayerBasicAttack::RegisterComboEventTasks()
{
	UAbilityTask_WaitGameplayEvent* ComboInputTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BSGameplayTags::Event_BasicAttack_ComboInput);
	ComboInputTask->EventReceived.AddDynamic(this, &UGA_PlayerBasicAttack::OnComboInput);
	ComboInputTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WindowOpenTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BSGameplayTags::Event_Combo_WindowOpen);
	WindowOpenTask->EventReceived.AddDynamic(this, &UGA_PlayerBasicAttack::OnComboWindowOpen);
	WindowOpenTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WindowCloseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BSGameplayTags::Event_Combo_WindowClose);
	WindowCloseTask->EventReceived.AddDynamic(this, &UGA_PlayerBasicAttack::OnComboWindowClose);
	WindowCloseTask->ReadyForActivation();
}

void UGA_PlayerBasicAttack::OnComboInput(FGameplayEventData Payload)
{
	if (bIsComboWindowOpen)
	{
		PlayNextCombo();
		return;
	}

	bSaveCombo = true;
}

void UGA_PlayerBasicAttack::OnComboWindowOpen(FGameplayEventData Payload)
{
	bIsComboWindowOpen = true;

	if (bSaveCombo)
	{
		bSaveCombo = false;
		PlayNextCombo();
	}
}

void UGA_PlayerBasicAttack::OnComboWindowClose(FGameplayEventData Payload)
{
	bIsComboWindowOpen = false;
	bSaveCombo = false;
}

void UGA_PlayerBasicAttack::PlayNextCombo()
{
	if (!HasEnoughStamina(CurrentActorInfo))
	{
		bSaveCombo = false;
		return;
	}
	
	AActor* AvatarActor = CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;
	IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(AvatarActor);
	if (!AnimChar)
	{
		return;
	}
	
	const FName NextSection = AnimChar->GetNextComboSection();
	UAnimMontage* NextMontage = AnimChar->GetNextComboMontage();
	if (!NextMontage && NextSection.IsNone())
	{
		return;
	}
	
	// ===== 현재 몽타주 확인
	UAnimInstance* AnimInstance = nullptr;
	UAnimMontage* ActiveMontage = nullptr;
	if (!NextMontage)
	{
		USkeletalMeshComponent* SKMesh = AvatarActor ? AvatarActor->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
		AnimInstance = SKMesh ? SKMesh->GetAnimInstance() : nullptr;
		ActiveMontage = GetCurrentMontage();
		
		if (!AnimInstance || !ActiveMontage || !ActiveMontage->IsValidSectionName(NextSection))
		{
			return;
		}
	}
	
	// 콤보 실행
	ApplyStaminaCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	bSpentStamina = true;
	bIsComboWindowOpen = false;
	bSaveCombo = false;

	AttackTarget = ResolveAttackTarget();
	if (AttackTarget)
	{
		FaceTarget(AttackTarget);
	}
	else
	{
		FaceAttackDirection();
	}


	if (NextMontage)
	{
		bComboTransitioning = true;
		if (!NextSection.IsNone() && NextMontage->IsValidSectionName(NextSection))
		{
			PlayMontage(NextMontage, NextSection);
		}
		else
		{
			PlayMontage(NextMontage);
		}
		return;
	}
	AnimInstance->Montage_JumpToSection(NextSection, ActiveMontage);
}

void UGA_PlayerBasicAttack::OnMontageCancelled()
{
	if (bComboTransitioning)
	{
		bComboTransitioning = false;
		return;
	}

	Super::OnMontageCancelled();
}
