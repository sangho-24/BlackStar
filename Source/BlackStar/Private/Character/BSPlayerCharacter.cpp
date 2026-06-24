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
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

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
	
	// 락온 카메라 보간
	if (LockOnTarget && Controller)
	{
		const FVector ToTarget = LockOnTarget->GetActorLocation() - GetActorLocation();
		if (!ToTarget.IsNearlyZero())
		{
			FRotator TargetRotation = ToTarget.Rotation();
			TargetRotation.Pitch -= 30.0f;

			const FRotator CurrentRotation = Controller->GetControlRotation();
			const FRotator NewRotation = FMath::RInterpTo(
				CurrentRotation,
				TargetRotation,
				DeltaTime,
				LockOnInterpSpeed
			);

			Controller->SetControlRotation(NewRotation);
		}
	}
	// 카메라 줌 보간
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
		EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Completed,this, &ABSPlayerCharacter::StopMoveAction);
		EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Canceled,this, &ABSPlayerCharacter::StopMoveAction);
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
	CurrentMoveInput = MovementVector;
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

void ABSPlayerCharacter::StopMoveAction(const FInputActionValue& Value)
{
	CurrentMoveInput = FVector2D::ZeroVector;
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
	if (LockOnTarget)
	{
		ClearLockOn();
		return;
	}
	LockOnTarget = FindBestLockOnTarget();
	if (LockOnTarget)
	{
		StartLockOnUpdateTimer();
	}
}

void ABSPlayerCharacter::ClearLockOn()
{
	LockOnTarget = nullptr;
	StopLockOnUpdateTimer();
}

void ABSPlayerCharacter::UpdateLockOnTarget()
{
	if (!LockOnTarget)
	{
		return;
	}

	if (!IsValid(LockOnTarget))
	{
		ClearLockOn();
		return;
	}

	const float Distance = FVector::Dist(GetActorLocation(), LockOnTarget->GetActorLocation());
	if (Distance > LockOnSearchRadius)
	{
		ClearLockOn();
		return;
	}

	if (!HasLineOfSightToTarget(LockOnTarget))
	{
		ClearLockOn();
	}
}

AActor* ABSPlayerCharacter::FindBestLockOnTarget() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	struct FLockOnCandidate
	{
		TWeakObjectPtr<AActor> Actor;
		float Score = 0.0f;
	};
	
	const FVector SearchLocation = GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(LockOnObjectChannel);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHasOverlap = World->OverlapMultiByObjectType(
		Overlaps,
		SearchLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(LockOnSearchRadius),
		QueryParams
	);

	if (bDrawLockOnDebug)
	{
		DrawDebugSphere(World, SearchLocation, LockOnSearchRadius, 32, FColor::Cyan, false, 1.0f);
	}

	if (!bHasOverlap)
	{
		return nullptr;
	}

	const FVector ViewLocation = FollowCamera ? FollowCamera->GetComponentLocation() : GetActorLocation();
	const FVector ViewForward = FollowCamera ? FollowCamera->GetForwardVector() : GetActorForwardVector();
	const float MinDot = FMath::Cos(FMath::DegreesToRadians(LockOnMaxAngle));

	TArray<FLockOnCandidate> Candidates;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == this)
		{
			continue;
		}

		if (!IsLockOnTargetable(Candidate))
		{
			continue;
		}

		const FVector TargetLocation = Candidate->GetActorLocation();
		const FVector ToTarget = TargetLocation - ViewLocation;
		const float Distance = ToTarget.Size();

		if (Distance <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const FVector Direction = ToTarget / Distance;
		const float Dot = FVector::DotProduct(ViewForward, Direction);

		if (Dot < MinDot)
		{
			continue;
		}
		// 점수 낮을수록 우선순위. Dot점수 최고점 0으로.
		const float AngleScore = 1.0f - Dot;
		const float Score = AngleScore * LockOnAngleWeight + Distance * LockOnDistanceWeight;
		Candidates.Add({ Candidate, Score });
	}

	Candidates.Sort([](const FLockOnCandidate& A, const FLockOnCandidate& B)
	{
		return A.Score < B.Score;
	});

	for (const FLockOnCandidate& Candidate : Candidates)
	{
		AActor* CandidateActor = Candidate.Actor.Get();
		if (!CandidateActor)
		{
			continue;
		}

		if (HasLineOfSightToTarget(CandidateActor))
		{
			return CandidateActor;
		}
	}

	return nullptr;
}


bool ABSPlayerCharacter::IsLockOnTargetable(AActor* Candidate) const
{
	if (!Candidate || Candidate == this)
	{
		return false;
	}

	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate))
	{
		return false;
	}

	return true;
}


bool ABSPlayerCharacter::HasLineOfSightToTarget(AActor* Candidate) const
{
	if (!Candidate)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector Start = FollowCamera ? FollowCamera->GetComponentLocation() : GetActorLocation();
	const FVector End = Candidate->GetActorLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		LockOnTraceChannel,
		QueryParams
	);

	const bool bHasSight = !bHit || HitResult.GetActor() == Candidate;

	if (bDrawLockOnDebug)
	{
		DrawDebugLine(World, Start, End, bHasSight ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.5f);
	}

	return bHasSight;
}

void ABSPlayerCharacter::StartLockOnUpdateTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		LockOnUpdateTimerHandle,
		this,
		&ABSPlayerCharacter::UpdateLockOnTarget,
		LockOnUpdateInterval,
		true
	);
}

void ABSPlayerCharacter::StopLockOnUpdateTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(LockOnUpdateTimerHandle);
}

AActor* ABSPlayerCharacter::GetCombatTarget() const
{
	return LockOnTarget;
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
