// 무기 전투모션셋

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ability/BSAbilityTypes.h"
#include "BSWeaponMotionSet.generated.h"

USTRUCT(BlueprintType)
struct FBSWeaponMovementSet
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UBlendSpace> LocomotionBlendSpace;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Jump")
	TObjectPtr<UAnimSequence> JumpStartAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Jump")
	TObjectPtr<UAnimSequence> JumpLoopAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Jump")
	TObjectPtr<UAnimSequence> JumpLandAnimation;
};

UCLASS()
class BLACKSTAR_API UBSWeaponMotionSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Motion")
	TMap<FGameplayTag, FAbilitySkillData> SkillDataMap;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Motion")
	FBSWeaponMovementSet MovementSet;
};
