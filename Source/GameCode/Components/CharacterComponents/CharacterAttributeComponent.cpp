// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributeComponent.h"

#include "DrawDebugHelpers.h"
#include "GameCode/GameCodeTypes.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "GameCode/Subsystems/DebugSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UCharacterAttributeComponent::UCharacterAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharacterAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterAttributeComponent, Health)
}

float UCharacterAttributeComponent::GetHealthPercent() const
{
	return Health / MaxHealth;
}

void UCharacterAttributeComponent::BroadCastHealthValue()
{
	OnHealthChangedEvent.Broadcast(GetHealthPercent());
}

void UCharacterAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(MaxHealth > 0.f, TEXT("UCharacterAttributeComponent::BeginPlay() max health can't be equal to 0"))
	checkf(GetOwner()->IsA<AGCBaseCharacter>(),
	       TEXT(
		       "UCharacterAttributeComponent::BeginPlay() UCharacterAttributeComponent can be used only with AGCBaseCharacter"
	       ))
	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(GetOwner());
	if (GetOwner()->HasAuthority())
	{
		CachedBaseCharacter->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributeComponent::OnTakeAnyDamage);
	}
	Health = MaxHealth;
	BroadCastHealthValue();
	Stamina = MaxStamina;
	BroadCastStaminaValue();
	Oxygen = MaxOxygen;
	BroadCastOxygenValue();
}

void UCharacterAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateStaminaValue(DeltaTime);
	UpdateOxygenValue(DeltaTime);
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DebugDrawAttributes();
#endif
}

bool UCharacterAttributeComponent::IsSprinting()
{
	return CachedBaseCharacter->GetBaseCharacterMovementComponent()->IsSprinting();
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

void UCharacterAttributeComponent::OnRep_Health()
{
	OnHealthChanged();
}

void UCharacterAttributeComponent::OnHealthChanged()
{
	if (Health <= 0.f)
	{
		if (OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();
		}
	}
}

void UCharacterAttributeComponent::DebugDrawAttributes()
{
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryCharacterAttributes))
	{
		return;
	}
	ShowAttribute3D(15.f, FString::Printf(TEXT("Health: %.2f"), Health), FColor::Red);
	ShowAttribute3D(10.f, FString::Printf(TEXT("Stamina: %.2f"), Stamina), FColor::Green);
	ShowAttribute3D(5.f, FString::Printf(TEXT("Oxygen: %.2f"), Oxygen), FColor::Blue);
}
#endif

void UCharacterAttributeComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                                   AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive())
	{
		return;
	}

	UE_LOG(LogDamage, Warning,
	       TEXT("UCharacterAttributeComponent::OnTakeAnyDamage %s received %.2f amount of damage from %s"),
	       *CachedBaseCharacter->GetName(), Damage, *DamageCauser->GetName())

	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	BroadCastHealthValue();
	OnHealthChanged();
}

void UCharacterAttributeComponent::UpdateStaminaValue(float DeltaTime)
{
	IsSprinting() ? ConsumeStamina(DeltaTime) : RestoreStamina(DeltaTime);

	if (FMath::IsNearlyZero(Stamina))
	{
		OutOfStaminaEvent.IsBound() ? OutOfStaminaEvent.Broadcast(true) : void();
	}

	if (FMath::IsNearlyEqual(Stamina, MaxStamina))
	{
		OutOfStaminaEvent.IsBound() ? OutOfStaminaEvent.Broadcast(false) : void();
	}
}

void UCharacterAttributeComponent::ConsumeStamina(float DeltaTime)
{
	Stamina -= SprintStaminaConsumptionVelocity * DeltaTime;
	Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
	BroadCastStaminaValue();
}

void UCharacterAttributeComponent::RestoreStamina(float DeltaTime)
{
	Stamina += StaminaRestoreVelocity * DeltaTime;
	Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
	BroadCastStaminaValue();
}

void UCharacterAttributeComponent::UpdateOxygenValue(float DeltaTime)
{
	CachedBaseCharacter->IsSwimmingUnderWater() ? ConsumeOxygen(DeltaTime) : RestoreOxygen(DeltaTime);
}

void UCharacterAttributeComponent::ConsumeOxygen(float DeltaTime)
{
	Oxygen -= SwimOxygenConsumptionVelocity * DeltaTime;
	Oxygen = FMath::Clamp(Oxygen, 0.f, MaxOxygen);
	BroadCastOxygenValue();
	if (FMath::IsNearlyZero(Oxygen))
	{
		CachedBaseCharacter->TakeDamage(SwimOxygenConsumptionVelocity * DeltaTime, FDamageEvent(),
		                                CachedBaseCharacter->GetController(), CachedBaseCharacter.Get());
	}
}

void UCharacterAttributeComponent::RestoreOxygen(float DeltaTime)
{
	Oxygen += SwimOxygenConsumptionVelocity * DeltaTime;
	Oxygen = FMath::Clamp(Oxygen, 0.f, MaxOxygen);
	BroadCastOxygenValue();
}

void UCharacterAttributeComponent::ShowAttribute3D(float Offset, FString Text, FColor Color)
{
	FVector TextLocation = CachedBaseCharacter->GetActorLocation() +
		(CachedBaseCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + Offset) * FVector::UpVector;
	DrawDebugString(GetWorld(), TextLocation, Text, nullptr, Color, 0.f, true);
}

void UCharacterAttributeComponent::BroadCastStaminaValue()
{
	OnStaminaChanged.Broadcast(Stamina / MaxStamina);
}

void UCharacterAttributeComponent::BroadCastOxygenValue()
{
	OnOxygenChanged.Broadcast(Oxygen / MaxOxygen);
}
