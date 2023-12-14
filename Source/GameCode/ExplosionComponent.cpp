// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosionComponent.h"

#include "Kismet/GameplayStatics.h"

void UExplosionComponent::Explode(AController* InstigatorController)
{
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(GetOwner());
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(),
		MaxDamage,
		MinDamage,
		GetComponentLocation(),
		InnerRadius,
		OuterRadius,
		DamageFalloff,
		DamageTypeClass,
		IgnoredActors,
		GetOwner(),
		InstigatorController,
		ECC_Visibility);

	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetComponentLocation());
	}
	OnExplosion.Broadcast();
}
