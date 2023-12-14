// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "MeleeHitRegistrator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeleeHit, const FHitResult&, HitResult, const FVector&, HitDirection);

UCLASS(meta = (BlueprintSpawnableComponent))
class GAMECODE_API UMeleeHitRegistrator : public USphereComponent
{
	GENERATED_BODY()

public:
	FOnMeleeHit OnMeleeHit;

	UMeleeHitRegistrator();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void ProcessHitRegistration();
	void SetHitRegistratorActive(const bool bIsActive_in) { bIsHitRegistratorActive = bIsActive_in; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Hit Registrator")
	bool bIsHitRegistratorActive = false;

private:
	FVector PreviousLocation = FVector::ZeroVector;
};
