// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GCProjectile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnProjectileHit, AGCProjectile*, Projectile, const FHitResult&, Hit,
                                               const FVector&, Direction);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExplode, AGCProjectile*, Projectile);


UCLASS()
class GAMECODE_API AGCProjectile : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnProjectileHit OnProjectileHit;

	FOnExplode OnThrowableItemExplode;

	AGCProjectile();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void LaunchProjectile(const FVector& ShootDirection);

	UFUNCTION(BlueprintNativeEvent)
	void SetProjectileActive(bool bIsProjectileActive);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UProjectileMovementComponent* ProjectileMovementComponent;

	virtual void OnProjectileLaunched();

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	           FVector NormalImpulse, const FHitResult& Hit);
};
