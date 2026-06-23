#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Ability/BSAbilityTypes.h"
#include "Interface/IAbilityAnimationInterface.h"
#include "Interface/ICombatInterface.h"
#include "BSBaseCharacter.generated.h"

class UBSAbilitySystemComponent;
class UAbilitySystemComponent;
class UBSBaseAttributeSet;
class UBSAbilitySet;
class UGameplayAbility;

UCLASS()
class BLACKSTAR_API ABSBaseCharacter : public ACharacter, public IAbilitySystemInterface, public IAbilityAnimationInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	ABSBaseCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UBSAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UBSBaseAttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Abilities|Setup")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities|Setup")
	TArray<TObjectPtr<UBSAbilitySet>> DefaultAbilitySets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Abilities|Setup")
	TMap<FGameplayTag, FAbilitySkillData> AbilitySkillDataMap;

	FMeleeTraceData CachedMeleeTraceData;
	FProjectileData CachedProjectileData;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 인라인 함수
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE UBSAbilitySystemComponent* GetBSAbilitySystemComponent() const { return AbilitySystemComponent; }
	FORCEINLINE UBSBaseAttributeSet* GetAttributeSet() const { return AttributeSet; }

	virtual FAbilitySkillData GetSkillDataForAbility(FGameplayTag AbilityTag) override;
	virtual void SetProjectileData(const FProjectileData& Data) override;
	virtual FProjectileData GetProjectileData() const override;
	virtual void SetMeleeTraceData(const FMeleeTraceData& Data) override;
	virtual FMeleeTraceData GetMeleeTraceData() const override;
	virtual void SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical) override;
	virtual void Death(AActor* Killer) override;
};
