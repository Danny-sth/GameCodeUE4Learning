// Fill out your copyright notice in the Description page of Project Settings.


#include "GCAIController.h"

#include "Perception/AIPerceptionComponent.h"

AGCAIController::AGCAIController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
}

AActor* AGCAIController::GetClosestSensedActor(TArray<TSubclassOf<UAISense>> SenseClasses) const
{
	if (!GetPawn())
	{
		return nullptr;
	}

	TArray<AActor*> PotentialTargets;
	for (const auto Sense : SenseClasses)
	{
		PerceptionComponent->GetCurrentlyPerceivedActors(Sense, PotentialTargets);
	}

	AActor* ClosestActor = nullptr;
	float MinSquaredDistance = FLT_MAX;

	for (const auto Target : PotentialTargets)
	{
		const float CurrentSquaredDistance = (GetPawn()->GetActorLocation() - Target->GetActorLocation()).SizeSquared();
		if (CurrentSquaredDistance < MinSquaredDistance)
		{
			MinSquaredDistance = CurrentSquaredDistance;
			ClosestActor = Target;
		}
	}
	return ClosestActor;
}
