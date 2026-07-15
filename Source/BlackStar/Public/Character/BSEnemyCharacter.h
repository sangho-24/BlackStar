// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BSBaseCharacter.h"
#include "BSEnemyCharacter.generated.h"

class UWidgetComponent;
class UUserWidget;
struct FOnAttributeChangeData;

UCLASS()
class BLACKSTAR_API ABSEnemyCharacter : public ABSBaseCharacter
{
	GENERATED_BODY()

protected:
	// ===== UI =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent *NameplateWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Setup")
	TSubclassOf<UUserWidget> NameplateWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Setup")
	FString EnemyName = TEXT("Enemy");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Setup")
	float NameplateHeight = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Setup")
	FVector2D NameplateSize = FVector2D(300.0f, 75.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Setup")
	float CorpseLifeTime = 5.0f;

	// ===== AI =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<ABSBaseCharacter> CombatTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	FVector LastKnownTargetLocation = FVector::ZeroVector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	bool bHasLastKnownTargetLocation = false;

private:
	FDelegateHandle CurrentHPDelegateHandle;
	FDelegateHandle MaxHPDelegateHandle;

public:
	ABSEnemyCharacter();
	void InitializeNameplate();

	virtual AActor *GetCombatTarget() const override;
	virtual void SetCombatTarget(AActor *NewTarget) override;
	virtual void ClearCombatTarget() override;

	void SetLastKnownTargetLocation(const FVector& Location);
	FVector GetLastKnownTargetLocation() const { return LastKnownTargetLocation; }
	bool HasLastKnownTargetLocation() const { return bHasLastKnownTargetLocation; }
	void ClearLastKnownTargetLocation();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnDeathStarted(AActor *Killer) override;
	virtual void OnDeathFinished(AActor *Killer) override;

private:
	void RefreshNameplate();
	void OnHPChanged(const FOnAttributeChangeData &Data);
};
