#pragma once

#include "CoreMinimal.h"
#include "Character/BSEnemyCharacter.h"
#include "BSNormalEnemyCharacter.generated.h"

class ABSPatrolRoute;
class UWidgetComponent;
class UUserWidget;
struct FOnAttributeChangeData;

UCLASS()
class BLACKSTAR_API ABSNormalEnemyCharacter : public ABSEnemyCharacter
{
    GENERATED_BODY()

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UWidgetComponent> NameplateWidgetComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Setup")
    TSubclassOf<UUserWidget> NameplateWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Setup")
    float NameplateHeight = 1.4f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Setup")
    FVector2D NameplateSize = FVector2D(300.0f, 50.0f);

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI")
    bool bHasLastKnownTargetLocation = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Patrol")
    FVector PatrolOrigin = FVector::ZeroVector;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Patrol|Setup", meta = (ExposeOnSpawn = "true"))
    TObjectPtr<ABSPatrolRoute> PatrolRoute = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Patrol")
    int32 CurrentPatrolPointIndex = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Patrol")
    int32 PatrolDirection = 1;

public:
    ABSNormalEnemyCharacter();

    void InitializeNameplate();

    FVector GetPatrolOrigin() const
    {
        return PatrolOrigin;
    }

    ABSPatrolRoute* GetPatrolRoute() const
    {
        return PatrolRoute;
    }

    void SetLastKnownTargetLocation(const FVector& Location);

    FVector GetLastKnownTargetLocation() const
    {
        return LastKnownTargetLocation;
    }

    bool HasLastKnownTargetLocation() const
    {
        return bHasLastKnownTargetLocation;
    }

    void ClearLastKnownTargetLocation();

    bool HasPatrolRoute() const;
    FVector GetCurrentPatrolPointLocation() const;
    void InitializePatrolPoint();
    void AdvancePatrolPoint();

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void OnDeathStarted(AActor* Killer) override;

private:
    FDelegateHandle CurrentHPDelegateHandle;
    FDelegateHandle MaxHPDelegateHandle;

    void RefreshNameplate();
    void OnHPChanged(const FOnAttributeChangeData& Data);
};