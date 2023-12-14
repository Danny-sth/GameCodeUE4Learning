// Fill out your copyright notice in the Description page of Project Settings.


#include "BTServiceThrow.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameCode/Characters/GCBaseCharacter.h"

UBTServiceThrow::UBTServiceThrow()
{
	NodeName = "Throw";
	Interval = 5.f;
}

void UBTServiceThrow::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	AAIController* AaiController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();

	if (!AaiController || !BlackboardComponent)
	{
		return;
	}

	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(AaiController->GetPawn());
	if (!Character)
	{
		return;
	}

	AActor* CurrentTargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(CurrenTargetKey.SelectedKeyName));
	if (!CurrentTargetActor)
	{
		return;
	}

	float DistSq = FVector::DistSquared(CurrentTargetActor->GetActorLocation(), Character->GetActorLocation());
	if (DistSq > FMath::Square(ThrowDistanceInterval.MaxThrowDistance) ||
		DistSq < FMath::Square(ThrowDistanceInterval.MinThrowDistance))
	{
		return;
	}
	Character->ThrowPrimaryItem();
}
