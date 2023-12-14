// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCode/Actors/Equipment/EquipableItem.h"
#include "RangeWeaponItem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnReloadComplete)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32)

UENUM()
enum class EReloadType : uint8
{
	FullClip,
	ByBullet
};

UENUM()
enum class EWeaponFireMode : uint8
{
	Single,
	FullAuto
};

class UAnimMontage;

UCLASS(Blueprintable)
class GAMECODE_API ARangeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	ARangeWeaponItem();

	void StartFire();
	void StopFire();
	FTransform GetForeGripTransform() const;
	void StartAim();
	void StopAim();

	float GetAimFOV() const { return AimFOV; }
	float GetAimMovementMaxSpeed() const { return AimMovementMaxSpeed; }

	float GetAimTurnModifier() const { return bIsAiming ? AimTurnModifier : 1.f; }
	float GetAimLookUpModifier() const { return bIsAiming ? AimLookUpModifier : 1.f; }

	int32 GetAmmo() const { return Ammo; }
	void SetAmmo(const int32 NewAmmo);
	bool CanShoot();

	void StartReload();
	void EndReload(bool bIsSucces);

	FOnAmmoChanged OnAmmoChanged;
	FOnReloadComplete OnReloadComplete;

	EAmunitionType GetAmmoType() const { return AmmoType; }
	int32 GetMaxAmmo() const { return MaxAmmo; }

	virtual EReticleType GetReticleType() const override;

	bool IsFiring() const
	{
		return bIsFiring;
	}

	bool IsReloading() const
	{
		return bIsReloading;
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UWeaponBarellComponent* WeaponBarell;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations | Weapon")
	UAnimMontage* WeaponFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations | Weapon")
	UAnimMontage* WeaponReloadMontage;

	// FullClip reload adds ammo only when  the whole animation is successfully played
	// ByBullet reload type requires section ReloadEnd in character reload animation has 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations | Weapon")
	EReloadType ReloadType = EReloadType::FullClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations | Character")
	UAnimMontage* CharacterFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations | Character")
	UAnimMontage* CharacterReloadMontage;

	// Rate of fire in rounds per minute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon | Parameters", meta = (ClampMin=1.f, UIMin=1.f))
	float RateOfFire = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon | Parameters")
	EWeaponFireMode WeaponFireMode = EWeaponFireMode::Single;

	// Bullet spread half in degrees
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon | Parameters",
		meta=(ClampMin=0.f, UIMin=0.f, UIMax=2.f, UIMax=2.f))
	float SpreadAngle = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon | Parameters | Aiming",
		meta=(ClampMin=0.f, UIMin=0.f, UIMax=2.f, UIMax=2.f))
	float AimSpreadAngle = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon | Parameters | Aiming",
		meta=(ClampMin=0.f, UIMin=0.f))
	float AimMovementMaxSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon | Parameters | Aiming",
		meta=(ClampMin=0.f, UIMin=0.f, ClampMax=120.f, UIMax=120.f))
	float AimFOV = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon | Parameters | Ammo",
		meta=(ClampMin=1.f, UIMin=1.f))
	int32 MaxAmmo = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon | Parameters | Ammo",
		meta=(ClampMin=1.f, UIMin=1.f))
	EAmunitionType AmmoType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon | Parameters | Ammo")
	bool bAutoReload = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reticle")
	EReticleType AimReticleType = EReticleType::Default;

private:
	float GetShotTimerInterval() { return 60.f / RateOfFire; }
	void MakeShot();
	float GetCurrentBulletSpreadAngle();
	float PlayAnimMontage(UAnimMontage* AnimMontage);
	void StopAnimMontage(UAnimMontage* AnimMontage, float BlendOutTime = 0.f);
	void OnShotTimerElapsed();

	FTimerHandle ShotTimer;
	bool bIsFiring = false;
	bool bIsAiming;

	bool bIsReloading = false;
	FTimerHandle ReloadTimer;

	float AimTurnModifier = 0.5f;
	float AimLookUpModifier = 0.5f;

	int32 Ammo = 0;
};
