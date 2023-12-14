// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCProjectile.h"
#include "GameCode/ExplosionComponent.h"
#include "ExplosiveProjectile.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API AExplosiveProjectile : public AGCProjectile
{
	GENERATED_BODY()

public:
	AExplosiveProjectile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UExplosionComponent* ExplosionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Explosion")
	float ExplosionDelay = 2.f;

	virtual void OnProjectileLaunched() override;

private:
	FTimerHandle ExplosionTimerHandle;
	AController* GetController();
	void OnExplosionTimerElapsed();
};
