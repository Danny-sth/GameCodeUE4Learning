// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GCBaseCharacterMovementComponent.generated.h"

struct FMantlingMovementParameters
{
	FVector InitialLocation = FVector::ZeroVector;
	FRotator InitialRotation = FRotator::ZeroRotator;

	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	FVector InitialAnimationLocation = FVector::ZeroVector;

	float Duration = 1.0f;
	float StartTime = 0.0f;

	UCurveVector* MantlingCurve;
	TWeakObjectPtr<UPrimitiveComponent> TargetComponent;
};

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	CMOVE_None = 0 UMETA(DisplayName = "None"),
	CMOVE_Mantling UMETA(DisplayName = "Mantling"),
	CMOVE_Ladder UMETA(DisplayName = "Ladder"),
	CMOVE_Zipline UMETA(DisplayName = "Zipline"),
	CMOVE_WallRun UMETA(DisplayName = "Wall run"),
	CMOVE_Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDetachFromLadderMethod : uint8
{
	Fall = 0,
	ReachingTheTop,
	ReachingTheBottom,
	JumpOff
};

UENUM(BlueprintType)
enum class EWallRunSide : uint8
{
	None,
	Left,
	Right
};

/**
 * 
 */
UCLASS()
class GAMECODE_API UGCBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual void PhysicsRotation(float DeltaTime) override;

	bool IsSprinting() const { return bIsSprinting; }

	void SetIsSprinting(bool bIsSprinting_In)
	{
		bIsSprinting = bIsSprinting_In;
	}

	bool IsOutOfStamina() const { return bIsOutOfStamina; }
	void SetOutOfStamina(bool bIsOutOfStamina_In) { bIsOutOfStamina = bIsOutOfStamina_In; }

	virtual float GetMaxSpeed() const override;

	void StartSprint();
	void StopSprint();

	void StartZipline();
	void EndZipline();
	bool IsOnZipline() const;
	UCurveVector* GetZiplineCurve() const { return ZiplineCurve; }
	void SetZipLineDuration(const float Value) { ZiplineDuration = Value; }
	float GetZiplineSpeed() const { return ZiplineSpeed; }

	void StartMantle(const FMantlingMovementParameters& MantlingParameters);
	void EndMantle();
	bool IsMantling() const;


	void AttachToLadder(const class ALadder* Ladder);

	float GetActorToCurrentLadderProjection(const FVector& Location) const;

	float GetLadderSpeedRatio() const;

	void DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod = EDetachFromLadderMethod::Fall);
	bool IsOnLadder() const;
	const class ALadder* GetCurrentLadder();

	// Wall run
	bool DetectWall();
	bool CheckWallOnTheSide(FVector TraceVector, EWallRunSide Side);
	void StartWallRun();
	void EndWallRun();
	void SetForwardAxis(const float Value) { this->ForwardAxis = Value; }
	void SetRightAxis(const float Value) { this->RightAxis = Value; }
	bool IsWallRunning() const { return bIsWallRunning; }
	EWallRunSide GetDetectedSide() const { return DetectedSide; }

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	void PhysMantling(float DeltaTime, int32 Iterations);
	void PhysLadder(float DeltaTime, int32 Iterations);
	void PhysZipline(float DeltaTime, int32 Iterations);
	void PhysWallRun(float DeltaTime, int32 Iterations);

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: sprint",
		meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: out of stamina",
		meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float OutOfStaminaSpeed = 300.0f;

	UPROPERTY(Category="Character Movement: Swimming", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float SwimmingCapsuleRadius = 60.0f;

	UPROPERTY(Category="Character Movement: Swimming", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float SwimmingCapsuleHalfHeight = 60.0f;

	UPROPERTY(Category="Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ClimbingOnLadderMaxSpeed = 200.0f;

	UPROPERTY(Category="Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ClimbingOnLadderBrakingDecelartion = 2048.0f;

	UPROPERTY(Category="Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float LadderToCharacterOffset = 60.0f;

	UPROPERTY(Category="Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float MaxLadderTopOffset = 90.0f;

	UPROPERTY(Category="Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float MinLadderBottomOffset = 90.0f;

	UPROPERTY(Category="Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float JumpOffFromLadderSpeed = 500.0f;

	UPROPERTY(Category="Character Movement: Zipline", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ZiplineSpeed = 700.0f; // todo

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveVector* ZiplineCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: wall run")
	float MaxWallRunTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: wall run")
	float MaxWallRunSpeed = 800.0f;

	class AGCBaseCharacter* GetBaseCharacterOwner() const;

private:
	bool bIsSprinting = false;

	bool bIsOutOfStamina = false;

	FMantlingMovementParameters CurrentMantlingParameters;
	FTimerHandle MantlingTimer;

	const ALadder* CurrentLadder = nullptr;

	FRotator ForceTargetRotation = FRotator::ZeroRotator;
	bool bForceRotation = false;

	FTimerHandle ZiplineTimer;
	float ZiplineDuration;
	float TraceDistance = 50.f;

	// Wall run
	FTimerHandle WallRunTimer;
	FHitResult WallHitResult;
	EWallRunSide DetectedSide = EWallRunSide::None;
	FVector CurrentWallRunDirection = FVector::ZeroVector;
	float ForwardAxis = 0.f;
	float RightAxis = 0.f;
	bool bIsWallRunning = false;
	bool bCanWallRunning = true;

	bool IsCanWallRunning();
	bool AreRequiredKeysDown(EWallRunSide Side);
};

class FSavedMove_GC : public FSavedMove_Character
{
	typedef FSavedMove_Character Super;

public:
	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter,
	                            float MaxDelta) const override;

	virtual void SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel,
	                        FNetworkPredictionData_Client_Character& ClientData) override;

	virtual void PrepMoveFor(ACharacter* C) override;

private:
	uint8 bSavedIsSprinting : 1;
	uint8 bSavedIsMantling : 1;
};

class FNetworkPredictionData_Client_GC : public FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;

public:
	FNetworkPredictionData_Client_GC(const UCharacterMovementComponent& ClientMovement);
	virtual FSavedMovePtr AllocateNewMove() override;
};
