#include "Character/BSPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GAS/BSAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Utility/BSGameplayTags.h"

ABSPlayerCharacter::ABSPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = DesiredArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ABSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CameraBoom)
    	DesiredArmLength = CameraBoom->TargetArmLength;
}

void ABSPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsZoomInterpolating && CameraBoom)
	{
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, DesiredArmLength, DeltaTime, ZoomInterpSpeed);
		if (FMath::IsNearlyEqual(CameraBoom->TargetArmLength, DesiredArmLength, 1.0f))
		{
			CameraBoom->TargetArmLength = DesiredArmLength;
			bIsZoomInterpolating = false;
		}
	}
}

void ABSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (MoveInput)
	{
		EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Triggered, this, &ABSPlayerCharacter::MoveAction);
	}
	if (LookInput)
	{
		EnhancedInputComponent->BindAction(LookInput, ETriggerEvent::Triggered, this, &ABSPlayerCharacter::LookAction);
	}
	if (ZoomInput)
	{
		EnhancedInputComponent->BindAction(ZoomInput, ETriggerEvent::Triggered, this, &ABSPlayerCharacter::ZoomAction);
	}
	if (JumpInput)
	{
		EnhancedInputComponent->BindAction(JumpInput, ETriggerEvent::Started, this, &ABSPlayerCharacter::JumpAction);
		EnhancedInputComponent->BindAction(JumpInput, ETriggerEvent::Completed, this, &ABSPlayerCharacter::StopJumpingAction);
	}
	if (BasicSkillInput)
	{
		EnhancedInputComponent->BindAction(BasicSkillInput, ETriggerEvent::Started, this, &ABSPlayerCharacter::BasicSkillAction);
	}
	if (LockOnInput)
	{
		EnhancedInputComponent->BindAction(LockOnInput, ETriggerEvent::Started, this, &ABSPlayerCharacter::LockOnAction);
	}
}

void ABSPlayerCharacter::MoveAction(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ABSPlayerCharacter::LookAction(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ABSPlayerCharacter::ZoomAction(const FInputActionValue& Value)
{
	if (!Controller || !CameraBoom)
		return;
	
	const float ZoomAxis = Value.Get<float>();

	DesiredArmLength = FMath::Clamp(DesiredArmLength - ZoomAxis * ZoomStep, MinArmLength, MaxArmLength);
	bIsZoomInterpolating = true;
}

void ABSPlayerCharacter::JumpAction()
{
	Jump();
}

void ABSPlayerCharacter::StopJumpingAction()
{
	StopJumping();
}

void ABSPlayerCharacter::BasicSkillAction()
{
	UBSAbilitySystemComponent* BSASC = GetBSAbilitySystemComponent();
	if (!BSASC)
	{
		return;
	}

	BSASC->AbilityInputTagPressed(BSGameplayTags::Input_BasicAttack);
}

void ABSPlayerCharacter::LockOnAction()
{
}

void ABSPlayerCharacter::SetNextComboMontage(UAnimMontage* Montage)
{
	NextComboMontage = Montage;
}

UAnimMontage* ABSPlayerCharacter::GetNextComboMontage() const
{
	return NextComboMontage;
}

void ABSPlayerCharacter::SetNextComboSection(FName SectionName)
{
	NextComboSection = SectionName;
}

FName ABSPlayerCharacter::GetNextComboSection() const
{
	return NextComboSection;
}
