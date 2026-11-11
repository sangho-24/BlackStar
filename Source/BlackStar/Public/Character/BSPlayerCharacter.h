#pragma once

#include "CoreMinimal.h"
#include "Character/BSBaseCharacter.h"
#include "InputActionValue.h"
#include "BSPlayerCharacter.generated.h"

class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UGameplayEffect;
class UBSWeaponComponent;
class UWidgetComponent;

UCLASS()
class BLACKSTAR_API ABSPlayerCharacter : public ABSBaseCharacter
{
	GENERATED_BODY()

public:
	ABSPlayerCharacter();

protected:
	// ===== 입력 관련 =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	TObjectPtr<UInputAction> MoveInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	TObjectPtr<UInputAction> LookInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	TObjectPtr<UInputAction> ZoomInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	TObjectPtr<UInputAction> LockOnInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	TObjectPtr<UInputAction> JumpInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	TObjectPtr<UInputAction> EvadeInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	TObjectPtr<UInputAction> DashInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	TObjectPtr<UInputAction> BasicSkillInput;

	FVector2D CurrentMoveInput = FVector2D::ZeroVector;

	// ===== 카메라 관련 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float MinArmLength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float MaxArmLength = 600.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Zoom")
	float DesiredArmLength = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float ZoomStep = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float ZoomInterpSpeed = 12.0f;

	UPROPERTY()
	TObjectPtr<UAnimMontage> NextComboMontage = nullptr;

	FName NextComboSection = NAME_None;

	bool bIsZoomInterpolating = false;

	// ===== 락온 관련 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn|Indicator")
	TObjectPtr<UWidgetComponent> LockOnIndicatorComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn|Indicator")
	FVector LockOnIndicatorOffset = FVector(0.0f, 0.0f, 40.0f);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<ABSBaseCharacter> LockOnTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	float LockOnSearchRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	float LockOnMaxAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	float LockOnAngleWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	float LockOnDistanceWeight = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	float LockOnInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	TEnumAsByte<ECollisionChannel> LockOnObjectChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	TEnumAsByte<ECollisionChannel> LockOnTraceChannel = ECC_Visibility;

	// 입력 누적량 기준. 화면 픽셀 단위는 아님.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn|Switch", meta = (ClampMin = "0.01"))
	float LockOnSwitchThreshold = 8.0f;

	// 이 시간 안의 같은 방향 입력을 누적.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn|Switch", meta = (ClampMin = "0.01"))
	float LockOnSwitchInputWindow = 0.2f;

	// 다시 전환하려면 좌우 움직임을 잠깐 멈춰야 함.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn|Switch", meta = (ClampMin = "0.01"))
	float LockOnSwitchNeutralTime = 0.12f;

	// 전환 시도 사이의 최소 간격.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn|Switch", meta = (ClampMin = "0.0"))
	float LockOnSwitchCooldown = 0.25f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn|Debug")
	bool bDrawLockOnDebug = false;

	FTimerHandle LockOnUpdateTimerHandle;
	
	bool bLockOnActive = false;
	bool bLockOnSwitchConsumed = false;

	float LockOnSwitchAccumulated = 0.0f;
	double LockOnGestureStartTime = 0.0f;
	double LastLockOnSwitchInputTime = -1.0f;
	double NextLockOnSwitchTime = 0.0f;

	// 애니메이션 중복 방지 플래그
	bool bPlayingUnlockAnimation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	float LockOnUpdateInterval = 0.5f;
	
	// ===== 애니메이션 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Setup")
	TSubclassOf<UGameplayEffect> StaminaRegenEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Setup")
	TSubclassOf<UGameplayEffect> StaminaRegenDelayEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade|Animation")
	float EvadeMontageBlendOutTime = 0.08f;
	
	// ===== 장비 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UBSWeaponComponent> WeaponComponent;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

	void MoveAction(const FInputActionValue &Value);
	void StopMoveAction(const FInputActionValue &Value);
	void LookAction(const FInputActionValue &Value);
	void ZoomAction(const FInputActionValue &Value);
	void JumpAction();
	void EvadeAction();
	void DashPressedAction();
	void DashReleasedAction();
	void StopDashAction();
	void StopJumpingAction();
	void BasicSkillAction();
	void LockOnAction();

	void UpdateLockOnTarget();
	ABSBaseCharacter *FindBestLockOnTarget() const;
	bool IsLockOnTargetable(ABSBaseCharacter *Candidate) const;
	bool HasLineOfSightToTarget(ABSBaseCharacter *Candidate) const;
	void StartLockOnUpdateTimer();
	void StopLockOnUpdateTimer();

	void StopEvadeMontageTail();
	
	void UpdateLockOnIndicator();
	void HandleLockOnSwitchInput(float MouseDeltaX);
	void ResetLockOnSwitchInput();
	ABSBaseCharacter* FindLockOnTargetInDirection(int32 Direction) const;
	
	virtual void OnDeathStarted(AActor *Killer) override;
	virtual void OnDeathFinished(AActor *Killer) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "LockOn")
	void OnLockOnIndicatorStateChanged(bool bLockedOn);
	
public:
	FVector2D GetCurrentMoveInput() const { return CurrentMoveInput; }
	void ApplyStaminaRegenDelay();
	float GetEvadeMontageBlendOutTime() const { return EvadeMontageBlendOutTime; }
	
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void FinishLockOnIndicatorFadeOut();
	
	// ===== 인터페이스 함수 =====
	UFUNCTION(BlueprintPure)
	virtual AActor *GetCombatTarget() const override;
	virtual void SetCombatTarget(AActor *NewTarget) override;
	virtual void ClearCombatTarget() override;

	virtual void SetNextComboMontage(UAnimMontage *Montage) override;
	virtual UAnimMontage *GetNextComboMontage() const override;
	virtual void SetNextComboSection(FName SectionName) override;
	virtual FName GetNextComboSection() const override;
	

	FORCEINLINE USpringArmComponent *GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent *GetFollowCamera() const { return FollowCamera; }
};

