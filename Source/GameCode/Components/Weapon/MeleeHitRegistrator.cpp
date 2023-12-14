// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeHitRegistrator.h"

#include "GameCode/GameCodeTypes.h"
#include "DrawDebugHelpers.h"
#include "GameCode/Subsystems/DebugSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameCode/Utils/GCTraceUtils.h"

UMeleeHitRegistrator::UMeleeHitRegistrator()
{
	PrimaryComponentTick.bCanEverTick = true;
	SphereRadius = 5.f;
	SetCollisionProfileName(CollisionProfileNoCollision);
}

void UMeleeHitRegistrator::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsHitRegistratorActive)
	{
		ProcessHitRegistration();
	}
	PreviousLocation = GetComponentLocation();
}

void UMeleeHitRegistrator::ProcessHitRegistration()
{
	FVector ComponentLocation = GetComponentLocation();
	FHitResult HitResult;

#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubSystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<
		UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubSystem->IsCategoryEnabled(DebugCategoryMeleeWeapon);
#else
	bool bIsDebugEnabled = false;
#endif

	bool bHasHit = GCTraceUtils::SweepSphereSingleByChanel(
		GetWorld(),
		HitResult,
		PreviousLocation,
		ComponentLocation,
		GetScaledSphereRadius(),
		ECC_Melee,
		FCollisionQueryParams::DefaultQueryParam,
		FCollisionResponseParams::DefaultResponseParam,
		bIsDebugEnabled,
		5.f
	);
	if (bHasHit)
	{
		FVector Direction = (ComponentLocation - PreviousLocation).GetSafeNormal();
		OnMeleeHit.Broadcast(HitResult, Direction);
	}
}
