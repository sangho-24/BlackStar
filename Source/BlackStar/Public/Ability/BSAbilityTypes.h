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

	// 경직 종류 (모션)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	EBSHitReactionType ReactionType = EBSHitReactionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	float KnockbackStrength = 0.0f;

	// 특정 공격에만 예외가 필요할 경우에 사용하려고 만들었는데
	// 쓸 일 없을 것 같음, 필요할 때 추가하자.
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	// FGameplayTagContainer IgnoreIfTargetHasTags;
	
	// 어떤 공격을 맞았나? (검, 화살, 마법 등 이펙트)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
    FGameplayTag DamageTypeTag;
};

USTRUCT(BlueprintType)
struct FAbilitySkillData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FName StartSection = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TArray<TObjectPtr<UAnimMontage>> MontageVariants;
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

