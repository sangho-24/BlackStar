#pragma once

#include "CoreMinimal.h"
#include "Character/BSBaseCharacter.h"
#include "InputActionValue.h"
#include "BSPlayerCharacter.generated.h"

class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<AActor> LockOnTarget = nullptr;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn|Debug")
	bool bDrawLockOnDebug = false;
	
	FTimerHandle LockOnUpdateTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	float LockOnUpdateInterval = 0.5f;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void MoveAction(const FInputActionValue& Value);
	void StopMoveAction(const FInputActionValue& Value);
	void LookAction(const FInputActionValue& Value);
	void ZoomAction(const FInputActionValue& Value);
	void JumpAction();
	void StopJumpingAction();
	void BasicSkillAction();
	void LockOnAction();
	
	void ClearLockOn();
	void UpdateLockOnTarget();
	AActor* FindBestLockOnTarget() const;
	bool IsLockOnTargetable(AActor* Candidate) const;
	bool HasLineOfSightToTarget(AActor* Candidate) const;
	void StartLockOnUpdateTimer();
	void StopLockOnUpdateTimer();

public:

	FVector2D GetCurrentMoveInput() const { return CurrentMoveInput; }
	// ===== 인터페이스 함수 =====
	virtual AActor* GetCombatTarget() const override;
	
	virtual void SetNextComboMontage(UAnimMontage* Montage) override;
	virtual UAnimMontage* GetNextComboMontage() const override;
	virtual void SetNextComboSection(FName SectionName) override;
	virtual FName GetNextComboSection() const override;

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
