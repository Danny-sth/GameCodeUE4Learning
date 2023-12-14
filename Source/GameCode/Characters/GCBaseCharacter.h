// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameCode/GameCodeTypes.h"
#include "GameCode/Actors/Interactive/Environment/Zipline.h"
#include "GameCode/Actors/Interactive/Environment/Ladder.h"
#include "GameCode/Components/CharacterComponents/CharacterAttributeComponent.h"
#include "GameCode/Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GCBaseCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool)
class UCharacterEquipmentComponent;

USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* MantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* FPMantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveVector* MantligCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionXY = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionZ = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeight = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeight = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeightStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeightStartTime = 0.5f;
};

class AInteractiveActor;
class UGCBaseCharacterMovementComponent;

UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AGCBaseCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGCBaseCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void MoveForward(float Value)
	{
	}

	virtual void MoveRight(float Value)
	{
	}

	void StartWallRun();

	virtual void Turn(float Value)
	{
	}

	virtual void LookUp(float Value)
	{
	}

	virtual void TurnAtRate(float Value)
	{
	}

	virtual void LookUpAtRate(float Value)
	{
	}

	virtual void ChangeCrouchState();

	virtual void StartSprint();
	virtual void StopSprint();
	bool CanFire();

	virtual void SwimForward(float Value)
	{
	}

	virtual void SwimRight(float Value)
	{
	}

	virtual void SwimUp(float Value)
	{
	}

	virtual void StartFire();
	void StopFire();
	void StartAiming();
	void StopAiming();
	bool IsAiming() const { return bIsAiming; }
	void Reload();
	float GetAimingMovementSpeed() const;
	void NextItem();
	void PreviousItem();
	void ThrowPrimaryItem();
	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();

	FOnAimingStateChanged OnAimingStateChanged;

	FRotator GetAimOffset();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Character")
	void OnStartAiming();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Character")
	void OnStopAiming();

	UFUNCTION(BlueprintCallable)
	void Mantle(bool bForce = false);
	virtual void Zipline();

	UPROPERTY(ReplicatedUsing=OnRep_IsMantling)
	bool bIsMantling;

	UFUNCTION()
	void OnRep_IsMantling(bool bIsMantling_Old);

	virtual bool CanJumpInternal_Implementation() const override;

	FORCEINLINE UGCBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const
	{
		return GCBaseCharacterMovementComponent;
	}

	float GetIKRightFootOffset() const { return IKRightFootOffset; }

	float GetIKLeftFootOffset() const { return IKLeftFootOffset; }

	float GetIKPelvisOffset() const { return IKPelvisOffset; }

	void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	void UnregisterInteractiveActor(AInteractiveActor* InteractiveActor);

	void ClimbLadderUp(float Value);
	void InteractWithLadder();
	const ALadder* GetAvailableLadder() const;
	AZipline* GetAvailableZipline();

	bool IsSwimmingUnderWater() const;

	const UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const { return CharacterEquipmentComponent; }
	UCharacterEquipmentComponent* GetCharacterEquipmentComponent_Mutable() const { return CharacterEquipmentComponent; }
	const UCharacterAttributeComponent* GetCharacterAttributeComponent() const { return CharacterAttributeComponent; }
	UCharacterAttributeComponent* GetCharacterAttributeComponent_Mutable() const { return CharacterAttributeComponent; }

	// IGenericTeamAgentInterface
	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId((uint8)Team); }
	// IGenericTeamAgentInterface

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|IK Setting")
	FName RightFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|IK Setting")
	FName LeftFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|IK Setting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKTraceDistance = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|IK Setting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKInterpSpeed = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Controls")
	float BaseTurnRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Controls")
	float BaseLookUpRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Movement")
	float SprintSpeed = 800.0f;

	UGCBaseCharacterMovementComponent* GCBaseCharacterMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Movement")
	class ULedgeDetectorComponent* LedgeDetectorComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings HighMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings LowMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling",
		meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LowMantleMaxHeight = 125.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | Landing")
	float MaxSafeAltitude = 4.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | Landing")
	UAnimMontage* HardLandingMontage;
	FVector DropAltitude;
	FTimerHandle MontageTimer;
	void OnMontageTimerElapsed();

	// Damage depending from fall height (in meters)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Attributes")
	UCurveFloat* FallDamageCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character | Components")
	UCharacterAttributeComponent* CharacterAttributeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Animations")
	UAnimMontage* OnDeathAnimMontage;

	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintStart();
	virtual void OnSprintStart_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintEnd();
	virtual void OnSprintEnd_Implementation();

	virtual bool CanSprint();

	bool CanMantle() const;
	bool CanZipline();

	virtual void OnMantle(FMantlingSettings& MantlingSettings, float MantlingAnimaStartTime);

	virtual void Falling() override;
	virtual void NotifyJumpApex() override;
	virtual void Landed(const FHitResult& Hit) override;

	virtual void OnDeath();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class UCharacterEquipmentComponent* CharacterEquipmentComponent;

	virtual void OnStartAimingInternal();
	virtual void OnStopAimingInternal();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Team")
	ETeams Team = ETeams::Enemy;

private:
	void UpdateIKSettings(float DeltaSeconds);
	float CalculateIKParametersForSocketName(const FName& SocketName) const;

	float CalculateIKPelvisOffset();

	float IKLeftFootOffset = 0.0f;
	float IKRightFootOffset = 0.0f;
	float IKPelvisOffset = 0.0f;

	void TryChangeSprintState(float DeltaTime);
	void SetOutOfStamina(bool bOutOfStamina) { GCBaseCharacterMovementComponent->SetOutOfStamina(bOutOfStamina); }
	bool bIsSprintRequested = false;

	FMantlingSettings& GetMantlingSettings(float LedgeHeight);

	TInteractiveActorsArray AvailableInteractiveActors;
	FTimerHandle DeathMontageTimer;
	void EnableRagdoll();

	bool bIsAiming;
	float CurrentAimingMovementSpeed;
};
