#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "BSBaseCharacter.generated.h"

class UAbilitySystemComponent;
class UBSBaseAttributeSet;
class UGameplayAbility;

UCLASS()
class BLACKSTAR_API ABSBaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABSBaseCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UBSBaseAttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Abilities|Setup")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 인라인 함수
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	FORCEINLINE UBSBaseAttributeSet* GetAttributeSet() const { return AttributeSet; }
};
