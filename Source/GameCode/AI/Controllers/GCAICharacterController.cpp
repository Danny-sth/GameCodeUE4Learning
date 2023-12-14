// Fill out your copyright notice in the Description page of Project Settings.


#include "GCAICharacterController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameCode/Components/CharacterComponents/AIPatrollingComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"

void AGCAICharacterController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (InPawn)
	{
		checkf(InPawn->IsA<AGCAICharacter>(), TEXT("AGCAICharacterController::SetPawn() - Pawn is not a character"));
		CachedAICharacter = StaticCast<AGCAICharacter*>(InPawn);
		RunBehaviorTree(CachedAICharacter->GetBehaviorTree());
	}
	else
	{
		CachedAICharacter = nullptr;
	}
}

void AGCAICharacterController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);
	if (!CachedAICharacter.IsValid())
	{
		return;
	}
	TryMoveToNextTarget();
}

void AGCAICharacterController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	if (!Result.IsSuccess())
	{
		return;
	}
	TryMoveToNextTarget();
}

void AGCAICharacterController::BeginPlay()
{
	Super::BeginPlay();
	UAIPatrollingComponent* AIPatrollingComponent = CachedAICharacter->GetAIPatrollingComponent();
	if (AIPatrollingComponent->CanPatrol())
	{
		FVector ClosestWayPoint = AIPatrollingComponent->SelectClosestWayPoint();
		if (Blackboard)
		{
			Blackboard->SetValueAsVector(BB_NextLocation, ClosestWayPoint);
			Blackboard->SetValueAsObject(BB_CurrentTarget, nullptr);
		}
		bIsPatrolling = true;
	}
}

void AGCAICharacterController::TryMoveToNextTarget()
{
	auto UaiPatrollingComponent = CachedAICharacter->GetAIPatrollingComponent();
	TArray<TSubclassOf<UAISense>> Senses;
	Senses.Add(UAISense_Sight::StaticClass());

	AActor* ClosestSensedActor = GetClosestSensedActor(Senses);
	if (ClosestSensedActor)
	{
		if (Blackboard)
		{
			Blackboard->SetValueAsObject(BB_CurrentTarget, ClosestSensedActor);
			SetFocus(ClosestSensedActor, EAIFocusPriority::Gameplay);
		}
		bIsPatrolling = false;
	}
	else if (UaiPatrollingComponent->CanPatrol())
	{
		FVector NextWayPoint = bIsPatrolling
			                       ? UaiPatrollingComponent->SelectNextWayPoint()
			                       : UaiPatrollingComponent->SelectClosestWayPoint();
		if (Blackboard)
		{
			Blackboard->SetValueAsVector(BB_NextLocation, NextWayPoint);
			Blackboard->SetValueAsObject(BB_CurrentTarget, nullptr);
			ClearFocus(EAIFocusPriority::Gameplay);
		}
		bIsPatrolling = true;
	}
}

bool AGCAICharacterController::IsTargetReached(FVector TargetLocation) const
{
	return (TargetLocation - CachedAICharacter->GetActorLocation()).SizeSquared() <= FMath::Square(TargetReachedRadius);
}
