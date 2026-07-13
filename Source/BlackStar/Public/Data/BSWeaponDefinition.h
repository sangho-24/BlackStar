// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BSWeaponDefinition.generated.h"

class UStaticMesh;
class USkeletalMesh;
class UBSWeaponMotionSet;

UENUM(BlueprintType)
enum class EBSWeaponMeshType : uint8
{
	StaticMesh,
	SkeletalMesh
};

UENUM(BlueprintType)
enum class EBSWeaponType : uint8
{
	None,
	Sword,
	GreatSword,
	Staff
};

UCLASS()
class BLACKSTAR_API UBSWeaponDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EBSWeaponType WeaponType = EBSWeaponType::None;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Visual")
	EBSWeaponMeshType MeshType = EBSWeaponMeshType::SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Visual",
		meta = (EditCondition = "MeshType == EBSWeaponMeshType::StaticMesh"))
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Visual",
		meta = (EditCondition = "MeshType == EBSWeaponMeshType::SkeletalMesh"))
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual")
	FName AttachSocketName = TEXT("weapon_r_socket");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual")
	FTransform GripOffset = FTransform::Identity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
	float BaseDamage = 10.0f;

	// 같은 무기군끼리 공유 가능
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UBSWeaponMotionSet> MotionSet;
};
