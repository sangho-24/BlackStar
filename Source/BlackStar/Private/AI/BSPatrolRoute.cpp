#include "AI/BSPatrolRoute.h"
#include "Components/SplineComponent.h"

ABSPatrolRoute::ABSPatrolRoute()
{
	PrimaryActorTick.bCanEverTick = false;
	
	PatrolSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PatrolSpline"));
	RootComponent = PatrolSpline;
}

int32 ABSPatrolRoute::GetNumPatrolPoints() const
{
	return PatrolSpline ? PatrolSpline->GetNumberOfSplinePoints() : 0;
}

bool ABSPatrolRoute::IsValidPatrolPointIndex(int32 PointIndex) const
{
	return PointIndex >= 0 && PointIndex < GetNumPatrolPoints();
}

FVector ABSPatrolRoute::GetPatrolPointLocation(int32 PointIndex) const
{
	if (!PatrolSpline || !IsValidPatrolPointIndex(PointIndex))
	{
		return GetActorLocation();
	}

	return PatrolSpline->GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::World);
}

int32 ABSPatrolRoute::FindClosestPatrolPointIndex(const FVector& WorldLocation) const
{
	const int32 PointCount = GetNumPatrolPoints();

	if (PointCount <= 0)
	{
		return INDEX_NONE;
	}

	int32 ClosestIndex = 0;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();

	for (int32 Index = 0; Index < PointCount; ++Index)
	{
		const FVector PointLocation = GetPatrolPointLocation(Index);

		const float DistanceSquared = FVector::DistSquared2D(WorldLocation, PointLocation);

		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestIndex = Index;
		}
	}
	return ClosestIndex;
}



