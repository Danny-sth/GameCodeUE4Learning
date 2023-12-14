// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameCode/Actors/Projectles/GCProjectile.h"
#include "WeaponBarellComponent.generated.h"

UENUM()
enum class EHitRegistrationType : uint8
{
	HitScan,
	Projectile
};

USTRUCT(BlueprintType)
struct FDecalInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Decal Info")
	UMaterialInterface* DecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Decal Info")
	FVector DecalSize = FVector(5.f, 5.f, 5.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Decal Info")
	float DecalLifeTime = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Decal Info")
	float DecalFadeOutTime = 5.f;
};

USTRUCT(BlueprintType)
struct FShotInfo
{
	GENERATED_BODY()

	FShotInfo() : Location_Mul_10(FVector_NetQuantize100::ZeroVector), Direction(FVector_NetQuantizeNormal::ZeroVector)
	{
	};

	FShotInfo(FVector Location, FVector Direction): Location_Mul_10(Location * 10.f), Direction(Direction)
	{
	};

	UPROPERTY()
	FVector_NetQuantize100 Location_Mul_10; // Местоположение умноженное на 10

	UPROPERTY()
	FVector_NetQuantizeNormal Direction;

	FVector GetLocation() const { return Location_Mul_10 * 0.1f; }
	FVector GetDirection() const { return Direction; }
};

class UNiagaraSystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMECODE_API UWeaponBarellComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UWeaponBarellComponent();

	virtual void BeginPlay() override;

	void Shot(FVector ShotStart, FVector ShotDirection, float SpreadAngle);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes")
	float FiringRange = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes", meta = (ClampMin=1, UIMin=1))
	int32 BulletsPerShot = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes | Hit Registration")
	EHitRegistrationType HitRegistrationType = EHitRegistrationType::HitScan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes | Hit Registration", meta = (UIMin = 1))
	int32 ProjectilePoolSize = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes | Hit Registration",
		meta = (EditCondition = "HitRegistrationType == EHitRegistrationType::Projectile"))
	TSubclassOf<AGCProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes | Damage")
	float DamageAmount = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes | Damage")
	UCurveFloat* FalloffDiagram;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes | Damage")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes | VFX")
	UNiagaraSystem* MuzzleFlashFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes | VFX")
	UNiagaraSystem* TraceFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Barrel Attributes | Decals")
	FDecalInfo DefaultDecalInfo;

private:
	void ShotInternal(const TArray<FShotInfo> Infos);

	UFUNCTION(Server, Reliable)
	void Server_Shot(const TArray<FShotInfo>& Infos);

	UPROPERTY(ReplicatedUsing=OnRep_LastShotsInfo)
	TArray<FShotInfo> LastShotsInfo;

	UPROPERTY(Replicated)
	TArray<AGCProjectile*> ProjectilePool;

	UPROPERTY(Replicated)
	int32 CurrentProjectileIndex;

	UFUNCTION()
	void OnRep_LastShotsInfo();

	bool HitScan(FVector ShotStart, FVector& ShotEnd, FVector ShotDirection);
	float CalculateDamageWithFallof(float BaseDamage, float Distance);
	FVector GetBulletSpreadOffset(float Angle, FRotator ShotRotation);
	APawn* GetOwningPawn() const;
	AController* GetController() const;
	void LaunchProjectile(const FVector& LaunchStart, const FVector LaunchDirection);

	UFUNCTION()
	void ProcessHit(const FHitResult& Hit, const FVector& ShotDirection);

	UFUNCTION()
	void ProcessProjectileHit(AGCProjectile* Projectile, const FHitResult& Hit, const FVector& ShotDirection);

	const FVector ProjectilePoolLocation = FVector(0, 0, -100);
};
