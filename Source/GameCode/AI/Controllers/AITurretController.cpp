// Fill out your copyright notice in the Description page of Project Settings.


#include "AITurretController.h"

#include "GameCode/AI/Characters/Turret.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (InPawn)
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("AAITurretController::SetPawn() - Pawn is not a turret"));
		CachedTurret = StaticCast<ATurret*>(InPawn);
	}
	else
	{
		CachedTurret = nullptr;
	}
}

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);

	if (!CachedTurret.IsValid())
	{
		return;
	}

	TArray<TSubclassOf<UAISense>> Senses;
	Senses.Add(UAISense_Sight::StaticClass());
	Senses.Add(UAISense_Damage::StaticClass());

	TArray<AActor*> PotentialTargets;
	AActor* ClosestSensedActor = GetClosestSensedActor(Senses);
	CachedTurret->CurrentTarget = ClosestSensedActor;
	CachedTurret->OnCurrentTargetSet();
}
