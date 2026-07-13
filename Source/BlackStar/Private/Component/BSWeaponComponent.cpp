// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/BSWeaponComponent.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/BSWeaponDefinition.h"
#include "Data/BSWeaponMotionSet.h"
#include "Character/BSBaseCharacter.h"


UBSWeaponComponent::UBSWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBSWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	if (DefaultWeapon)
	{
		EquipWeapon(DefaultWeapon);
	}
}

void UBSWeaponComponent::EquipWeapon(UBSWeaponDefinition* NewWeapon)
{
	if (!NewWeapon)
	{
		UnequipWeapon();
		return;
	}

	UMeshComponent* NewMeshComponent = nullptr;

	switch (NewWeapon->MeshType)
	{
	case EBSWeaponMeshType::StaticMesh:
		{
			if (!NewWeapon->StaticMesh)
			{
				return;
			}
			UStaticMeshComponent* StaticMeshComponent = GetOrCreateStaticMeshComponent();
			if (!StaticMeshComponent)
			{
				return;
			}
			StaticMeshComponent->SetStaticMesh(NewWeapon->StaticMesh);
			NewMeshComponent = StaticMeshComponent;
			break;
		}
	case EBSWeaponMeshType::SkeletalMesh:
		{
			if (!NewWeapon->SkeletalMesh)
			{
				return;
			}
			USkeletalMeshComponent* SkeletalMeshComponent = GetOrCreateSkeletalMeshComponent();
			if (!SkeletalMeshComponent)
			{
				return;
			}
			SkeletalMeshComponent->SetSkeletalMesh(NewWeapon->SkeletalMesh);
			NewMeshComponent = SkeletalMeshComponent;
			break;
		}
	}

	if (!AttachWeaponMeshComponent(NewMeshComponent, NewWeapon))
	{
		return;
	}
	HideWeaponMeshComponents();
	EquippedWeapon = NewWeapon;
	NewMeshComponent->SetHiddenInGame(false);
	NewMeshComponent->SetVisibility(true);
	
	OnWeaponChanged.Broadcast();
}

void UBSWeaponComponent::UnequipWeapon()
{
	EquippedWeapon = nullptr;
	HideWeaponMeshComponents();

	if (WeaponStaticMeshComponent)
	{
		WeaponStaticMeshComponent->SetStaticMesh(nullptr);
	}

	if (WeaponSkeletalMeshComponent)
	{
		WeaponSkeletalMeshComponent->SetSkeletalMesh(nullptr);
	}
	OnWeaponChanged.Broadcast();
}

float UBSWeaponComponent::GetBaseDamage() const
{
	return EquippedWeapon ? EquippedWeapon -> BaseDamage: 0.0f;
}

UBSWeaponMotionSet* UBSWeaponComponent::GetMotionSet() const
{
	return EquippedWeapon ? EquippedWeapon -> MotionSet : nullptr;
}

UMeshComponent* UBSWeaponComponent::GetActiveWeaponMeshComponent() const
{
	if (!EquippedWeapon)
	{
		return nullptr;
	}

	switch (EquippedWeapon->MeshType)
	{
	case EBSWeaponMeshType::StaticMesh:
		return WeaponStaticMeshComponent;
	case EBSWeaponMeshType::SkeletalMesh:
		return WeaponSkeletalMeshComponent;
	default:
		return nullptr;
	}
}


UStaticMeshComponent* UBSWeaponComponent::GetOrCreateStaticMeshComponent()
{
	if (WeaponStaticMeshComponent)
	{
		return WeaponStaticMeshComponent;
	}

	ABSBaseCharacter* OwnerCharacter = Cast<ABSBaseCharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return nullptr;
	}

	WeaponStaticMeshComponent =
		NewObject<UStaticMeshComponent>(OwnerCharacter,TEXT("WeaponStaticMeshComponent"));

	if (!WeaponStaticMeshComponent)
	{
		return nullptr;
	}

	OwnerCharacter->AddInstanceComponent(WeaponStaticMeshComponent);

	WeaponStaticMeshComponent->SetupAttachment(OwnerCharacter->GetMesh());

	WeaponStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponStaticMeshComponent->SetGenerateOverlapEvents(false);
	WeaponStaticMeshComponent->SetVisibility(false);
	WeaponStaticMeshComponent->SetHiddenInGame(true);
	WeaponStaticMeshComponent->RegisterComponent();

	return WeaponStaticMeshComponent;
}

USkeletalMeshComponent* UBSWeaponComponent::GetOrCreateSkeletalMeshComponent()
{
	if (WeaponSkeletalMeshComponent)
	{
		return WeaponSkeletalMeshComponent;
	}

	ABSBaseCharacter* OwnerCharacter = Cast<ABSBaseCharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return nullptr;
	}

	WeaponSkeletalMeshComponent =
		NewObject<USkeletalMeshComponent>(OwnerCharacter,TEXT("WeaponSkeletalMeshComponent"));

	if (!WeaponSkeletalMeshComponent)
	{
		return nullptr;
	}

	OwnerCharacter->AddInstanceComponent(WeaponSkeletalMeshComponent);

	WeaponSkeletalMeshComponent->SetupAttachment(OwnerCharacter->GetMesh());

	WeaponSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponSkeletalMeshComponent->SetGenerateOverlapEvents(false);
	WeaponSkeletalMeshComponent->SetVisibility(false);
	WeaponSkeletalMeshComponent->SetHiddenInGame(true);

	// 무기 자체 애니메이션이 생기기 전까지 일단 비활성화
	WeaponSkeletalMeshComponent->SetComponentTickEnabled(false);

	WeaponSkeletalMeshComponent->RegisterComponent();

	return WeaponSkeletalMeshComponent;
}

bool UBSWeaponComponent::AttachWeaponMeshComponent(UMeshComponent* MeshComponent,const UBSWeaponDefinition* WeaponDefinition)
{
	if (!MeshComponent || !WeaponDefinition)
	{
		return false;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return false;
	}

	USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh();

	const FName SocketName = WeaponDefinition->AttachSocketName;

	if (SocketName.IsNone() || !CharacterMesh->DoesSocketExist(SocketName))
	{
		UE_LOG(LogTemp,Warning, TEXT("캐릭터에 '%s' 소켓 없음"),*SocketName.ToString());
		return false;
	}

	MeshComponent->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	MeshComponent->SetRelativeTransform(WeaponDefinition->GripOffset);

	return true;
}

void UBSWeaponComponent::HideWeaponMeshComponents()
{
	if (WeaponStaticMeshComponent)
	{
		WeaponStaticMeshComponent->SetVisibility(false);
		WeaponStaticMeshComponent->SetHiddenInGame(true);
	}

	if (WeaponSkeletalMeshComponent)
	{
		WeaponSkeletalMeshComponent->SetVisibility(false);
		WeaponSkeletalMeshComponent->SetHiddenInGame(true);
		WeaponSkeletalMeshComponent->SetComponentTickEnabled(false);
	}
}





