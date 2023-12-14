// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableItem.h"

#include "GameCode/Actors/Projectles/GCProjectile.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Net/UnrealNetwork.h"

AThrowableItem::AThrowableItem()
{
	SetReplicates(true);
}

void AThrowableItem::BeginPlay()
{
	Super::BeginPlay();
	SetThrowableAmmoAmount(MaxAmmoAmount);

	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}
	ThrowableItemsPool.Reserve(ThrowableItemsPoolSize);
	for (int32 i = 0; i < ThrowableItemsPoolSize; ++i)
	{
		AGCProjectile* Projectile = GetWorld()->SpawnActor<AGCProjectile>(ProjectileClass, ThrowableItemsPoolLocation,
		                                                                  FRotator::ZeroRotator);
		Projectile->SetProjectileActive(false);
		ThrowableItemsPool.Add(Projectile);
	}
}

void AThrowableItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AThrowableItem, ThrowableItemsPool)
	DOREPLIFETIME(AThrowableItem, CurrentThrowableIndex)
}

void AThrowableItem::OnExplode(AGCProjectile* Projectile)
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ThrowableItemsPoolLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);
	Projectile->OnThrowableItemExplode.RemoveAll(this);
}

void AThrowableItem::Throw()
{
	AGCBaseCharacter* CharacterOwner = GetCachedCharacterOwner();
	if (!CharacterOwner || !CanThrow())
	{
		return;
	}

	FVector ThrowLocation;
	FRotator ThrowRotation;
	FVector ThrowableSocketLocation = CharacterOwner->GetMesh()->GetSocketLocation(SocketCharacterThrowable);
	if (CharacterOwner->IsPlayerControlled() && GetLocalRole() == ROLE_Authority)
	{
		APlayerController* PlayerController = CharacterOwner->GetController<APlayerController>();
		PlayerController->GetPlayerViewPoint(ThrowLocation, ThrowRotation);
	}
	else
	{
		ThrowLocation = ThrowableSocketLocation;
		ThrowRotation = CharacterOwner->GetBaseAimRotation();
	}

	FTransform PlayerViewTransform(ThrowRotation, ThrowLocation);
	FVector ViewDirection = ThrowRotation.RotateVector(FVector::ForwardVector);
	FVector ViewUpVector = ThrowRotation.RotateVector(FVector::UpVector);
	FVector LaunchDirection = ViewDirection + FMath::Tan(FMath::DegreesToRadians(ThrowAngle)) * ViewUpVector;
	FVector SocketInViewSpace = PlayerViewTransform.InverseTransformPosition(ThrowableSocketLocation);
	FVector SpawnAtLocation = ThrowLocation + ViewDirection * SocketInViewSpace.X;

	AGCProjectile* Projectile = ThrowableItemsPool[CurrentThrowableIndex];
	if (Projectile)
	{
		Projectile->SetActorLocation(SpawnAtLocation);
		Projectile->SetActorRotation(LaunchDirection.ToOrientationRotator());
		Projectile->SetProjectileActive(true);
		Projectile->SetOwner(GetOwner());
		Projectile->OnThrowableItemExplode.AddDynamic(this, &AThrowableItem::OnExplode);
		Projectile->LaunchProjectile(LaunchDirection.GetSafeNormal());
		++CurrentThrowableIndex;
		if (CurrentThrowableIndex == ThrowableItemsPool.Num())
		{
			CurrentThrowableIndex = 0;
		}
		SetThrowableAmmoAmount(AmmoAmount - 1);
	}
}

void AThrowableItem::SetThrowableAmmoAmount(const int32 NewAmmoAmount)
{
	AmmoAmount = NewAmmoAmount;
	OnThrowableAmmoChanged.Broadcast(NewAmmoAmount);
}

bool AThrowableItem::CanThrow() const
{
	return AmmoAmount > 0;
}
