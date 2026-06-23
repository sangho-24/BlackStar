#include "Character/BSBaseCharacter.h"
#include "GAS/BSAbilitySystemComponent.h"
#include "GAS/BSAbilitySet.h"
#include "GAS/BSBaseAttributeSet.h"
#include "GameplayAbilitySpec.h"

ABSBaseCharacter::ABSBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UBSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UBSBaseAttributeSet>(TEXT("AttributeSet"));
}

void ABSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		if (HasAuthority())
		{
			for (const UBSAbilitySet* AbilitySet : DefaultAbilitySets)
				if (AbilitySet)
					AbilitySet->GiveToAbilitySystem(AbilitySystemComponent);

			for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
				if (AbilityClass)
					AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		}
	}
}

void ABSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAbilitySystemComponent* ABSBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

FAbilitySkillData ABSBaseCharacter::GetSkillDataForAbility(FGameplayTag AbilityTag)
{
	if (const FAbilitySkillData* FoundData = AbilitySkillDataMap.Find(AbilityTag))
	{
		return *FoundData;
	}

	return FAbilitySkillData();
}

void ABSBaseCharacter::SetProjectileData(const FProjectileData& Data)
{
	CachedProjectileData = Data;
}

FProjectileData ABSBaseCharacter::GetProjectileData() const
{
	return CachedProjectileData;
}

void ABSBaseCharacter::SetMeleeTraceData(const FMeleeTraceData& Data)
{
	CachedMeleeTraceData = Data;
}

FMeleeTraceData ABSBaseCharacter::GetMeleeTraceData() const
{
	return CachedMeleeTraceData;
}

void ABSBaseCharacter::SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical)
{
}

void ABSBaseCharacter::Death(AActor* Killer)
{
}
