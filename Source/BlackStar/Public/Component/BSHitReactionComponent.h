
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Ability/BSAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "BSHitReactionComponent.generated.h"

class ABSBaseCharacter;
class UAbilitySystemComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLACKSTAR_API UBSHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TObjectPtr<ABSBaseCharacter> OwnerCharacter = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> OwnerASC = nullptr;
	
public:	
	UBSHitReactionComponent();

protected:
	virtual void BeginPlay() override;

public:	
	void ApplyHitReaction(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData);
	// Called every frame
	// virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool CanApplyHitReaction(const FBSHitReactionData& ReactionData) const;
	void ApplyFlinch(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData);
	void ApplyKnockback(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData);
	void ApplyLaunch(AActor* Attacker, const FHitResult& HitResult, const FBSHitReactionData& ReactionData);
	FVector GetReactionDirection(AActor* Attacker, const FHitResult& HitResult) const;
	void CancelAbilitiesForReaction(const FBSHitReactionData& ReactionData);
		
};
