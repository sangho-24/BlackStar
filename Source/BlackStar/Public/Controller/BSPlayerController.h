// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayEffectTypes.h"
#include "BSPlayerController.generated.h"

class UHUDWidget;
class ABSPlayerCharacter;
class UBSBaseAttributeSet;
class UAbilitySystemComponent;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class BLACKSTAR_API ABSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Setup")
	TSubclassOf<UHUDWidget> HUDWidgetClass;
	
private:
	UPROPERTY()
	UHUDWidget* HUDWidget;
	// 최적화용 캐시
	UPROPERTY()
	TObjectPtr<ABSPlayerCharacter> CachedPlayer;
	UPROPERTY()
	TObjectPtr<UBSBaseAttributeSet> CachedAS;
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
private:
	void BindHUDDelegates();
	void UnbindHUDDelegates();
	// 델리게이트 콜백
	void OnHPChanged(const FOnAttributeChangeData& Data);
	void OnMPChanged(const FOnAttributeChangeData& Data);
	void OnStatsChanged(const FOnAttributeChangeData& Data);
};
