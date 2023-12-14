// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosiveProjectile.h"

#include "Net/UnrealNetwork.h"

AExplosiveProjectile::AExplosiveProjectile()
{
	ExplosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExplosionComponent"));
	ExplosionComponent->SetupAttachment(GetRootComponent());
}

void AExplosiveProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME(UExplosionComponent, Exp)
}

void AExplosiveProjectile::OnProjectileLaunched()
{
	Super::OnProjectileLaunched();
	GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &AExplosiveProjectile::OnExplosionTimerElapsed,
	                                ExplosionDelay, false);
}

AController* AExplosiveProjectile::GetController()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return OwnerPawn ? OwnerPawn->GetController() : nullptr;
}

void AExplosiveProjectile::OnExplosionTimerElapsed()
{
	ExplosionComponent->Explode(GetController());
	OnThrowableItemExplode.Broadcast(this);
}
