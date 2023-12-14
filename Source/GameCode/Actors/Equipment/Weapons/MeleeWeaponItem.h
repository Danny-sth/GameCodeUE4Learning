// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCode/Actors/Equipment/EquipableItem.h"
#include "GameCode/Components/Weapon/MeleeHitRegistrator.h"
#include "MeleeWeaponItem.generated.h"


USTRUCT(BlueprintType)
struct FMeleeAttackDescription
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Melee Attack")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Melee Attack", meta=(ClampMin=0.f, UIMin=0.f))
	float Damage = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Melee Attack")
	UAnimMontage* AttackMontage;
};

UCLASS(Blueprintable)
class GAMECODE_API AMeleeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()

public:
	AMeleeWeaponItem();

	void StartAttack(EMeleeAttackType AttackType);
	virtual void BeginPlay() override;
	void SetHitRegistrationActive(bool bIsActive_in);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee Attack")
	TMap<EMeleeAttackType, FMeleeAttackDescription> Attacks;

private:
	UFUNCTION()
	void ProcessHit(const FHitResult& HitResult, const FVector& HitDirection);

	TArray<UMeleeHitRegistrator*> Registrators;
	TSet<AActor*> HittedActors;

	FTimerHandle AttackTimerHandle;
	FMeleeAttackDescription* CurrentAttack;

	void OnAttackTimerFinished();
};
