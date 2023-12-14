// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBarellComponent.h"

#include "DrawDebugHelpers.h"
#include "GameCode/GameCodeTypes.h"
#include "GameCode/Subsystems/DebugSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Damage.h"

UWeaponBarellComponent::UWeaponBarellComponent()
{
	SetIsReplicatedByDefault(true);
}

void UWeaponBarellComponent::BeginPlay()
{
	Super::BeginPlay();


	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	if (!ProjectileClass)
	{
		return;
	}

	ProjectilePool.Reserve(ProjectilePoolSize);

	for (int32 i = 0; i < ProjectilePoolSize; ++i)
	{
		AGCProjectile* Projectile = GetWorld()->SpawnActor<AGCProjectile>(ProjectileClass, ProjectilePoolLocation,
		                                                                  FRotator::ZeroRotator);
		Projectile->SetOwner(GetOwningPawn());
		Projectile->SetProjectileActive(false);
		ProjectilePool.Add(Projectile);
	}
}

void UWeaponBarellComponent::Shot(FVector ShotStart, FVector ShotDirection, float SpreadAngle)
{
	TArray<FShotInfo> ShotInfos;
	for (int i = 0; i < BulletsPerShot; i++)
	{
		ShotDirection += GetBulletSpreadOffset(FMath::RandRange(0.f, SpreadAngle),
		                                       ShotDirection.ToOrientationRotator());
		ShotInfos.Emplace(ShotStart, ShotDirection);
	}

	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_Shot(ShotInfos);
	}

	ShotInternal(ShotInfos);
}

void UWeaponBarellComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams RepLifetimeParams;
	RepLifetimeParams.Condition = COND_SimulatedOnly;
	RepLifetimeParams.RepNotifyCondition = REPNOTIFY_Always;
	DOREPLIFETIME_WITH_PARAMS(UWeaponBarellComponent, LastShotsInfo, RepLifetimeParams)
	DOREPLIFETIME(UWeaponBarellComponent, ProjectilePool)
	DOREPLIFETIME(UWeaponBarellComponent, CurrentProjectileIndex)
}

void UWeaponBarellComponent::ShotInternal(const TArray<FShotInfo> Infos)
{
	if (GetOwner()->HasAuthority())
	{
		LastShotsInfo = Infos;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX,
	                                               GetComponentLocation(), GetComponentRotation());
	for (auto Info : Infos)
	{
		FVector ShotStart = Info.GetLocation();
		FVector ShotDirection = Info.GetDirection();
		FVector ShotEnd = ShotStart + FiringRange * ShotDirection;

#if ENABLE_DRAW_DEBUG
		UDebugSubsystem* DebugSubSystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<
			UDebugSubsystem>();
		bool bIsDebugEnabled = DebugSubSystem->IsCategoryEnabled(DebugCategoryRangeWeapon);
#else
		bool bIsDebugEnabled = false;
#endif
		switch (HitRegistrationType)
		{
		case EHitRegistrationType::HitScan:
			{
				bool bHasHit = HitScan(ShotStart, ShotEnd, ShotDirection);
				if (bIsDebugEnabled && bHasHit)
				{
					DrawDebugSphere(GetWorld(), ShotEnd, 10.f, 24, FColor::Red, false, 1.f);
				}
				break;
			}
		case EHitRegistrationType::Projectile:
			{
				LaunchProjectile(ShotStart, ShotDirection);
				break;
			}
		}

		UNiagaraComponent* TraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TraceFX,
			GetComponentLocation(), GetComponentRotation());
		if (TraceFXComponent)
		{
			TraceFXComponent->SetVectorParameter(FXParamTraceEnd, ShotEnd);
		}
		if (bIsDebugEnabled)
		{
			DrawDebugLine(GetWorld(), GetComponentLocation(), ShotEnd, FColor::Red, false, 1.f, 0, 3.f);
		}
	}
}

void UWeaponBarellComponent::Server_Shot_Implementation(const TArray<FShotInfo>& Infos)
{
	ShotInternal(Infos);
}

void UWeaponBarellComponent::OnRep_LastShotsInfo()
{
	ShotInternal(LastShotsInfo);
}

bool UWeaponBarellComponent::HitScan(FVector ShotStart, FVector& ShotEnd,
                                     FVector ShotDirection)
{
	FHitResult ShotResult;
	bool bHasHit = GetWorld()->LineTraceSingleByChannel(ShotResult, ShotStart, ShotEnd, ECC_Bullet);
	if (bHasHit)
	{
		ShotEnd = ShotResult.ImpactPoint;
		ProcessHit(ShotResult, ShotDirection);
	}
	return bHasHit;
}

float UWeaponBarellComponent::CalculateDamageWithFallof(float BaseDamage, float Distance)
{
	return IsValid(FalloffDiagram) ? BaseDamage * FalloffDiagram->GetFloatValue(Distance) : BaseDamage;
}

FVector UWeaponBarellComponent::GetBulletSpreadOffset(float Angle, FRotator ShotRotation)
{
	float SpreadSize = FMath::Tan(Angle);
	float RotationAngle = FMath::RandRange(0.f, 2 * PI);
	float SpreadY = FMath::Cos(RotationAngle);
	float SpreadZ = FMath::Sin(RotationAngle);

	return (ShotRotation.RotateVector(FVector::UpVector) * SpreadZ
		+ ShotRotation.RotateVector(FVector::RightVector) * SpreadY) * SpreadSize;
}

APawn* UWeaponBarellComponent::GetOwningPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner)
	{
		PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}
	return PawnOwner;
}

AController* UWeaponBarellComponent::GetController() const
{
	APawn* OwningPawn = GetOwningPawn();
	return OwningPawn ? OwningPawn->GetController() : nullptr;
}

void UWeaponBarellComponent::LaunchProjectile(const FVector& LaunchStart, const FVector LaunchDirection)
{
	AGCProjectile* Projectile = ProjectilePool[CurrentProjectileIndex];
	Projectile->SetActorLocation(LaunchStart);
	Projectile->SetActorRotation(LaunchDirection.ToOrientationRotator());
	Projectile->SetProjectileActive(true);
	Projectile->SetOwner(GetOwningPawn());
	Projectile->OnProjectileHit.AddDynamic(this, &UWeaponBarellComponent::ProcessProjectileHit);
	Projectile->LaunchProjectile(LaunchDirection.GetSafeNormal());
	++CurrentProjectileIndex;
	if (CurrentProjectileIndex == ProjectilePool.Num())
	{
		CurrentProjectileIndex = 0;
	}
}

void UWeaponBarellComponent::ProcessHit(const FHitResult& Hit, const FVector& ShotDirection)
{
	AActor* HittedActor = Hit.GetActor();
	if (GetOwner()->HasAuthority() && IsValid(HittedActor))
	{
		DamageAmount = CalculateDamageWithFallof(DamageAmount,
		                                         FVector::Distance(GetComponentLocation(),
		                                                           HittedActor->GetTargetLocation()));
		FPointDamageEvent DamageEvent;
		DamageEvent.HitInfo = Hit;
		DamageEvent.ShotDirection = ShotDirection;
		DamageEvent.DamageTypeClass = DamageTypeClass;
		HittedActor->TakeDamage(DamageAmount, DamageEvent, GetController(), GetOwner());
		UAISense_Damage::ReportDamageEvent(GetWorld(), HittedActor, GetOwningPawn(), DamageAmount,
		                                   GetOwner()->GetActorLocation(), Hit.ImpactPoint);
	}
	UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(), DefaultDecalInfo.DecalMaterial,
		DefaultDecalInfo.DecalSize, Hit.ImpactPoint, Hit.ImpactNormal.ToOrientationRotator());
	if (DecalComponent)
	{
		DecalComponent->SetFadeOut(DefaultDecalInfo.DecalLifeTime, DefaultDecalInfo.DecalFadeOutTime);
		DecalComponent->SetFadeScreenSize(0.0001f);
	}
}

void UWeaponBarellComponent::ProcessProjectileHit(AGCProjectile* Projectile, const FHitResult& Hit,
                                                  const FVector& ShotDirection)
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ProjectilePoolLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);
	Projectile->OnProjectileHit.RemoveAll(this);
	ProcessHit(Hit, ShotDirection);
}
