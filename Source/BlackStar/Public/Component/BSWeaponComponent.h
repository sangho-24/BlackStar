// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BSWeaponComponent.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class UBSWeaponDefinition;
class UBSWeaponMotionSet;
class UMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBSOnWeaponChanged);

UCLASS()
class BLACKSTAR_API UBSWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// BP에서 지정하면 BeginPlay 때 자동 장착
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UBSWeaponDefinition> DefaultWeapon;
	
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FBSOnWeaponChanged OnWeaponChanged;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> WeaponStaticMeshComponent;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> WeaponSkeletalMeshComponent;

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UBSWeaponDefinition> EquippedWeapon;
	
protected:
	virtual void BeginPlay() override;
	
private:
	UStaticMeshComponent* GetOrCreateStaticMeshComponent();
	USkeletalMeshComponent* GetOrCreateSkeletalMeshComponent();
	bool AttachWeaponMeshComponent(UMeshComponent* MeshComponent, const UBSWeaponDefinition* WeaponDefinition);
	void HideWeaponMeshComponents();
	
public:	
	// Sets default values for this component's properties
	UBSWeaponComponent();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(UBSWeaponDefinition* NewWeapon);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipWeapon();

	UBSWeaponDefinition* GetEquippedWeapon() const {return EquippedWeapon;}
	float GetBaseDamage() const;
	UBSWeaponMotionSet* GetMotionSet() const;

	// 근접 Trace / 투사체 소켓 위치를 얻을 때 사용.
	// Static / Skeletal 양쪽 모두 반환 가능하다.
	UMeshComponent* GetActiveWeaponMeshComponent() const;
	
};
