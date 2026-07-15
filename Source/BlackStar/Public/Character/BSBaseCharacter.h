#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Ability/BSAbilityTypes.h"
#include "Data/BSWeaponMotionSet.h"
#include "Interface/IAbilityAnimationInterface.h"
#include "Interface/ICombatInterface.h"
#include "GenericTeamAgentInterface.h"
#include "BSBaseCharacter.generated.h"

class UBSAbilitySystemComponent;
class UAbilitySystemComponent;
class UBSBaseAttributeSet;
class UBSAbilitySet;
class UGameplayAbility;
class AFloatingDamageActor;
class UBSHitReactionComponent;
class UBSWeaponComponent;


UCLASS()
class BLACKSTAR_API ABSBaseCharacter : public ACharacter
	, public IAbilitySystemInterface
	, public IAbilityAnimationInterface
	, public ICombatInterface
	, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ABSBaseCharacter();
	

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UBSAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UBSHitReactionComponent> HitReactionComponent;

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
	
	bool bIsTurning = false;
	FRotator TurnTargetRotation = FRotator::ZeroRotator;
	float CurrentTurnSpeed = 720.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Setup")
	float TurnTolerance = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Setup")
	TSubclassOf<AFloatingDamageActor> FloatingDamageActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Setup")
	float FloatingDamageRandomRadius = 30.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Setup")
	TObjectPtr<UAnimMontage> DeathMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Movement|Setup")
	FBSWeaponMovementSet DefaultMovementSet;
	
	bool bIsDead = false;
	bool bDeathFinished = false;
	TWeakObjectPtr<AActor> DeathKiller;

protected:
	virtual void BeginPlay() override;
	
	void UpdateTurning(float DeltaTime);

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void StartTurning(const FRotator& TargetRotation, float TurnSpeed = 720.0f);
	void StopTurning();
	bool IsTurning() const { return bIsTurning; }
	void NotifyDeathAnimationFinished();
	
	// 인라인 함수
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE UBSAbilitySystemComponent* GetBSAbilitySystemComponent() const { return AbilitySystemComponent; }
	FORCEINLINE UBSBaseAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UBSWeaponComponent* GetWeaponComponent() const;
	
	UFUNCTION(BlueprintPure, Category = "Animation|Movement")
	FBSWeaponMovementSet GetCurrentMovementSet() const;
	
	// 인터페이스
	virtual FAbilitySkillData GetSkillDataForAbility(FGameplayTag AbilityTag) override;
	virtual void SetProjectileData(const FProjectileData& Data) override;
	virtual FProjectileData GetProjectileData() const override;
	virtual void SetMeleeTraceData(const FMeleeTraceData& Data) override;
	virtual FMeleeTraceData GetMeleeTraceData() const override;
	virtual void SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical) override;
	virtual void Death(AActor* Killer) override;
	virtual bool IsDead() const override { return bIsDead; }
	virtual void ApplyHitReaction(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData) override;
	
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
private:
	void DisableCharacterOnDeath();
	void PlayDeathMontage();
	void FinishDeath();
	
protected:
	virtual UAnimMontage* SelectDeathMontage(AActor* Killer) const;
	virtual void OnDeathStarted(AActor* Killer);
	virtual void OnDeathFinished(AActor* Killer);
};
