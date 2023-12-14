// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTServiceThrow.generated.h"

USTRUCT(BlueprintType)
struct FThrowDistanceInterval
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinThrowDistance = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxThrowDistance = 1500.0;
};

UCLASS()
class GAMECODE_API UBTServiceThrow : public UBTService
{
	GENERATED_BODY()

public:
	UBTServiceThrow();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Throw params")
	FBlackboardKeySelector CurrenTargetKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Throw params")
	FThrowDistanceInterval ThrowDistanceInterval;
};
