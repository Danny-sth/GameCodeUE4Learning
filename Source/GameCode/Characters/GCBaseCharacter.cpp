// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacter.h"

#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Curves/CurveVector.h"
#include "GameCode/Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "GameCode/Actors/Interactive/Environment/Ladder.h"
#include "GameCode/Components/LedgeDetectorComponent.h"
#include "GameCode/Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameCode/Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Net/UnrealNetwork.h"

AGCBaseCharacter::AGCBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGCBaseCharacterMovementComponent>(
		CharacterMovementComponentName))
{
	GCBaseCharacterMovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(GetCharacterMovement());
	LedgeDetectorComponent = CreateDefaultSubobject<ULedgeDetectorComponent>(TEXT("LedgeDetector"));
	GetMesh()->CastShadow = true;
	GetMesh()->bCastDynamicShadow = true;

	CharacterAttributeComponent = CreateDefaultSubobject<UCharacterAttributeComponent>(TEXT("Attribute Component"));
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("Equipment component"));
}

void AGCBaseCharacter::StartWallRun()
{
	GetBaseCharacterMovementComponent()->StartWallRun();
}

void AGCBaseCharacter::ChangeCrouchState()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AGCBaseCharacter::StartSprint()
{
	bIsSprintRequested = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void AGCBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

bool AGCBaseCharacter::CanFire()
{
	if (CharacterEquipmentComponent->IsEquipping())
	{
		return false;
	}
	return true;
}

void AGCBaseCharacter::StartFire()
{
	if (IsValid(CharacterEquipmentComponent))
	{
		if (!CanFire()) return;
		ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
		if (CurrentRangeWeapon)
		{
			CurrentRangeWeapon->StartFire();
		}
	}
}

void AGCBaseCharacter::StopFire()
{
	if (IsValid(CharacterEquipmentComponent))
	{
		ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
		if (CurrentRangeWeapon)
		{
			CurrentRangeWeapon->StopFire();
		}
	}
}

void AGCBaseCharacter::StartAiming()
{
	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (!CurrentRangeWeapon)
	{
		return;
	}
	bIsAiming = true;
	CurrentAimingMovementSpeed = CurrentRangeWeapon->GetAimMovementMaxSpeed();
	CurrentRangeWeapon->StartAim();
	OnStartAiming();
}

void AGCBaseCharacter::StopAiming()
{
	if (!bIsAiming)
	{
		return;
	}
	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (CurrentRangeWeapon)
	{
		CurrentRangeWeapon->StopAim();
	}
	bIsAiming = false;
	CurrentAimingMovementSpeed = 0.f;
	OnStopAiming();
}

void AGCBaseCharacter::Reload()
{
	if (CharacterEquipmentComponent->GetCurrentRangeWeapon())
	{
		CharacterEquipmentComponent->ReloadCurrentWeapon();
	}
}

float AGCBaseCharacter::GetAimingMovementSpeed() const
{
	return CurrentAimingMovementSpeed;
}

void AGCBaseCharacter::NextItem()
{
	CharacterEquipmentComponent->EquipNextItem();
}

void AGCBaseCharacter::PreviousItem()
{
	CharacterEquipmentComponent->EquipPreviousItem();
}

void AGCBaseCharacter::ThrowPrimaryItem()
{
	CharacterEquipmentComponent->EquipItemInSlot(EEquipmentSlots::PrimaryItemSlot);
}

void AGCBaseCharacter::PrimaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if (CurrentMeleeWeapon)
	{
		CurrentMeleeWeapon->StartAttack(EMeleeAttackType::Primary);
	}
}

void AGCBaseCharacter::SecondaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if (CurrentMeleeWeapon)
	{
		CurrentMeleeWeapon->StartAttack(EMeleeAttackType::Secondary);
	}
}

FRotator AGCBaseCharacter::GetAimOffset()
{
	FVector AimDirectionWorld = GetBaseAimRotation().Vector();
	FVector AimDirectionLocal = GetTransform().InverseTransformVectorNoScale(AimDirectionWorld);
	return AimDirectionLocal.ToOrientationRotator();
}

void AGCBaseCharacter::OnStartAiming_Implementation()
{
	OnStartAimingInternal();
}

void AGCBaseCharacter::OnStopAiming_Implementation()
{
	OnStopAimingInternal();
}

void AGCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	CharacterAttributeComponent->OnDeathEvent.AddUObject(this, &AGCBaseCharacter::OnDeath);
	CharacterAttributeComponent->OutOfStaminaEvent.AddUObject(this, &AGCBaseCharacter::SetOutOfStamina);
}

void AGCBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TryChangeSprintState(DeltaTime);
	UpdateIKSettings(DeltaTime);
}

void AGCBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AAIController* AIController = Cast<AAIController>(NewController);
	if (AIController)
	{
		FGenericTeamId TeamId((uint8)Team);
		AIController->SetGenericTeamId(TeamId);
	}
}

void AGCBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGCBaseCharacter, bIsMantling)
}

void AGCBaseCharacter::Mantle(bool bForce /*= false*/)
{
	if (!(CanMantle() || bForce))
	{
		return;
	}
	if (bIsCrouched)
	{
		UnCrouch();
	}

	bIsMantling = true;

	FLedgeDescription LedgeDescription;
	if (LedgeDetectorComponent->DetectLedge(LedgeDescription))
	{
		FMantlingMovementParameters MantlingParameters;
		MantlingParameters.InitialLocation = GetActorLocation();
		MantlingParameters.InitialRotation = GetActorRotation();
		MantlingParameters.TargetLocation = LedgeDescription.Location;
		MantlingParameters.TargetRotation = LedgeDescription.Rotation;
		MantlingParameters.TargetComponent = LedgeDescription.TargetActor;

		float MantlingHeight = LedgeDescription.MantlingHeight;
		FMantlingSettings& MantlingSettings = GetMantlingSettings(MantlingHeight);

		float MinRange;
		float MaxRange;
		MantlingSettings.MantligCurve->GetTimeRange(MinRange, MaxRange);

		MantlingParameters.Duration = MaxRange - MinRange;
		MantlingParameters.MantlingCurve = MantlingSettings.MantligCurve;

		FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
		FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);
		MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);

		MantlingParameters.InitialAnimationLocation = MantlingParameters.TargetLocation - MantlingSettings.
			AnimationCorrectionZ * FVector::UpVector + MantlingSettings.AnimationCorrectionXY * LedgeDescription.
			LedgeNormal;

		if (IsLocallyControlled() || GetLocalRole() == ROLE_Authority)
		{
			GetBaseCharacterMovementComponent()->StartMantle(MantlingParameters);
		}

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(MantlingSettings.MantlingMontage, 1.0f, EMontagePlayReturnType::Duration,
		                           MantlingParameters.StartTime);
		OnMantle(MantlingSettings, MantlingParameters.StartTime);
	}
}

void AGCBaseCharacter::Zipline()
{
	if (!CanZipline())
	{
		return;
	}
	const AZipline* Zipline = GetAvailableZipline();
	if (IsValid(Zipline))
	{
		if (Zipline->IsOnZipline())
		{
			const float DistanceToTarget = (Zipline->GetTargetPillarLocation() - GetActorLocation()).Size();
			const float RemainingTime = DistanceToTarget / GetBaseCharacterMovementComponent()->GetZiplineSpeed();
			GetBaseCharacterMovementComponent()->SetZipLineDuration(RemainingTime);
			GetBaseCharacterMovementComponent()->StartZipline();
		}
	}
}

void AGCBaseCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.AddUnique(InteractiveActor);
}

void AGCBaseCharacter::UnregisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.RemoveSingleSwap(InteractiveActor);
}

void AGCBaseCharacter::ClimbLadderUp(float Value)
{
	if (GetBaseCharacterMovementComponent()->IsOnLadder() && !FMath::IsNearlyZero(Value))
	{
		FVector LadderUpVector = GetBaseCharacterMovementComponent()->GetCurrentLadder()->GetActorUpVector();
		AddMovementInput(LadderUpVector, Value);
	}
}

void AGCBaseCharacter::InteractWithLadder()
{
	if (GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		GetBaseCharacterMovementComponent()->DetachFromLadder(EDetachFromLadderMethod::JumpOff);
	}
	else
	{
		const ALadder* AvailableLadder = GetAvailableLadder();
		if (IsValid(AvailableLadder))
		{
			if (AvailableLadder->GetIsOnTop())
			{
				PlayAnimMontage(AvailableLadder->GetAttachFromTopAnimMontage());
			}
			GetBaseCharacterMovementComponent()->AttachToLadder(AvailableLadder);
		}
	}
}

const ALadder* AGCBaseCharacter::GetAvailableLadder() const
{
	const ALadder* Result = nullptr;
	for (const AInteractiveActor* InteractiveActor : AvailableInteractiveActors)
	{
		if (InteractiveActor->IsA<ALadder>())
		{
			Result = StaticCast<const ALadder*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

AZipline* AGCBaseCharacter::GetAvailableZipline()
{
	AZipline* Result = nullptr;
	for (AInteractiveActor* InteractiveActor : AvailableInteractiveActors)
	{
		if (InteractiveActor->IsA<AZipline>())
		{
			Result = StaticCast<AZipline*>(InteractiveActor);
		}
	}
	return Result;
}

bool AGCBaseCharacter::IsSwimmingUnderWater() const
{
	if (GetBaseCharacterMovementComponent()->IsSwimming())
	{
		APhysicsVolume* Volume = GetBaseCharacterMovementComponent()->GetPhysicsVolume();
		float VolumeTopPlane = Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z * Volume->
			GetActorScale3D().Z;
		FVector HeadLocation = GetMesh()->GetSocketLocation(FName("head"));
		if (HeadLocation.Z < VolumeTopPlane)
		{
			return true;
		}
	}
	return false;
}

void AGCBaseCharacter::OnRep_IsMantling(bool bIsMantling_Old)
{
	if (GetLocalRole() == ROLE_SimulatedProxy && !bIsMantling_Old && bIsMantling)
	{
		Mantle(true);
	}
}

bool AGCBaseCharacter::CanJumpInternal_Implementation() const
{
	return bIsCrouched ||
		!GetBaseCharacterMovementComponent()->IsMantling() ||
		!GetBaseCharacterMovementComponent()->IsOnLadder() ||
		!GetBaseCharacterMovementComponent()->IsOnZipline() ||
		Super::CanJumpInternal_Implementation();
}

void AGCBaseCharacter::UpdateIKSettings(float DeltaSeconds)
{
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, CalculateIKParametersForSocketName(RightFootSocketName),
	                                     DeltaSeconds, IKInterpSpeed);
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, CalculateIKParametersForSocketName(LeftFootSocketName),
	                                    DeltaSeconds, IKInterpSpeed);
	IKPelvisOffset = FMath::FInterpTo(IKPelvisOffset, CalculateIKPelvisOffset(), DeltaSeconds, IKInterpSpeed);
}

float AGCBaseCharacter::CalculateIKParametersForSocketName(const FName& SocketName) const
{
	float Result = 0.0f;

	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	const FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	const FVector TraceEnd = TraceStart - (CapsuleHalfHeight + IKTraceDistance) * FVector::UpVector;

	FHitResult HitResult;
	const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);


	const FVector FootSizeBox = FVector(1.f, 15.f, 7.f);
	if (UKismetSystemLibrary::BoxTraceSingle(GetWorld(), TraceStart, TraceEnd, FootSizeBox,
	                                         GetMesh()->GetSocketRotation(SocketName), TraceType, true,
	                                         TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true))
	{
		float CharacterBottom = TraceStart.Z - CapsuleHalfHeight;
		Result = CharacterBottom - HitResult.Location.Z;
	}

	return Result;
}

float AGCBaseCharacter::CalculateIKPelvisOffset()
{
	return IKRightFootOffset > IKLeftFootOffset ? -IKRightFootOffset : -IKLeftFootOffset;
}

void AGCBaseCharacter::OnMontageTimerElapsed()
{
	Controller->SetIgnoreMoveInput(false);
}

void AGCBaseCharacter::OnSprintStart_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("AGCBaseCharacter::OnSprintStart_Implementation"));
}

void AGCBaseCharacter::OnSprintEnd_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AGCBaseCharacter::OnSprintEnd_Implementation"));
}

bool AGCBaseCharacter::CanSprint()
{
	return !(GCBaseCharacterMovementComponent->IsOutOfStamina() || GCBaseCharacterMovementComponent->IsCrouching());
}

bool AGCBaseCharacter::CanMantle() const
{
	return !GetBaseCharacterMovementComponent()->IsMantling() && !GetBaseCharacterMovementComponent()->IsOnLadder()
		&& !GetBaseCharacterMovementComponent()->IsOnZipline();
}

bool AGCBaseCharacter::CanZipline()
{
	return !GetBaseCharacterMovementComponent()->IsMantling() &&
		!GetBaseCharacterMovementComponent()->IsOnLadder() &&
		!GetBaseCharacterMovementComponent()->IsOnZipline();
}

void AGCBaseCharacter::OnMantle(FMantlingSettings& MantlingSettings, float MantlingAnimaStartTime)
{
}

void AGCBaseCharacter::Falling()
{
	Super::Falling();
	GetBaseCharacterMovementComponent()->bNotifyApex = true;
}

void AGCBaseCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();
	DropAltitude = GetActorLocation();
}

void AGCBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (GetActorLocation().Z <= DropAltitude.Z)
	{
		float FallHeight = (DropAltitude - GetActorLocation()).Z * 0.01f;
		if (IsValid(FallDamageCurve) && FallHeight >= MaxSafeAltitude)
		{
			Controller->SetIgnoreMoveInput(true);
			float Duration = GetMesh()->GetAnimInstance()->Montage_Play(HardLandingMontage, 1.f,
			                                                            EMontagePlayReturnType::Duration, 0, false);
			GetWorld()->GetTimerManager().SetTimer(MontageTimer, this,
			                                       &AGCBaseCharacter::OnMontageTimerElapsed, Duration, false);
			TakeDamage(FallDamageCurve->GetFloatValue(FallHeight),
			           FDamageEvent(), GetController(), Hit.Actor.Get());
		}
	}
}

void AGCBaseCharacter::OnDeath()
{
	GetCharacterMovement()->DisableMovement();
	float Duration = PlayAnimMontage(OnDeathAnimMontage);
	if (Duration == 0.f)
	{
		EnableRagdoll();
	}
}

void AGCBaseCharacter::OnStartAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(true);
	}
}

void AGCBaseCharacter::OnStopAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(false);
	}
}

void AGCBaseCharacter::TryChangeSprintState(float DeltaTime)
{
	if (bIsSprintRequested && !GCBaseCharacterMovementComponent->IsSprinting() && CanSprint())
	{
		GCBaseCharacterMovementComponent->StartSprint();
		OnSprintStart();
	}
	if (GCBaseCharacterMovementComponent->IsSprinting() && !(bIsSprintRequested && CanSprint()))
	{
		GCBaseCharacterMovementComponent->StopSprint();
		OnSprintEnd();
	}
}

FMantlingSettings& AGCBaseCharacter::GetMantlingSettings(float LedgeHeight)
{
	return LedgeHeight > LowMantleMaxHeight ? HighMantleSettings : LowMantleSettings;
}

void AGCBaseCharacter::EnableRagdoll()
{
	GetMesh()->SetCollisionProfileName(CollisionProfileRagdoll);
	GetMesh()->SetSimulatePhysics(true);
}
