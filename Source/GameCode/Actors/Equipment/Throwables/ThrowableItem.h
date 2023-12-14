// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCode/Actors/Equipment/EquipableItem.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "ThrowableItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThrowableAmmoChanged, int32, NewAmmo);

/**
 * 
 */
UCLASS(Blueprintable)
class GAMECODE_API AThrowableItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	AThrowableItem();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnExplode(AGCProjectile* Projectile);

	void Throw();
	int32 GetAmmoAmount() const { return AmmoAmount; }
	void SetThrowableAmmoAmount(const int32 NewAmmoAmount);
	EAmunitionType GetAmmoType() const { return AmmoType; };

	FOnThrowableAmmoChanged OnThrowableAmmoChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwables")
	TSubclassOf<class AGCProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Throwables",
		meta = (UIMin = -90.f, UIMax = 90.f, ClampMin = -90.f, ClampMax = 90.f))
	float ThrowAngle = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwables | Parameters | Ammo",
		meta=(ClampMin=1.f, UIMin=1.f))
	int32 MaxAmmoAmount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwables | Parameters | Ammo",
		meta=(ClampMin=1.f, UIMin=1.f))
	EAmunitionType AmmoType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwables | Pool", meta = (UIMin = 1))
	int32 ThrowableItemsPoolSize = 10;

private:
	int32 AmmoAmount = 0;
	bool CanThrow() const;

	UPROPERTY(Replicated)
	TArray<AGCProjectile*> ThrowableItemsPool;

	UPROPERTY(Replicated)
	int32 CurrentThrowableIndex;

	const FVector ThrowableItemsPoolLocation = FVector(0.f, 0.f, -100.f);
};
