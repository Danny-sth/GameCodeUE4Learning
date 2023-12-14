// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameCode/GameCodeTypes.h"
#include "AIPatrollingComponent.generated.h"

class APatrollingPath;

USTRUCT(BlueprintType)
struct FPatrollingParams
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Path")
	EPatrollingType PatrollingType = EPatrollingType::Circle;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Path")
	APatrollingPath* PatrollingPath;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMECODE_API UAIPatrollingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	bool CanPatrol() const;
	FVector SelectClosestWayPoint();
	FVector SelectNextWayPoint();

	EPatrollingType GetPatrollingType() const { return PatrollingParams.PatrollingType; }
	APatrollingPath* GetPatrollingPath() const { return PatrollingParams.PatrollingPath; }

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Path")
	FPatrollingParams PatrollingParams;

private:
	int32 CurrentWayPointIndex = -1;
	bool bIsReverseDirection = false;

	void SwitchDirection();
	int32 GetLastWaypointIndex();
	FVector GetNextByOrderWaypoint();
	FVector GetPreviousByOrderWaypoint();
};
