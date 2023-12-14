// Fill out your copyright notice in the Description page of Project Settings.


#include "AIPatrollingComponent.h"

#include "GameCode/Actors/Navigation/PatrollingPath.h"

bool UAIPatrollingComponent::CanPatrol() const
{
	return GetPatrollingPath() && GetPatrollingPath()->GetWayPoints().Num() > 0;
}

FVector UAIPatrollingComponent::SelectClosestWayPoint()
{
	FVector ActorLocation = GetOwner()->GetActorLocation();
	auto WayPoints = GetPatrollingPath()->GetWayPoints();

	FVector ClosestWayPoint;
	float MinSqDistance = FLT_MAX;
	for (int32 i = 0; i < GetPatrollingPath()->GetWayPoints().Num(); ++i)
	{
		FVector WayPointWorldTransform = GetPatrollingPath()->GetTransform().TransformPosition(WayPoints[i]);
		float CurrentSqDistance = (ActorLocation - WayPointWorldTransform).SizeSquared();
		if (CurrentSqDistance < MinSqDistance)
		{
			ClosestWayPoint = WayPointWorldTransform;
			CurrentWayPointIndex = i;
			MinSqDistance = CurrentSqDistance;
		}
	}
	return ClosestWayPoint;
}


FVector UAIPatrollingComponent::SelectNextWayPoint()
{
	switch (GetPatrollingType())
	{
	case EPatrollingType::Circle:
		{
			return GetNextByOrderWaypoint();
		}
	case EPatrollingType::PingPong:
		{
			SwitchDirection();
			return bIsReverseDirection ? GetPreviousByOrderWaypoint() : GetNextByOrderWaypoint();
		}
	}
	return FVector::ZeroVector;
}

FVector UAIPatrollingComponent::GetNextByOrderWaypoint()
{
	CurrentWayPointIndex == GetLastWaypointIndex() ? CurrentWayPointIndex = 0 : ++CurrentWayPointIndex;
	return GetPatrollingPath()->GetTransform()
	                          .TransformPosition(GetPatrollingPath()->GetWayPoints()[CurrentWayPointIndex]);
}

FVector UAIPatrollingComponent::GetPreviousByOrderWaypoint()
{
	CurrentWayPointIndex == 0 ? CurrentWayPointIndex = GetLastWaypointIndex() : --CurrentWayPointIndex;
	return GetPatrollingPath()->GetTransform()
	                          .TransformPosition(GetPatrollingPath()->GetWayPoints()[CurrentWayPointIndex]);
}

int32 UAIPatrollingComponent::GetLastWaypointIndex()
{
	return GetPatrollingPath()->GetWayPoints().Num() - 1;
}

void UAIPatrollingComponent::SwitchDirection()
{
	if (CurrentWayPointIndex == GetLastWaypointIndex())
	{
		bIsReverseDirection = true;
	}
	if (CurrentWayPointIndex == 0)
	{
		bIsReverseDirection = false;
	}
}
