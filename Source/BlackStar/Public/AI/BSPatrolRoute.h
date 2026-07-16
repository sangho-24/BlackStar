#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BSPatrolRoute.generated.h"

class USplineComponent;

// PingPong은 1-2-3-2-1 / Loop는 1-2-3-1-2-3
UENUM(BlueprintType)
enum class EBSPatrolRouteMode : uint8
{
	Loop,
	PingPong
};

UCLASS()
class BLACKSTAR_API ABSPatrolRoute : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	TObjectPtr<USplineComponent> PatrolSpline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	EBSPatrolRouteMode PatrolMode = EBSPatrolRouteMode::PingPong;

	
public:	
	ABSPatrolRoute();
	
	int32 GetNumPatrolPoints() const;
	bool IsValidPatrolPointIndex(int32 PointIndex) const;
	FVector GetPatrolPointLocation(int32 PointIndex) const;
	int32 FindClosestPatrolPointIndex(const FVector& WorldLocation) const;

	EBSPatrolRouteMode GetPatrolMode() const { return PatrolMode; }

};
