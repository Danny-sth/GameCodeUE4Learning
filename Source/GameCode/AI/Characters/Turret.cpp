// Fill out your copyright notice in the Description page of Project Settings.


#include "Turret.h"

#include "AIController.h"
#include "GameCode/Components/NPC/Turret/HealthComponent.h"
#include "GameCode/Components/Weapon/WeaponBarellComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* TurretRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretRoot"));
	SetRootComponent(TurretRoot);

	TurretBaseComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TurretBaseComponent"));
	TurretBaseComponent->SetupAttachment(TurretRoot);

	TurretBarrelComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TurretBarrelComponent"));
	TurretBarrelComponent->SetupAttachment(TurretBaseComponent);

	WeaponBarell = CreateDefaultSubobject<UWeaponBarellComponent>(TEXT("WeaponBarell"));
	WeaponBarell->SetupAttachment(TurretBarrelComponent);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	SetReplicates(true);
}

void ATurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATurret, CurrentTarget)
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->OnDeathEvent.AddDynamic(this, &ATurret::OnDeath);
}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	switch (CurrentState)
	{
	case ETurretState::Searching:
		{
			SearchingMovement(DeltaTime);
			break;
		}
	case ETurretState::Firing:
		{
			FiringMovement(DeltaTime);
			break;
		}
	}
}

void ATurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AAIController* AIController = Cast<AAIController>(NewController);
	if (AIController)
	{
		FGenericTeamId TeamId((uint8)Team);
		AIController->SetGenericTeamId(TeamId);
	}
}

void ATurret::OnCurrentTargetSet()
{
	SetCurrentTurretState(CurrentTarget ? ETurretState::Firing : ETurretState::Searching);
}

FVector ATurret::GetPawnViewLocation() const
{
	return WeaponBarell->GetComponentLocation();
}

FRotator ATurret::GetViewRotation() const
{
	return WeaponBarell->GetComponentRotation();
}

void ATurret::SetCurrentTurretState(ETurretState NewState)
{
	bool bIsStateChanged = NewState != CurrentState;
	CurrentState = NewState;
	if (!bIsStateChanged)
	{
		return;
	}
	switch (CurrentState)
	{
	case ETurretState::Searching:
		{
			GetWorldTimerManager().ClearTimer(ShotTimerHandle);
			break;
		}
	case ETurretState::Firing:
		{
			GetWorldTimerManager().SetTimer(ShotTimerHandle, this, &ATurret::MakeShot,
			                                GetFireInterval(), true, FireDelayTime);
			break;
		}
	}
}

void ATurret::MakeShot()
{
	WeaponBarell->Shot(WeaponBarell->GetComponentLocation(),
	                   WeaponBarell->GetComponentRotation().RotateVector(FVector::ForwardVector),
	                   FMath::DegreesToRadians(BulletSpreadAngle));
}

void ATurret::SearchingMovement(float DeltaTime)
{
	FRotator CurrentRotation = TurretBaseComponent->GetRelativeRotation();
	CurrentRotation.Yaw += BaseSearchingRotationRate * DeltaTime;
	TurretBaseComponent->SetRelativeRotation(CurrentRotation);

	FRotator TurretBarellRotation = TurretBarrelComponent->GetRelativeRotation();
	TurretBarellRotation.Pitch = FMath::FInterpTo(TurretBarellRotation.Pitch, 0.0f, DeltaTime, BarrelPitchRotationRate);
	TurretBarrelComponent->SetRelativeRotation(TurretBarellRotation);
}

void ATurret::FiringMovement(float DeltaTime)
{
	FQuat LookAtQuat = (CurrentTarget->GetActorLocation() - TurretBaseComponent->GetComponentLocation())
	                   .GetSafeNormal2D().ToOrientationQuat();
	FQuat TargetQuat = FMath::QInterpTo(TurretBaseComponent->GetComponentQuat(),
	                                    LookAtQuat, DeltaTime, BaseFiringInerpSpeed);
	TurretBaseComponent->SetWorldRotation(TargetQuat);

	float BarellLookAtPitchAngle = (CurrentTarget->GetActorLocation() - WeaponBarell->GetComponentLocation())
	                               .GetSafeNormal().ToOrientationRotator().Pitch;
	FRotator BarellRelativeRotation = TurretBarrelComponent->GetRelativeRotation();
	BarellRelativeRotation.Pitch = FMath::FInterpTo(BarellRelativeRotation.Pitch,
	                                                BarellLookAtPitchAngle, DeltaTime, BarrelPitchRotationRate);
	TurretBarrelComponent->SetRelativeRotation(BarellRelativeRotation);
}

float ATurret::GetFireInterval() const
{
	return 60.f / FireRate;
}

void ATurret::OnRep_CurrentTarget()
{
	OnCurrentTargetSet();
}

void ATurret::OnDeath()
{
	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());
	}
	Destroy();
}
