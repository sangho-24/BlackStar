#include "Character/BSBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "GAS/BSBaseAttributeSet.h"

ABSBaseCharacter::ABSBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UBSBaseAttributeSet>(TEXT("AttributeSet"));
}

void ABSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

