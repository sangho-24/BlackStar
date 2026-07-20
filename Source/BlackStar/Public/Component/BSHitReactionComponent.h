
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Ability/BSAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "BSHitReactionComponent.generated.h"

class ABSBaseCharacter;
class UAbilitySystemComponent;

UENUM()
enum class EHitReactionDirection : uint8
{
	Front,
	Back,
	Left,
	Right
};

// UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
UCLASS()
class BLACKSTAR_API UBSHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TObjectPtr<ABSBaseCharacter> OwnerCharacter = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> OwnerASC = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Hit Reaction|Cue")
	FGameplayTag DefaultReactionCueTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "Hit Reaction|Cue")
	TMap<FGameplayTag, FGameplayTag> ReactionCueOverridesByDamageType;
	
	UPROPERTY(EditDefaultsOnly, Category = "Hit Reaction|Rules")
	TMap<FGameplayTag, FGameplayTag> ImmunityStateTagsByDamageType;

public:	
	UBSHitReactionComponent();

protected:
	virtual void BeginPlay() override;

public:	
	void ApplyHitReaction(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData);
	// Called every frame
	// virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool CanPlayTargetHitCue(const FBSHitReactionData& ReactionData) const;
	bool CanApplyGameplayReaction(const FBSHitReactionData& ReactionData) const;
	bool HasDamageTypeImmunity(const FGameplayTag& DamageTypeTag) const;
	
	void ApplyFlinch(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData);
	void ApplyKnockback(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData);
	void ApplyLaunch(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData);
	
	FVector GetReactionDirection(AActor* Attacker, const FHitResult& HitResult) const;
	void CancelAbilitiesForReaction(const FBSHitReactionData& ReactionData);
	
	EHitReactionDirection GetReactionMontageDirection(AActor* Attacker, const FHitResult& HitResult) const;
	FName GetReactionMontageSectionName(EHitReactionDirection Direction) const;
	FGameplayTag GetReactionMontageTag(EBSHitReactionType ReactionType) const;
	bool PlayReactionMontage(EBSHitReactionType ReactionType, EHitReactionDirection Direction);
	void BeginReactionMontage();
	void EndReactionMontage(UAnimMontage* Montage, bool bInterrupted);
	
	FGameplayTag GetReactionCueTag(const FGameplayTag DamageTypeTag) const;
	void PlayReactionCue(FGameplayTag CueTag, AActor* Attacker, const FHitResult& HitResult) const;
};

