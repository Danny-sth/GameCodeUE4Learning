// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCode/GameCodeTypes.h"
#include "GameFramework/Pawn.h"
#include "Turret.generated.h"

class UHealthComponent;
class UWeaponBarellComponent;

UENUM(BlueprintType)
enum class ETurretState : uint8
{
	Searching,
	Firing
};

UCLASS()
class GAMECODE_API ATurret : public APawn
{
	GENERATED_BODY()

public:
	ATurret();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;

	void OnCurrentTargetSet();

	virtual FVector GetPawnViewLocation() const override;
	virtual FRotator GetViewRotation() const override;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentTarget)
	AActor* CurrentTarget = nullptr;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TurretBaseComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TurretBarrelComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponBarellComponent* WeaponBarell;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret params", meta=(ClampMin="0.0", ClampMin="0.0"))
	float BaseSearchingRotationRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret params", meta=(ClampMin="0.0", ClampMin="0.0"))
	float BaseFiringInerpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret params", meta=(ClampMin="0.0", ClampMin="0.0"))
	float BarrelPitchRotationRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret params", meta=(ClampMin="0.0", ClampMin="0.0"))
	float MaxBarrelPitchAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret params", meta=(ClampMin="0.0", ClampMin="0.0"))
	float MinBarrelPitchAngle = -30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret params | Fire", meta=(ClampMin="0.0", ClampMin="0.0"))
	float FireRate = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret params | Fire", meta=(ClampMin="0.0", ClampMin="0.0"))
	float BulletSpreadAngle = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret params | Fire", meta=(ClampMin="0.0", ClampMin="0.0"))
	float FireDelayTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret params | Team")
	ETeams Team = ETeams::Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Explosion VFX")
	UParticleSystem* ExplosionFX;

	UFUNCTION()
	void OnDeath();

private:
	ETurretState CurrentState = ETurretState::Searching;
	FTimerHandle ShotTimerHandle;

	void MakeShot();
	void SetCurrentTurretState(ETurretState NewState);

	void SearchingMovement(float DeltaTime);
	void FiringMovement(float DeltaTime);
	float GetFireInterval() const;

	UFUNCTION()
	void OnRep_CurrentTarget();
};
