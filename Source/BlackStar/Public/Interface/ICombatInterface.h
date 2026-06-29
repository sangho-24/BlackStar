#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ICombatInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

class BLACKSTAR_API ICombatInterface
{
	GENERATED_BODY()

public:
	virtual void SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical) {}
	virtual void Death(AActor *Killer) {}
	virtual bool IsDead() const { return false; }

	virtual AActor *GetCombatTarget() const { return nullptr; }
	virtual void SetCombatTarget(AActor *NewTarget) {}
	virtual void ClearCombatTarget() {}
};
