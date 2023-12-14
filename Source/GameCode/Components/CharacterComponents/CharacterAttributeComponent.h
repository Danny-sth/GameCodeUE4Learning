// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterAttributeComponent.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnDeathEventSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FOutOfStaminaEventSignature, bool)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, HealthPercent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, StaminaPercent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOxygenChanged, float, OxygenPercent);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMECODE_API UCharacterAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterAttributeComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	FOnDeathEventSignature OnDeathEvent;
	bool IsAlive() const { return Health > 0.f; }

	FOutOfStaminaEventSignature OutOfStaminaEvent;
	FOnHealthChanged OnHealthChangedEvent;
	FOnStaminaChanged OnStaminaChanged;
	FOnOxygenChanged OnOxygenChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.0f))
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRestoreVelocity = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float SprintStaminaConsumptionVelocity = 20.0f;
	bool IsSprinting();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen")
	float MaxOxygen = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen")
	float OxygenRestoreVelocity = 15.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen")
	float SwimOxygenConsumptionVelocity = 2.0f;

private:
	UPROPERTY(ReplicatedUsing=OnRep_Health)
	float Health = 0.f;

	UFUNCTION()
	void OnRep_Health();
	void OnHealthChanged();

	float Stamina = 0.f;
	float Oxygen = 0.f;


#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DebugDrawAttributes();
#endif

	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	                     AController* InstigatedBy, AActor* DamageCauser);

	TWeakObjectPtr<class AGCBaseCharacter> CachedBaseCharacter;

	void UpdateStaminaValue(float DeltaTime);
	void ConsumeStamina(float DeltaTime);
	void RestoreStamina(float DeltaTime);

	void UpdateOxygenValue(float DeltaTime);
	void ConsumeOxygen(float DeltaTime);
	void RestoreOxygen(float DeltaTime);

	void ShowAttribute3D(float Offset, FString Text, FColor Color);

	void BroadCastHealthValue();
	float GetHealthPercent() const;
	void BroadCastStaminaValue();
	void BroadCastOxygenValue();
};
