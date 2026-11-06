
#include "Controller/BSPlayerController_Debug.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Character/BSPlayerCharacter.h"
#include "Character/BSEnemyCharacter.h"
#include "Component/BSWeaponComponent.h"
#include "Data/BSWeaponDefinition.h"
#include "Engine/World.h"
#include "GAS/BSBaseAttributeSet.h"
#include "Utility/BSGameplayTags.h"

void ABSPlayerController_Debug::DebugSpawnEnemies(FName EnemyId, int32 Count)
{
	ABSPlayerCharacter* PlayerCharacter = GetTestPlayer();
	if (!Player)
	{
		return;
	}

	const TSubclassOf<ABSEnemyCharacter>* EnemyClass = EnemyCatalog.Find(EnemyId);

	if (!EnemyClass || !EnemyClass->Get())
	{
		ClientMessage(FString::Printf(TEXT("몬스터 없음: %s"), *EnemyId.ToString()));
		return;
	}

	Count = FMath::Clamp(Count, 1, 30);

	const FRotator Facing(0.0f, PlayerCharacter->GetActorRotation().Yaw, 0.0f);

	const FVector Forward = Facing.Vector();
	const FVector Right = FRotationMatrix(Facing).GetUnitAxis(EAxis::Y);

	int32 SpawnedCount = 0;

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const int32 Row = Index / 5;
		const int32 Column = Index % 5;

		const FVector Location = PlayerCharacter->GetActorLocation() + Forward * (600.0f + Row * 180.0f) + Right * ((Column - 2) * 180.0f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		ABSEnemyCharacter* Enemy = GetWorld()->SpawnActor<ABSEnemyCharacter>(*EnemyClass, Location, (-Forward).Rotation(), Params);

		if (!Enemy)
		{
			continue;
		}

		// Spawned Pawn도 AIController를 갖도록 보완.
		if (!Enemy->GetController())
		{
			Enemy->SpawnDefaultController();
		}

		SpawnedEnemies.Add(Enemy);
		++SpawnedCount;
	}

	ClientMessage(FString::Printf(TEXT("%d / %d 몬스터 스폰"), SpawnedCount, Count));
}

void ABSPlayerController_Debug::DebugClearEnemies()
{
	// 제거된 대상을 락온한 상태를 정리.
	if (ABSPlayerCharacter* PlayerCharacter = Cast<ABSPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->ClearCombatTarget();
	}

	for (const TWeakObjectPtr<ABSEnemyCharacter>& Entry : SpawnedEnemies)
	{
		ABSEnemyCharacter* Enemy = Entry.Get();
		if (!IsValid(Enemy))
		{
			continue;
		}

		if (AAIController* AI = Cast<AAIController>(Enemy->GetController()))
		{
			AI->Destroy();
		}

		Enemy->Destroy();
	}
	SpawnedEnemies.Reset();
	ClientMessage(TEXT("디버그 몬스터 제거"));
}

void ABSPlayerController_Debug::DebugEquipWeapon(FName WeaponId)
{
	ABSPlayerCharacter* PlayerCharacter = GetTestPlayer();
	if (!PlayerCharacter)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();

	if (ASC && ASC->HasMatchingGameplayTag(BSGameplayTags::State_Attacking))
	{
		ClientMessage(TEXT("공격 끝나고 바꾸셈"));
		return;
	}

	UBSWeaponComponent* Weapon = PlayerCharacter->GetWeaponComponent();
	if (!Weapon)
	{
		return;
	}

	if (WeaponId.IsNone())
	{
		Weapon->UnequipWeapon();
		return;
	}

	const TObjectPtr<UBSWeaponDefinition>* Definition = WeaponCatalog.Find(WeaponId);

	if (!Definition || !IsValid(Definition->Get()))
	{
		ClientMessage(FString::Printf(TEXT("무기 없음: %s"), *WeaponId.ToString()));
		return;
	}

	Weapon->EquipWeapon(Definition->Get());
}

void ABSPlayerController_Debug::DebugHealHP()
{
	ABSPlayerCharacter* PlayerCharacter = GetTestPlayer();
	if (!PlayerCharacter)
	{
		return;
	}

	UBSBaseAttributeSet* AS = PlayerCharacter->GetAttributeSet();
	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();

	if (AS && ASC)
	{
		ASC->SetNumericAttributeBase(UBSBaseAttributeSet::GetCurrentHPAttribute(), AS->GetMaxHP());
	}
}
void ABSPlayerController_Debug::DebugHealMP()
{
	ABSPlayerCharacter* PlayerCharacter = GetTestPlayer();
	if (!PlayerCharacter)
	{
		return;
	}

	UBSBaseAttributeSet* AS = PlayerCharacter->GetAttributeSet();
	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();

	if (AS && ASC)
	{
		ASC->SetNumericAttributeBase(UBSBaseAttributeSet::GetCurrentMPAttribute(), AS->GetMaxMP());
	}
}

void ABSPlayerController_Debug::DebugHealStamina()
{
	ABSPlayerCharacter* PlayerCharacter = GetTestPlayer();
	if (!PlayerCharacter)
	{
		return;
	}

	UBSBaseAttributeSet* AS = PlayerCharacter->GetAttributeSet();
	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();

	if (AS && ASC)
	{
		ASC->SetNumericAttributeBase(UBSBaseAttributeSet::GetCurrentStaminaAttribute(), AS->GetMaxStamina());
	}
}

void ABSPlayerController_Debug::DebugFixHP(bool Enabled)
{
	ABSPlayerCharacter* PlayerCharacter = GetTestPlayer();
	if (!PlayerCharacter)
	{
		return;
	}

	if (UBSBaseAttributeSet* AS = PlayerCharacter->GetAttributeSet())
	{
		AS->SetDebugHPFixed(Enabled);
	}
}

void ABSPlayerController_Debug::DebugFixMP(bool Enabled)
{
	ABSPlayerCharacter* PlayerCharacter = GetTestPlayer();
	if (!PlayerCharacter)
	{
		return;
	}

	if (UBSBaseAttributeSet* AS = PlayerCharacter->GetAttributeSet())
	{
		AS->SetDebugMPFixed(Enabled);
	}
}

void ABSPlayerController_Debug::DebugFixStamina(bool Enabled)
{
	ABSPlayerCharacter* PlayerCharacter = GetTestPlayer();
	if (!PlayerCharacter)
	{
		return;
	}

	if (UBSBaseAttributeSet* AS = PlayerCharacter->GetAttributeSet())
	{
		AS->SetDebugStaminaFixed(Enabled);
	}
}

ABSPlayerCharacter* ABSPlayerController_Debug::GetTestPlayer() const
{
	ABSPlayerCharacter* PlayerCharacter = Cast<ABSPlayerCharacter>(GetPawn());
	return IsValid(PlayerCharacter) && !PlayerCharacter->IsDead() ? PlayerCharacter : nullptr;
}
