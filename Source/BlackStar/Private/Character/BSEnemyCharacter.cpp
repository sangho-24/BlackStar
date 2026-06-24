#include "Character/BSEnemyCharacter.h"

#include "GAS/BSAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/BSBaseAttributeSet.h"
#include "UI/NameplateWidget.h"

ABSEnemyCharacter::ABSEnemyCharacter()
{
	NameplateWidgetComponent =
		CreateDefaultSubobject<UWidgetComponent>(
			TEXT("NameplateWidgetComponent"));

	// Mesh보다 Capsule에 붙이는 쪽이 메시 회전/오프셋 영향을 덜 받음
	NameplateWidgetComponent->SetupAttachment(GetCapsuleComponent());
	NameplateWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	NameplateWidgetComponent->SetDrawSize(NameplateSize);
	NameplateWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABSEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeNameplate();
}

void ABSEnemyCharacter::InitializeNameplate()
{
	if (NameplateWidgetComponent)
	{
		const float CapsuleHalfHeight =
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		NameplateWidgetComponent->SetRelativeLocation(
			FVector(0.0f, 0.0f, CapsuleHalfHeight * NameplateHeight));

		if (NameplateWidgetClass)
		{
			NameplateWidgetComponent->SetWidgetClass(NameplateWidgetClass);
			NameplateWidgetComponent->InitWidget();
		}

		RefreshNameplate();
		
		if (AbilitySystemComponent)
		{
			CurrentHPDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					UBSBaseAttributeSet::GetCurrentHPAttribute()) .AddUObject(this, &ABSEnemyCharacter::OnHPChanged);

			MaxHPDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					UBSBaseAttributeSet::GetMaxHPAttribute()) .AddUObject(this, &ABSEnemyCharacter::OnHPChanged);
		}
	}
}

void ABSEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UBSBaseAttributeSet::GetCurrentHPAttribute()).Remove(CurrentHPDelegateHandle);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UBSBaseAttributeSet::GetMaxHPAttribute()).Remove(MaxHPDelegateHandle);
	}
	Super::EndPlay(EndPlayReason);
}


void ABSEnemyCharacter::RefreshNameplate()
{
	if (!NameplateWidgetComponent)
	{
		return;
	}

	UNameplateWidget* Nameplate = Cast<UNameplateWidget>(NameplateWidgetComponent->GetUserWidgetObject());

	if (!Nameplate)
	{
		return;
	}

	Nameplate->UpdateName(EnemyName);

	if (AttributeSet)
	{
		Nameplate->UpdateHP(
			AttributeSet->GetCurrentHP(),
			AttributeSet->GetMaxHP());
	}
}

void ABSEnemyCharacter::OnHPChanged(const FOnAttributeChangeData& Data)
{
	RefreshNameplate();
}
