#include "Character/BSPlayerCharacter.h"
#include "Character/BSEnemyCharacter.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Component/BSWeaponComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "GAS/BSAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Utility/BSGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "GameplayEffect.h"
#include "Animation/AnimInstance.h"

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
	
	WeaponComponent = CreateDefaultSubobject<UBSWeaponComponent>(TEXT("WeaponComponent"));
	
	LockOnIndicatorComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnIndicatorComponent"));
	LockOnIndicatorComponent->SetupAttachment(RootComponent);
	LockOnIndicatorComponent->SetAbsolute(true, false, false);
	LockOnIndicatorComponent->SetWidgetSpace(EWidgetSpace::Screen);
	LockOnIndicatorComponent->SetDrawSize(FVector2D(24.0f, 24.0f));
	LockOnIndicatorComponent->SetPivot(FVector2D(0.5f, 0.5f));
	LockOnIndicatorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LockOnIndicatorComponent->SetGenerateOverlapEvents(false);
	LockOnIndicatorComponent->SetVisibility(false);
}

void ABSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (CameraBoom)
	{
		DesiredArmLength = CameraBoom->TargetArmLength;
	}
	
	if (AbilitySystemComponent && StaminaRegenEffect)
	{
		FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
			StaminaRegenEffect,1.0f,ContextHandle);

		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void ABSPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearCombatTarget();
	Super::EndPlay(EndPlayReason);
}

void ABSPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 락온 카메라 보간.
	if (bLockOnActive && IsValid(LockOnTarget) && Controller)
	{
		const FVector ToTarget = LockOnTarget->GetActorLocation() - GetActorLocation();

		if (!ToTarget.IsNearlyZero())
		{
			FRotator TargetRotation = ToTarget.Rotation();
			TargetRotation.Pitch -= 30.0f;

			const FRotator NewRotation = FMath::RInterpTo(
				Controller->GetControlRotation(),
				TargetRotation,
				DeltaTime,
				LockOnInterpSpeed);

			Controller->SetControlRotation(NewRotation);
		}
	}

		// 카메라 줌 보간.
		if (bIsZoomInterpolating && CameraBoom)
		{
			CameraBoom->TargetArmLength = FMath::FInterpTo(
				CameraBoom->TargetArmLength,
				DesiredArmLength,
				DeltaTime,
				ZoomInterpSpeed);

			if (FMath::IsNearlyEqual(
					CameraBoom->TargetArmLength,
					DesiredArmLength,
					1.0f))
			{
				CameraBoom->TargetArmLength = DesiredArmLength;
				bIsZoomInterpolating = false;
			}
		}
	UpdateLockOnIndicator();
}


void ABSPlayerCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (MoveInput)
	{
		EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Triggered, this, &ABSPlayerCharacter::MoveAction);
		EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Completed, this, &ABSPlayerCharacter::StopMoveAction);
		EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Canceled, this, &ABSPlayerCharacter::StopMoveAction);
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
	if (EvadeInput)
	{
		EnhancedInputComponent->BindAction(EvadeInput, ETriggerEvent::Triggered,this, &ABSPlayerCharacter::EvadeAction);
	}
	if (DashInput)
	{
		EnhancedInputComponent->BindAction(DashInput, ETriggerEvent::Triggered,this, &ABSPlayerCharacter::DashPressedAction);
		EnhancedInputComponent->BindAction(DashInput, ETriggerEvent::Completed,this, &ABSPlayerCharacter::DashReleasedAction);
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

void ABSPlayerCharacter::MoveAction(const FInputActionValue &Value)
{
	if (UBSAbilitySystemComponent* BSASC = GetBSAbilitySystemComponent())
	{
		if (BSASC->HasMatchingGameplayTag(BSGameplayTags::State_Block_Move))
		{
			CurrentMoveInput = FVector2D::ZeroVector;
			return;
		}
	}
	
	const FVector2D MovementVector = Value.Get<FVector2D>();
	CurrentMoveInput = MovementVector;
	if (MovementVector.IsNearlyZero())
	{
		return;
	}

	StopEvadeMontageTail();
	
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

void ABSPlayerCharacter::StopMoveAction(const FInputActionValue &Value)
{
	CurrentMoveInput = FVector2D::ZeroVector;
}

void ABSPlayerCharacter::LookAction(const FInputActionValue &Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
	
	if (bLockOnActive)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			float MouseX = 0.0f;
			float MouseY = 0.0f;
			PlayerController->GetInputMouseDelta(MouseX, MouseY);
			HandleLockOnSwitchInput(MouseX);
		}
	}
}

void ABSPlayerCharacter::ZoomAction(const FInputActionValue &Value)
{
	if (!Controller || !CameraBoom)
		return;

	const float ZoomAxis = Value.Get<float>();

	DesiredArmLength = FMath::Clamp(DesiredArmLength - ZoomAxis * ZoomStep, MinArmLength, MaxArmLength);
	bIsZoomInterpolating = true;
}

void ABSPlayerCharacter::JumpAction()
{
	if (UBSAbilitySystemComponent* BSASC = GetBSAbilitySystemComponent())
	{
		if (BSASC->HasMatchingGameplayTag(BSGameplayTags::State_Block_Move))
		{
			return;
		}
	}

	StopEvadeMontageTail();
	Jump();
}

void ABSPlayerCharacter::EvadeAction()
{
	UBSAbilitySystemComponent* BSASC = GetBSAbilitySystemComponent();
	if (!BSASC)
	{
		return;
	}
	BSASC->AbilityInputTagPressed(BSGameplayTags::Input_Evade);
}

void ABSPlayerCharacter::DashPressedAction()
{
	UBSAbilitySystemComponent* BSASC = GetBSAbilitySystemComponent();
	if (!BSASC)
	{
		return;
	}
	BSASC->AbilityInputTagPressed(BSGameplayTags::Input_Dash);
}

void ABSPlayerCharacter::DashReleasedAction()
{
	UBSAbilitySystemComponent* BSASC = GetBSAbilitySystemComponent();
	if (!BSASC)
	{
		return;
	}
	BSASC->AbilityInputTagReleased(BSGameplayTags::Input_Dash);
}

void ABSPlayerCharacter::StopJumpingAction()
{
	StopJumping();
}

void ABSPlayerCharacter::BasicSkillAction()
{
	UBSAbilitySystemComponent *BSASC = GetBSAbilitySystemComponent();
	if (!BSASC)
	{
		return;
	}

	BSASC->AbilityInputTagPressed(BSGameplayTags::Input_BasicAttack);
}

void ABSPlayerCharacter::LockOnAction()
{
	if (bLockOnActive)
	{
		ClearCombatTarget();
		return;
	}
	SetCombatTarget(FindBestLockOnTarget());
}

void ABSPlayerCharacter::UpdateLockOnTarget()
{
	if (!bLockOnActive)
	{
		return;
	}

	if (IsDead() || !IsLockOnTargetable(LockOnTarget))
	{
		ClearCombatTarget();
		return;
	}

	const float DistanceSq = FVector::DistSquared(GetActorLocation(), LockOnTarget->GetActorLocation());
	if (DistanceSq > FMath::Square(LockOnSearchRadius))
	{
		ClearCombatTarget();
		return;
	}

	if (!HasLineOfSightToTarget(LockOnTarget))
	{
		ClearCombatTarget();
	}
}

ABSBaseCharacter *ABSPlayerCharacter::FindBestLockOnTarget() const
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	struct FLockOnCandidate
	{
		TWeakObjectPtr<ABSBaseCharacter> Actor;
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
		QueryParams);

	if (bDrawLockOnDebug)
	{
		DrawDebugSphere(World, SearchLocation, LockOnSearchRadius, 32, FColor::Cyan, false, 1.0f);
	}

	if (!bHasOverlap)
	{
		return nullptr;
	}

	const FVector ViewLocation = FollowCamera 
		? FollowCamera->GetComponentLocation() : GetActorLocation();
	const FVector ViewForward = FollowCamera 
		? FollowCamera->GetForwardVector() : GetActorForwardVector();
	const float MinDot = FMath::Cos(FMath::DegreesToRadians(LockOnMaxAngle));

	TArray<FLockOnCandidate> Candidates;

	for (const FOverlapResult &Overlap : Overlaps)
	{
		ABSBaseCharacter *Candidate = Cast<ABSBaseCharacter>(Overlap.GetActor());
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
		Candidates.Add({Candidate, Score});
	}

	Candidates.Sort([](const FLockOnCandidate &A, const FLockOnCandidate &B)
					{ return A.Score < B.Score; });

	for (const FLockOnCandidate &Candidate : Candidates)
	{
		ABSBaseCharacter *CandidateActor = Candidate.Actor.Get();

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

bool ABSPlayerCharacter::IsLockOnTargetable(ABSBaseCharacter *Candidate) const
{
	if (!IsValid(Candidate) || Candidate == this || Candidate->IsDead())
	{
		return false;
	}

	if (!Cast<ABSEnemyCharacter>(Candidate))
	{
		return false;
	}
	
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate))
	{
		return false;
	}

	return true;
}

bool ABSPlayerCharacter::HasLineOfSightToTarget(ABSBaseCharacter *Candidate) const
{
	if (!Candidate)
	{
		return false;
	}

	UWorld *World = GetWorld();
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
		QueryParams);

	const bool bHasSight = !bHit || HitResult.GetActor() == Candidate;

	if (bDrawLockOnDebug)
	{
		DrawDebugLine(World, Start, End, bHasSight ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.5f);
	}

	return bHasSight;
}

void ABSPlayerCharacter::StartLockOnUpdateTimer()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		LockOnUpdateTimerHandle,
		this,
		&ABSPlayerCharacter::UpdateLockOnTarget,
		LockOnUpdateInterval,
		true);
}

void ABSPlayerCharacter::StopLockOnUpdateTimer()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(LockOnUpdateTimerHandle);
}

void ABSPlayerCharacter::StopEvadeMontageTail()
{
	const FAbilitySkillData SkillData = GetSkillDataForAbility(BSGameplayTags::Ability_Player_Evade);
	UAnimMontage* CurrentEvadeMontage = SkillData.Montage;
	
	if (!CurrentEvadeMontage)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance && AnimInstance -> Montage_IsPlaying(CurrentEvadeMontage))
	{
		AnimInstance->Montage_Stop(EvadeMontageBlendOutTime, CurrentEvadeMontage);
	}
}

void ABSPlayerCharacter::UpdateLockOnIndicator()
{
	if (!LockOnIndicatorComponent)
	{
		return;
	}
	// 페이드아웃이 끝날 때까지 기존 표시를 유지.
	if (!bLockOnActive && bPlayingUnlockAnimation)
	{
		return;
	}
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	const bool bShow = PlayerController && bLockOnActive && IsValid(LockOnTarget) && !LockOnTarget->IsDead() && !IsDead();
	LockOnIndicatorComponent->SetVisibility(bShow);
	if (!bShow)
	{
		return;
	}
	LockOnIndicatorComponent->SetOwnerPlayer(PlayerController->GetLocalPlayer());
	LockOnIndicatorComponent->SetWorldLocation(LockOnTarget->GetActorLocation() + LockOnIndicatorOffset);
}

void ABSPlayerCharacter::HandleLockOnSwitchInput(float MouseDeltaX)
{
	if (!bLockOnActive || !IsValid(LockOnTarget))
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!PlayerController || PlayerController->bShowMouseCursor || PlayerController->IsLookInputIgnored())
	{
		return;
	}

	// 매우 작은 흔들림은 무시.
	if (FMath::Abs(MouseDeltaX) <= 0.01f)
	{
		return;
	}

	const double Now = FPlatformTime::Seconds();

	// 초기화 직후나, 일정 시간 움직이지 않았을 경우.
	const bool bNewGesture = LastLockOnSwitchInputTime < 0.0 
		|| Now - LastLockOnSwitchInputTime >= LockOnSwitchNeutralTime;

	if (bNewGesture)
	{
		bLockOnSwitchConsumed = false;
		LockOnSwitchAccumulated = 0.0f;
		LockOnGestureStartTime = Now;
	}

	LastLockOnSwitchInputTime = Now;

	// 한 번의 연속 움직임에서는 한 번만 전환.
	if (bLockOnSwitchConsumed || Now < NextLockOnSwitchTime)
	{
		return;
	}

	const bool bDirectionChanged = !FMath::IsNearlyZero(LockOnSwitchAccumulated) 
		&& FMath::Sign(LockOnSwitchAccumulated) != FMath::Sign(MouseDeltaX);

	const bool bWindowExpired = Now - LockOnGestureStartTime > LockOnSwitchInputWindow;

	if (bDirectionChanged || bWindowExpired)
	{
		LockOnSwitchAccumulated = 0.0f;
		LockOnGestureStartTime = Now;
	}

	LockOnSwitchAccumulated += MouseDeltaX;

	if (FMath::Abs(LockOnSwitchAccumulated) < LockOnSwitchThreshold)
	{
		return;
	}

	const int32 Direction = LockOnSwitchAccumulated > 0.0f ? 1 : -1;

	// 후보가 없어도 이번 움직임의 전환 시도는 종료.
	bLockOnSwitchConsumed = true;
	LockOnSwitchAccumulated = 0.0f;

	NextLockOnSwitchTime = Now + LockOnSwitchCooldown;

	if (ABSBaseCharacter* NextTarget = FindLockOnTargetInDirection(Direction))
	{
		SetCombatTarget(NextTarget);
	}
}

void ABSPlayerCharacter::ResetLockOnSwitchInput()
{
	LockOnSwitchAccumulated = 0.0f;
	LockOnGestureStartTime = 0.0;
	LastLockOnSwitchInputTime = -1.0;
	NextLockOnSwitchTime = 0.0;
	bLockOnSwitchConsumed = false;
}

ABSBaseCharacter* ABSPlayerCharacter::FindLockOnTargetInDirection(int32 Direction) const
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    UWorld* World = GetWorld();

    if (!PlayerController || !World || !IsValid(LockOnTarget) || Direction == 0)
    {
        return nullptr;
    }

    Direction = Direction > 0 ? 1 : -1;

    int32 Width = 0;
    int32 Height = 0;

    PlayerController->GetViewportSize(Width, Height);

    if (Width <= 0 || Height <= 0)
    {
        return nullptr;
    }

    FVector2D CurrentScreen;
    if (!PlayerController->ProjectWorldLocationToScreen(LockOnTarget->GetActorLocation(),CurrentScreen))
    {
        return nullptr;
    }

    TArray<FOverlapResult> Overlaps;

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(LockOnObjectChannel);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    World->OverlapMultiByObjectType(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ObjectParams,
        FCollisionShape::MakeSphere(LockOnSearchRadius),
        QueryParams);

    ABSBaseCharacter* BestTarget = nullptr;
    float BestScore = TNumericLimits<float>::Max();

    TSet<ABSBaseCharacter*> CheckedActors;

    const FVector ViewLocation = FollowCamera
        ? FollowCamera->GetComponentLocation() : GetActorLocation();

    const FVector ViewForward = FollowCamera
        ? FollowCamera->GetForwardVector() : GetActorForwardVector();

    const float MinDot = FMath::Cos(FMath::DegreesToRadians(LockOnMaxAngle));

    for (const FOverlapResult& Overlap : Overlaps)
    {
        ABSBaseCharacter* Candidate = Cast<ABSBaseCharacter>(Overlap.GetActor());

        if (!IsValid(Candidate) || Candidate == LockOnTarget || CheckedActors.Contains(Candidate))
        {
            continue;
        }

        // 같은 Actor의 여러 충돌 컴포넌트를 중복 검사하지 않음.
        CheckedActors.Add(Candidate);

        if (!IsLockOnTargetable(Candidate))
        {
            continue;
        }

        const FVector TargetLocation = Candidate->GetActorLocation();

        if (FVector::DistSquared(GetActorLocation(), TargetLocation) > FMath::Square(LockOnSearchRadius))
        {
            continue;
        }

        const FVector ViewDirection = (TargetLocation - ViewLocation).GetSafeNormal();

        if (FVector::DotProduct(ViewForward, ViewDirection) < MinDot)
        {
            continue;
        }

        FVector2D CandidateScreen;

        if (!PlayerController->ProjectWorldLocationToScreen(TargetLocation, CandidateScreen))
        {
            continue;
        }

        // 화면 안에 있는 후보만 선택.
        if (CandidateScreen.X < 0.0f || CandidateScreen.X > Width 
        	|| CandidateScreen.Y < 0.0f || CandidateScreen.Y > Height)
        {
            continue;
        }

        const FVector2D ScreenDelta = CandidateScreen - CurrentScreen;

        // 화면 가로 위치가 거의 겹치는 대상은 제외.
        if (ScreenDelta.X * Direction <= Width * 0.01f)
        {
            continue;
        }

        if (!HasLineOfSightToTarget(Candidate))
        {
            continue;
        }

        // 가로로 가까운 대상을 우선하되 높이 차이도 고려.
        const float Score = FMath::Abs(ScreenDelta.X) / Width + FMath::Abs(ScreenDelta.Y) / Height * 0.35f;

        if (Score < BestScore)
        {
            BestScore = Score;
            BestTarget = Candidate;
        }
    }

    return BestTarget;
}


void ABSPlayerCharacter::OnDeathStarted(AActor *Killer)
{
	ClearCombatTarget();
	Super::OnDeathStarted(Killer);
}

void ABSPlayerCharacter::OnDeathFinished(AActor *Killer)
{
	Super::OnDeathFinished(Killer);
	// 게임 오버 UI, 리스폰 타이머 등
}

void ABSPlayerCharacter::SetCombatTarget(AActor *NewTarget)
{
	ABSBaseCharacter* Candidate =Cast<ABSBaseCharacter>(NewTarget);
	if (!IsLockOnTargetable(Candidate))
	{
		ClearCombatTarget();
		return;
	}
	if (bLockOnActive && LockOnTarget == Candidate)
	{
		return;
	}
	LockOnTarget = Candidate;
	if (!bLockOnActive)
	{
		bLockOnActive = true;

		ResetLockOnSwitchInput();
		StartLockOnUpdateTimer();
		if (UBSAbilitySystemComponent* BSASC = GetBSAbilitySystemComponent())
		{
			BSASC->AddLooseGameplayTag(BSGameplayTags::State_LockOn);
		}
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->bOrientRotationToMovement = false;
			Movement->bUseControllerDesiredRotation = true;
		}
	}
	bPlayingUnlockAnimation = false;
	UpdateLockOnIndicator();
	OnLockOnIndicatorStateChanged(true);
}

void ABSPlayerCharacter::ClearCombatTarget()
{
	const bool bWasLockedOn = bLockOnActive;
	LockOnTarget = nullptr;
	StopLockOnUpdateTimer();
	if (bLockOnActive)
	{
		if (UBSAbilitySystemComponent* BSASC = GetBSAbilitySystemComponent())
		{
			BSASC->RemoveLooseGameplayTag(BSGameplayTags::State_LockOn);
		}
	}
	bLockOnActive = false;
	
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bUseControllerDesiredRotation = false;
		Movement->bOrientRotationToMovement = true;
	}
	
	ResetLockOnSwitchInput();
	if (bWasLockedOn)
	{
		bPlayingUnlockAnimation = true;
		OnLockOnIndicatorStateChanged(false);
	}
	UpdateLockOnIndicator();
}

void ABSPlayerCharacter::ApplyStaminaRegenDelay()
{
	if (!AbilitySystemComponent || !StaminaRegenDelayEffect)
	{
		return;
	}

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StaminaRegenDelayEffect,1.0f, ContextHandle);

	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void ABSPlayerCharacter::FinishLockOnIndicatorFadeOut()
{
	// 도중에 다시 락온했거나 이미 종료 처리했다면 무시.
	if (bLockOnActive || !bPlayingUnlockAnimation)
	{
		return;
	}
	bPlayingUnlockAnimation = false;
	UpdateLockOnIndicator();
}

AActor *ABSPlayerCharacter::GetCombatTarget() const
{
	return LockOnTarget;
}

void ABSPlayerCharacter::SetNextComboMontage(UAnimMontage *Montage)
{
	NextComboMontage = Montage;
}

UAnimMontage *ABSPlayerCharacter::GetNextComboMontage() const
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
