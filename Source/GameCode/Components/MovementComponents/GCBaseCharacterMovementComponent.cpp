// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "GameCode/Actors/Interactive/Environment/Ladder.h"
#include "GameCode/Characters/GCBaseCharacter.h"

void UGCBaseCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));


		// Accumulate a desired new rotation.
		const float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForceTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForceTargetRotation;
			// PITCH
			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			// YAW
			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			// ROLL
			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			// Set the new rotation.
			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, /*bSweep*/ false);
		}
		else
		{
			ForceTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
		}
		return;
	}

	if (IsOnLadder())
	{
		return;
	}
	Super::PhysicsRotation(DeltaTime);
}

float UGCBaseCharacterMovementComponent::GetMaxSpeed() const
{
	if (bIsSprinting)
	{
		return SprintSpeed;
	}
	if (bIsOutOfStamina)
	{
		return OutOfStaminaSpeed;
	}
	if (IsOnLadder())
	{
		return ClimbingOnLadderMaxSpeed;
	}
	if (GetBaseCharacterOwner()->IsAiming())
	{
		return GetBaseCharacterOwner()->GetAimingMovementSpeed();
	}
	return Super::GetMaxSpeed();
}

void UGCBaseCharacterMovementComponent::StartSprint()
{
	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UGCBaseCharacterMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

void UGCBaseCharacterMovementComponent::StartZipline()
{
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Zipline);
}

void UGCBaseCharacterMovementComponent::EndZipline()
{
	SetMovementMode(MOVE_Walking);
}

bool UGCBaseCharacterMovementComponent::IsOnZipline() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom &&
		CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Zipline;
}

void UGCBaseCharacterMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	CurrentMantlingParameters = MantlingParameters;
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Mantling);
}

void UGCBaseCharacterMovementComponent::EndMantle()
{
	GetBaseCharacterOwner()->bIsMantling = false;
	SetMovementMode(MOVE_Walking);
}

bool UGCBaseCharacterMovementComponent::IsMantling() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)
		ECustomMovementMode::CMOVE_Mantling;
}

void UGCBaseCharacterMovementComponent::AttachToLadder(const ALadder* Ladder)
{
	CurrentLadder = Ladder;

	FRotator TargetOrientationRotation = CurrentLadder->GetActorForwardVector().ToOrientationRotator();
	TargetOrientationRotation.Yaw += 180.0f;

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderForwardVector = CurrentLadder->GetActorForwardVector();
	float Projection = GetActorToCurrentLadderProjection(GetActorLocation());

	FVector NewCharacterLocation = CurrentLadder->GetActorLocation() + Projection * LadderUpVector +
		LadderToCharacterOffset * LadderForwardVector;
	if (CurrentLadder->GetIsOnTop())
	{
		NewCharacterLocation = CurrentLadder->GetAttachFromTopAnimMontageStartingLocation();
	}

	GetOwner()->SetActorLocation(NewCharacterLocation);
	GetOwner()->SetActorRotation(TargetOrientationRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Ladder);
}

float UGCBaseCharacterMovementComponent::GetActorToCurrentLadderProjection(const FVector& Location) const
{
	checkf(IsValid(CurrentLadder),
	       TEXT(
		       "UGCBaseCharacterMovementComponent::GetCharacterToCurrentLadderProjection() cannot be invoked when current ladder is null"
	       ));

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderToCharacterDistance = Location - CurrentLadder->GetActorLocation();
	return FVector::DotProduct(LadderUpVector, LadderToCharacterDistance);
}

float UGCBaseCharacterMovementComponent::GetLadderSpeedRatio() const
{
	checkf(IsValid(CurrentLadder),
	       TEXT("UGCBaseCharacterMovementComponent::GetLadderSpeedRatio() cannot be invoked when current ladder is null"
	       ));

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	return FVector::DotProduct(LadderUpVector, Velocity) / ClimbingOnLadderMaxSpeed;
}

void UGCBaseCharacterMovementComponent::DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod)
{
	switch (DetachFromLadderMethod)
	{
	case EDetachFromLadderMethod::JumpOff:
		{
			FVector JumpDirection = CurrentLadder->GetActorForwardVector();
			SetMovementMode(MOVE_Falling);

			FVector JumpVelocity = JumpDirection * JumpOffFromLadderSpeed;

			ForceTargetRotation = JumpDirection.ToOrientationRotator();
			bForceRotation = true;

			Launch(JumpVelocity);
			break;
		}
	case EDetachFromLadderMethod::ReachingTheTop:
		{
			GetBaseCharacterOwner()->Mantle(true);
			break;
		}
	case EDetachFromLadderMethod::ReachingTheBottom:
		{
			SetMovementMode(MOVE_Walking);
			break;
		}
	case EDetachFromLadderMethod::Fall:
	default:
		{
			SetMovementMode(MOVE_Falling);
			break;
		}
	}
}

bool UGCBaseCharacterMovementComponent::IsOnLadder() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)
		ECustomMovementMode::CMOVE_Ladder;
}

const ALadder* UGCBaseCharacterMovementComponent::GetCurrentLadder()
{
	return CurrentLadder;
}

bool UGCBaseCharacterMovementComponent::DetectWall()
{
	FVector LeftTrace = UpdatedComponent->GetForwardVector() - UpdatedComponent->GetRightVector();
	FVector RightTrace = UpdatedComponent->GetForwardVector() + UpdatedComponent->GetRightVector();

	return CheckWallOnTheSide(LeftTrace, EWallRunSide::Left) || CheckWallOnTheSide(RightTrace, EWallRunSide::Right);
}

bool UGCBaseCharacterMovementComponent::CheckWallOnTheSide(FVector TraceVector, EWallRunSide Side)
{
	FVector StartLocation = UpdatedComponent->GetComponentLocation();
	FVector EndLocation = StartLocation + TraceVector * TraceDistance;
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation,
	                                         ECC_WallRunnable, CollisionQueryParams))
	{
		WallHitResult = HitResult;
		if (Side == EWallRunSide::Left)
		{
			DetectedSide = EWallRunSide::Left;
			CurrentWallRunDirection = FVector::CrossProduct(HitResult.Normal, FVector::UpVector).GetSafeNormal();
		}
		else if (Side == EWallRunSide::Right)
		{
			DetectedSide = EWallRunSide::Right;
			CurrentWallRunDirection = FVector::CrossProduct(FVector::UpVector, HitResult.Normal).GetSafeNormal();
		}
		return true;
	}
	return false;
}

void UGCBaseCharacterMovementComponent::StartWallRun()
{
	if (IsFalling() && IsCanWallRunning())
	{
		bIsWallRunning = true;
		SetMovementMode(MOVE_Custom, uint8(ECustomMovementMode::CMOVE_WallRun));
	}
}

void UGCBaseCharacterMovementComponent::EndWallRun()
{
	bIsWallRunning = false;
	SetMovementMode(MOVE_Falling);
}

void UGCBaseCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bool bIsMantling_Old = GetBaseCharacterOwner()->bIsMantling;
	bIsSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bool bIsMantling = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;

	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_Authority)
	{
		if (!bIsMantling_Old && bIsMantling)
		{
			GetBaseCharacterOwner()->Mantle(true);
		}
	}

	// FLAG_JumpPressed	= 0x01,	// Jump pressed
	// 	FLAG_WantsToCrouch	= 0x02,	// Wants to crouch
	// 	FLAG_Reserved_1		= 0x04,	// Reserved for future use
	// 	FLAG_Reserved_2		= 0x08,	// Reserved for future use
	// 	// Remaining bit masks are available for custom flags.
	// 	FLAG_Custom_0		= 0x10, todo  sprinting flag
	// 	FLAG_Custom_1		= 0x20, todo mantling flag
	// 	FLAG_Custom_2		= 0x40,
	// 	FLAG_Custom_3		= 0x80,
}

FNetworkPredictionData_Client* UGCBaseCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UGCBaseCharacterMovementComponent* MutableThis = const_cast<UGCBaseCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_GC(*this);
	}
	return ClientPredictionData;
	// Super::GetPredictionData_Client()
}

void UGCBaseCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	UE_LOG(LogTemp, Warning, TEXT("UGCBaseCharacterMovementComponent::PhysCustom"))

	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}

	switch (CustomMovementMode)
	{
	case (uint8)ECustomMovementMode::CMOVE_Mantling:
		{
			PhysMantling(DeltaTime, Iterations);
			break;
		}
	case (uint8)ECustomMovementMode::CMOVE_Ladder:
		{
			PhysLadder(DeltaTime, Iterations);
			break;
		}
	case (uint8)ECustomMovementMode::CMOVE_Zipline:
		{
			PhysZipline(DeltaTime, Iterations);
			break;
		}
	case (uint8)ECustomMovementMode::CMOVE_WallRun:
		{
			PhysWallRun(DeltaTime, Iterations);
			break;
		}
	default:
		break;
	}
	Super::PhysCustom(DeltaTime, Iterations);
}

void UGCBaseCharacterMovementComponent::PhysMantling(float DeltaTime, int32 Iterations)
{
	float ElapsedTime =
		GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimer) + CurrentMantlingParameters.StartTime;

	FVector MantlingCurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);

	float PositionAlpha = MantlingCurveValue.X;
	float XYCorrectionAlpha = MantlingCurveValue.Y;
	float ZCorrectionAlpha = MantlingCurveValue.Z;

	FVector CorrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialLocation,
	                                               CurrentMantlingParameters.InitialAnimationLocation,
	                                               XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialLocation.Z,
	                                         CurrentMantlingParameters.InitialAnimationLocation.Z, ZCorrectionAlpha);

	FVector NewLocation = FMath::Lerp(CorrectedInitialLocation, CurrentMantlingParameters.TargetLocation,
	                                  PositionAlpha);
	FRotator NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation,
	                                   CurrentMantlingParameters.TargetRotation, PositionAlpha);

	FVector Delta = NewLocation - GetActorLocation();
	Velocity = Delta / DeltaTime;

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, NewRotation, false, Hit);
}

void UGCBaseCharacterMovementComponent::PhysLadder(float DeltaTime, int32 Iterations)
{
	CalcVelocity(DeltaTime, 1.0f, false, ClimbingOnLadderBrakingDecelartion);
	FVector Delta = Velocity * DeltaTime;

	if (HasAnimRootMotion())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
		return;
	}
	FVector NewPos = GetActorLocation() + Delta;
	float NewPosProjection = GetActorToCurrentLadderProjection(NewPos);

	if (NewPosProjection < MinLadderBottomOffset)
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheBottom);
		return;
	}
	else if (NewPosProjection > (CurrentLadder->GetLadderHeight() - MaxLadderTopOffset))
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheTop);
		return;
	}
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

void UGCBaseCharacterMovementComponent::PhysZipline(float DeltaTime, int32 Iterations)
{
	float ProgressRatio = GetWorld()->GetTimerManager().GetTimerElapsed(ZiplineTimer) / ZiplineDuration;
	FVector TargetPillarLocation = GetBaseCharacterOwner()->GetAvailableZipline()->GetTargetPillarLocation();
	FVector NewLocation = FMath::Lerp(GetActorLocation(), TargetPillarLocation, ProgressRatio);

	FVector Delta = (NewLocation - GetActorLocation()).GetSafeNormal() * ZiplineSpeed * DeltaTime;
	FHitResult HitResult;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, HitResult);
}

void UGCBaseCharacterMovementComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
	if (!AreRequiredKeysDown(DetectedSide))
	{
		EndWallRun();
		return;
	}
	FVector DesiredPosition = UpdatedComponent->GetComponentLocation() + CurrentWallRunDirection;
	float ProgressRatio = GetWorld()->GetTimerManager().GetTimerElapsed(WallRunTimer) / MaxWallRunTime;
	FVector NewLocation = FMath::Lerp(UpdatedComponent->GetComponentLocation(), DesiredPosition, ProgressRatio);
	FRotator NewRotation = (DesiredPosition - UpdatedComponent->GetComponentLocation()).ToOrientationRotator();
	const FVector Delta = (NewLocation - UpdatedComponent->GetComponentLocation())
		.GetSafeNormal() * MaxWallRunSpeed * DeltaTime;
	FHitResult HitResult;
	if (WallHitResult.IsValidBlockingHit())
	{
		SafeMoveUpdatedComponent(Delta,
		                         NewRotation, true, HitResult);
	}
}

void UGCBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode,
                                                              uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	PreviousMovementMode != MOVE_Walking ? bCanWallRunning = false : bCanWallRunning = true;
	if (MovementMode == MOVE_Swimming)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
	}
	else if (PreviousMovementMode == MOVE_Swimming)
	{
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(
			DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
			DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		CurrentLadder = nullptr;
	}

	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
		case (uint8)ECustomMovementMode::CMOVE_Mantling:
			{
				GetWorld()->GetTimerManager().SetTimer(MantlingTimer, this,
				                                       &UGCBaseCharacterMovementComponent::EndMantle,
				                                       CurrentMantlingParameters.Duration, false);
				break;
			}
		case (uint8)ECustomMovementMode::CMOVE_Zipline:
			{
				GetWorld()->GetTimerManager().SetTimer(ZiplineTimer, this,
				                                       &UGCBaseCharacterMovementComponent::EndZipline,
				                                       ZiplineDuration, false);
				break;
			}
		case (uint8)ECustomMovementMode::CMOVE_WallRun:
			{
				GetWorld()->GetTimerManager().SetTimer(WallRunTimer, this,
				                                       &UGCBaseCharacterMovementComponent::EndWallRun,
				                                       MaxWallRunTime, false);
				break;
			}
		default:
			break;
		}
	}
}

AGCBaseCharacter* UGCBaseCharacterMovementComponent::GetBaseCharacterOwner() const
{
	return StaticCast<AGCBaseCharacter*>(CharacterOwner);
}

bool UGCBaseCharacterMovementComponent::IsCanWallRunning()
{
	return bCanWallRunning && DetectWall();
}

bool UGCBaseCharacterMovementComponent::AreRequiredKeysDown(EWallRunSide Side)
{
	if (ForwardAxis < 0.1f)
	{
		return false;
	}
	if (Side == EWallRunSide::Right && RightAxis < -0.f)
	{
		return false;
	}
	if (Side == EWallRunSide::Left && RightAxis > 0.1)
	{
		return false;
	}
	return true;
}

void FSavedMove_GC::Clear()
{
	Super::Clear();
	bSavedIsSprinting = 0;
	bSavedIsMantling = 0;
}

uint8 FSavedMove_GC::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bSavedIsSprinting)
	{
		Result |= FLAG_Custom_0;
	}
	if (bSavedIsMantling)
	{
		Result &= ~FLAG_JumpPressed;
		Result |= FLAG_Custom_1;
	}
	return Result;

	// FLAG_JumpPressed	= 0x01,	// Jump pressed
	// 	FLAG_WantsToCrouch	= 0x02,	// Wants to crouch
	// 	FLAG_Reserved_1		= 0x04,	// Reserved for future use
	// 	FLAG_Reserved_2		= 0x08,	// Reserved for future use
	// 	// Remaining bit masks are available for custom flags.
	// 	FLAG_Custom_0		= 0x10, todo  sprinting flag
	// 	FLAG_Custom_1		= 0x20, todo mantling flag
	// 	FLAG_Custom_2		= 0x40,
	// 	FLAG_Custom_3		= 0x80,
}

bool FSavedMove_GC::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_GC* NewMove = StaticCast<const FSavedMove_GC*>(NewMovePtr.Get());
	if (bSavedIsSprinting != NewMove->bSavedIsSprinting || bSavedIsMantling != NewMove->bSavedIsMantling)
	{
		return false;
	}

	return Super::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

void FSavedMove_GC::SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel,
                               FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(InCharacter, InDeltaTime, NewAccel, ClientData);

	check(InCharacter->IsA<AGCBaseCharacter>())
	AGCBaseCharacter* InBaseCharacter = StaticCast<AGCBaseCharacter*>(InCharacter);
	auto* MovementComponent = InBaseCharacter->GetBaseCharacterMovementComponent();
	bSavedIsSprinting = MovementComponent->IsSprinting();
	bSavedIsMantling = InBaseCharacter->bIsMantling;
}

void FSavedMove_GC::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);
	auto MovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(C->GetMovementComponent());
	MovementComponent->SetIsSprinting(bSavedIsSprinting);
}

FNetworkPredictionData_Client_GC::FNetworkPredictionData_Client_GC(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_GC::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_GC());
	// FNetworkPredictionData_Client_Character::AllocateNewMove()
}
