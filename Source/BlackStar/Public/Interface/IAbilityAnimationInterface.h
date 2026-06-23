#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Ability/BSAbilityTypes.h"
#include "IAbilityAnimationInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UAbilityAnimationInterface : public UInterface
{
	GENERATED_BODY()
};

class BLACKSTAR_API IAbilityAnimationInterface
{
	GENERATED_BODY()

public:
	virtual FAbilitySkillData GetSkillDataForAbility(FGameplayTag AbilityTag) { return FAbilitySkillData(); }

	virtual void SetNextComboMontage(UAnimMontage* Montage) {}
	virtual UAnimMontage* GetNextComboMontage() const { return nullptr; }
	virtual void SetNextComboSection(FName SectionName) {}
	virtual FName GetNextComboSection() const { return NAME_None; }

	virtual AActor* GetCombatTarget() const { return nullptr; }

	virtual void SetProjectileData(const FProjectileData& Data) {}
	virtual FProjectileData GetProjectileData() const { return FProjectileData(); }

	virtual void SetMeleeTraceData(const FMeleeTraceData& Data) {}
	virtual FMeleeTraceData GetMeleeTraceData() const { return FMeleeTraceData(); }
};
