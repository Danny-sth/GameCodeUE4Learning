// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformTrigger.h"
#include "Components/BoxComponent.h"
#include "GameCode/GameCodeTypes.h"
#include "Net/UnrealNetwork.h"

APlatformTrigger::APlatformTrigger()
{
	bReplicates = true;
	NetUpdateFrequency = 2.f;
	MinNetUpdateFrequency = 2.f;
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	SetRootComponent(TriggerBox);
}

void APlatformTrigger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME(APlatformTrigger, bIsActivated)
}

void APlatformTrigger::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APlatformTrigger::OnTriggerOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &APlatformTrigger::OnTriggerOverlapEnd);
}

void APlatformTrigger::OnIsActivated(bool bIsActivated_In)
{
	if (OnTriggerActivated.IsBound())
	{
		OnTriggerActivated.Broadcast(bIsActivated_In);
	}
}

void APlatformTrigger::Multicast_OnIsActivated_Implementation(bool bIsActivated_In)
{
	bIsActivated = bIsActivated_In;
	if (OnTriggerActivated.IsBound())
	{
		OnTriggerActivated.Broadcast(bIsActivated_In);
	}
}

void APlatformTrigger::OnRep_IsActivated(bool bIsActivated_Old)
{
	OnIsActivated(bIsActivated);
}

void APlatformTrigger::OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                             const FHitResult& SweepResult)
{
	APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!IsValid(OtherPawn))
	{
		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		OverlappedPawns.AddUnique(OtherPawn);

		if (!bIsActivated && OverlappedPawns.Num() > 0)
		{
			Multicast_OnIsActivated(true);
		}
	}
}

void APlatformTrigger::OnTriggerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!IsValid(OtherPawn))
	{
		return;
	}
	if (GetLocalRole() == ROLE_Authority)
	{
		OverlappedPawns.RemoveSingleSwap(OtherPawn);

		if (bIsActivated && OverlappedPawns.Num() == 0)
		{
			Multicast_OnIsActivated(false);
		}
	}
}
