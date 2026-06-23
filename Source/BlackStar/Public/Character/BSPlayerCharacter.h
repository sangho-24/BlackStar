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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void MoveAction(const FInputActionValue& Value);
	void LookAction(const FInputActionValue& Value);
	void ZoomAction(const FInputActionValue& Value);
	void JumpAction();
	void StopJumpingAction();
	void BasicSkillAction();
	void LockOnAction();

public:
	virtual void SetNextComboMontage(UAnimMontage* Montage) override;
	virtual UAnimMontage* GetNextComboMontage() const override;
	virtual void SetNextComboSection(FName SectionName) override;
	virtual FName GetNextComboSection() const override;

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
