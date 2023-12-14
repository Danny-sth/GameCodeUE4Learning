// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCAIController.h"
#include "GameCode/AI/Characters/GCAICharacter.h"
#include "GCAICharacterController.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API AGCAICharacterController : public AGCAIController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;
	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float TargetReachedRadius = 100.f;

	virtual void BeginPlay() override;

private:
	TWeakObjectPtr<AGCAICharacter> CachedAICharacter;

	bool bIsPatrolling = false;

	void TryMoveToNextTarget();
	bool IsTargetReached(FVector TargetLocation) const;
};
