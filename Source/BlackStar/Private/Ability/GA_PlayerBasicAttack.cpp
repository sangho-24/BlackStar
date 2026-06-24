#include "Ability/GA_PlayerBasicAttack.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interface/IAbilityAnimationInterface.h"
#include "Utility/BSGameplayTags.h"
#include "GameFramework/Character.h"
#include "Character/BSPlayerCharacter.h"
#include "TimerManager.h"


UGA_PlayerBasicAttack::UGA_PlayerBasicAttack()
{
	AbilityTag = BSGameplayTags::Ability_Player_BasicAttack;
	ActivationOwnedTags.AddTag(BSGameplayTags::State_Combo);

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(AbilityTag);
	SetAssetTags(AssetTags);
}

bool UGA_PlayerBasicAttack::StartAttack(const FGameplayEventData* TriggerEventData)
{
	bSaveCombo = false;
	bIsComboWindowOpen = false;
	bComboTransitioning = false;

	const bool bStarted = Super::StartAttack(TriggerEventData);
	if (bStarted)
	{
		RegisterComboEventTasks();
	}

	return bStarted;
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
	AActor* AvatarActor = CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;
	IAbilityAnimationInterface* AnimChar = Cast<IAbilityAnimationInterface>(AvatarActor);
	if (!AnimChar)
	{
		return;
	}

	bIsComboWindowOpen = false;
	bSaveCombo = false;
	bComboTransitioning = true;

	AttackTarget = ResolveAttackTarget();
	if (AttackTarget)
	{
		FaceTarget(AttackTarget);
	}
	else
	{
		FaceAttackDirection();
	}

	const FName NextSection = AnimChar->GetNextComboSection();
	UAnimMontage* NextMontage = AnimChar->GetNextComboMontage();
	if (NextMontage)
	{
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

	if (!NextSection.IsNone())
	{
		USkeletalMeshComponent* Mesh = AvatarActor ? AvatarActor->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
		UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->Montage_JumpToSection(NextSection);
		}
		return;
	}
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
