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
	virtual void Death(AActor* Killer) {}
};
