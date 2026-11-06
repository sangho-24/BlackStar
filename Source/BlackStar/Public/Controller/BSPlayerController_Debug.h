#pragma once

#include "CoreMinimal.h"
#include "Controller/BSPlayerController.h"
#include "BSPlayerController_Debug.generated.h"

class ABSPlayerCharacter;
class ABSEnemyCharacter;
class UBSWeaponDefinition;

UCLASS()
class BLACKSTAR_API ABSPlayerController_Debug : public ABSPlayerController
{
	GENERATED_BODY()

protected:
	// 예: Whisper -> BP_Enemy_Whisper
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DebugRoom")
	TMap<FName, TSubclassOf<ABSEnemyCharacter>> EnemyCatalog;

	// 예: RoyalSword -> DA_RoyalSword
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DebugRoom")
	TMap<FName, TObjectPtr<UBSWeaponDefinition>> WeaponCatalog;

public:
	UFUNCTION(Exec, BlueprintCallable, Category = "DebugRoom")
	void DebugSpawnEnemies(FName EnemyId, int32 Count = 1);

	UFUNCTION(Exec, BlueprintCallable, Category = "DebugRoom")
	void DebugClearEnemies();

	UFUNCTION(Exec, BlueprintCallable, Category = "DebugRoom")
	void DebugEquipWeapon(FName WeaponId);

	UFUNCTION(Exec, BlueprintCallable, Category = "DebugRoom")
	void DebugHealHP();
	
	UFUNCTION(Exec, BlueprintCallable, Category = "DebugRoom")
	void DebugHealMP();

	UFUNCTION(Exec, BlueprintCallable, Category = "DebugRoom")
	void DebugHealStamina();

	UFUNCTION(Exec, BlueprintCallable, Category = "DebugRoom")
	void DebugFixHP(bool Enabled);
	
	UFUNCTION(Exec, BlueprintCallable, Category = "DebugRoom")
	void DebugFixMP(bool Enabled);

	UFUNCTION(Exec, BlueprintCallable, Category = "DebugRoom")
	void DebugFixStamina(bool Enabled);
	
	UFUNCTION(BlueprintCallable, Category = "DebugRoom")
	TMap<FName, TSubclassOf<ABSEnemyCharacter>> GetEnemyCatalog() const { return EnemyCatalog; }

private:
	TArray<TWeakObjectPtr<ABSEnemyCharacter>> SpawnedEnemies;

	ABSPlayerCharacter* GetTestPlayer() const;
};