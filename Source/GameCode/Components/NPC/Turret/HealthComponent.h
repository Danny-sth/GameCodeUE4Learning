// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathEvent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMECODE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();
	virtual void BeginPlay() override;

	FOnDeathEvent OnDeathEvent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta=(ClampMin="0.0", ClampMin="0.0"))
	float MaxHealth = 100.0f;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	                  AController* InstigatedBy, AActor* DamageCauser);

private:
	float CurrentHealth = 0.0f;

	bool IsDead();
};
