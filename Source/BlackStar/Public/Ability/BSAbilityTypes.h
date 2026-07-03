#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BSAbilityTypes.generated.h"

class ABSProjectile;

UENUM(BlueprintType)
enum class EBSHitReactionType : uint8
{
	None,
	Flinch,
	Knockback,
	Launch
};

USTRUCT(BlueprintType)
struct FBSHitReactionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	EBSHitReactionType ReactionType = EBSHitReactionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	float KnockbackStrength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	FGameplayTagContainer IgnoreIfTargetHasTags;
};

USTRUCT(BlueprintType)
struct FAbilitySkillData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FName StartSection = NAME_None;
};

USTRUCT(BlueprintType)
struct FMeleeTraceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName StartSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName EndSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceRadius = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float ExtraLength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FGameplayTag HitCueTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reaction")
	FBSHitReactionData HitReactionData;
};

USTRUCT(BlueprintType)
struct FProjectileData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FName SpawnSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<ABSProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float DamageMultiplier = 1.0f;
};

