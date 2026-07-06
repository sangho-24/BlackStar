#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Ability/BSAbilityTypes.h"
#include "GA_AttackBase.generated.h"

class ABSProjectile;
class UGameplayEffect;

UCLASS()
class BLACKSTAR_API UGA_AttackBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_AttackBase();
	// virtual void PostInitProperties() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Setup")
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Setup")
	TSubclassOf<UGameplayEffect> MeleeDamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float MeleeBaseDamage = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.016"))
	float TraceTickRate = 0.033f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TEnumAsByte<ECollisionChannel> DamageTraceChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Debug")
	bool bDrawDebugTrace = false;

	UPROPERTY()
	TObjectPtr<AActor> AttackTarget;

	FMeleeTraceData ActiveTraceData;
	FProjectileData ActiveProjectileData;
	TArray<TWeakObjectPtr<AActor>> HitActors;
	FTimerHandle TraceTimerHandle;

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual bool StartAttack(const FGameplayEventData* TriggerEventData);
	virtual void RegisterCommonEventTasks();
	virtual void PlayMontage(UAnimMontage* Montage, FName StartSection = NAME_None);

	void StartMeleeTrace();
	void StopMeleeTrace();
	UFUNCTION() void DoMeleeTrace();
	void ApplyMeleeDamage(AActor* TargetActor, const FHitResult& HitResult);

	UFUNCTION() virtual void OnMontageCompleted();
	UFUNCTION() virtual void OnMontageCancelled();

	UFUNCTION() virtual void OnMeleeTraceStart(FGameplayEventData Payload);
	UFUNCTION() virtual void OnMeleeTraceEnd(FGameplayEventData Payload);
	UFUNCTION() virtual void OnSpawnProjectile(FGameplayEventData Payload);

	virtual AActor* ResolveAttackTarget() const;
	virtual void FaceTarget(AActor* TargetActor);
	virtual void FaceAttackDirection();
};
