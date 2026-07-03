#include "Controller/BSPlayerController.h"
#include "UI/HUDWidget.h"
#include "Character/BSPlayerCharacter.h"
#include "GAS/BSBaseAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"



void ABSPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ABSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
	{
		if (CurrentContext)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}

}

void ABSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UnbindHUDDelegates();
	
	CachedPlayer = Cast<ABSPlayerCharacter>(InPawn);
	if (!CachedPlayer)
	{
		CachedASC = nullptr;
		CachedAS = nullptr;
		return;
	}

	CachedAS = CachedPlayer->GetAttributeSet();
	CachedASC = CachedPlayer->GetAbilitySystemComponent();
	if (!CachedASC || !CachedAS)
		return;
	
	// HUD 생성, 중복 생성 방지
	if (!HUDWidget && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	if (!HUDWidget) // 생성 실패(nullptr)시 탈출
		return;
	
	BindHUDDelegates();

	// 초기값 업데이트
	HUDWidget->UpdateHP(CachedAS->GetCurrentHP(), CachedAS->GetMaxHP());
	HUDWidget->UpdateMP(CachedAS->GetCurrentMP(), CachedAS->GetMaxMP());
	HUDWidget->UpdateStats(CachedAS);
}

void ABSPlayerController::OnUnPossess()
{
	UnbindHUDDelegates();

	CachedPlayer = nullptr;
	CachedASC = nullptr;
	CachedAS = nullptr;

	Super::OnUnPossess();
}

void ABSPlayerController::BindHUDDelegates()
{
	if (!CachedASC || !CachedAS)
		return;
	
	CachedASC->GetGameplayAttributeValueChangeDelegate(
		CachedAS->GetCurrentHPAttribute()).AddUObject(this, &ABSPlayerController::OnHPChanged);
	CachedASC->GetGameplayAttributeValueChangeDelegate(
		CachedAS->GetMaxHPAttribute()).AddUObject(this, &ABSPlayerController::OnHPChanged);

	CachedASC->GetGameplayAttributeValueChangeDelegate(
		CachedAS->GetCurrentMPAttribute()).AddUObject(this, &ABSPlayerController::OnMPChanged);
	CachedASC->GetGameplayAttributeValueChangeDelegate(
		CachedAS->GetMaxMPAttribute()).AddUObject(this, &ABSPlayerController::OnMPChanged);

	CachedASC->GetGameplayAttributeValueChangeDelegate(
		CachedAS->GetMagicPowerAttribute()).AddUObject(this, &ABSPlayerController::OnStatsChanged);
	CachedASC->GetGameplayAttributeValueChangeDelegate(
		CachedAS->GetDefenseAttribute()).AddUObject(this, &ABSPlayerController::OnStatsChanged);
}

void ABSPlayerController::UnbindHUDDelegates()
{
	if (!CachedASC)
		return;

	CachedASC->GetGameplayAttributeValueChangeDelegate(
		UBSBaseAttributeSet::GetCurrentHPAttribute()).RemoveAll(this);
	CachedASC->GetGameplayAttributeValueChangeDelegate(
		UBSBaseAttributeSet::GetMaxHPAttribute()).RemoveAll(this);

	CachedASC->GetGameplayAttributeValueChangeDelegate(
		UBSBaseAttributeSet::GetCurrentMPAttribute()).RemoveAll(this);
	CachedASC->GetGameplayAttributeValueChangeDelegate(
		UBSBaseAttributeSet::GetMaxMPAttribute()).RemoveAll(this);

	CachedASC->GetGameplayAttributeValueChangeDelegate(
		UBSBaseAttributeSet::GetMagicPowerAttribute()).RemoveAll(this);
	CachedASC->GetGameplayAttributeValueChangeDelegate(
		UBSBaseAttributeSet::GetDefenseAttribute()).RemoveAll(this);
}

void ABSPlayerController::OnHPChanged(const FOnAttributeChangeData& Data)
{
	if (!HUDWidget || !CachedAS)
		return;
	HUDWidget->UpdateHP(CachedAS->GetCurrentHP(), CachedAS->GetMaxHP());
}

void ABSPlayerController::OnMPChanged(const FOnAttributeChangeData& Data)
{
	if (!HUDWidget || !CachedAS)
		return;
	HUDWidget->UpdateMP(CachedAS->GetCurrentMP(), CachedAS->GetMaxMP());
}

void ABSPlayerController::OnStatsChanged(const FOnAttributeChangeData& Data)
{
	if (!HUDWidget || !CachedAS)
		return;
	HUDWidget->UpdateStats(CachedAS);
}
