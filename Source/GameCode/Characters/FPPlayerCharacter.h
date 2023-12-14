// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "Controllers/GCPlayerController.h"
#include "FPPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API AFPPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual FRotator GetViewRotation() const override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void Landed(const FHitResult& Hit) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character | First person")
	USkeletalMeshComponent* FirstPersonMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character | First person")
	UCameraComponent* FirstPersonCameraComponent;

	// Ladder
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | First person | Camera | Ladder | Pitch",
		meta = (UIMin = -89.f, UIMax = 89.f))
	float LadderCameraMinPitch = -60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | First person | Camera | Ladder | Pitch",
		meta = (UIMin = -89.f, UIMax = 89.f))
	float LadderCameraMaxPitch = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | First person | Camera | Ladder | Yaw",
		meta = (UIMin = 0.f, UIMax = 359.f))
	float LadderCameraMinYaw = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | First person | Camera | Ladder | Yaw",
		meta = (UIMin = -89.f, UIMax = 89.f))
	float LadderCameraMaxYaw = 175.f;

	// Zipline
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | First person | Camera | Zipline | Pitch",
		meta = (UIMin = -89.f, UIMax = 89.f))
	float ZiplineCameraMinPitch = -89.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | First person | Camera | Zipline | Pitch",
		meta = (UIMin = -89.f, UIMax = 89.f))
	float ZiplineCameraMaxPitch = 89.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | First person | Camera | Zipline | Yaw",
		meta = (UIMin = -89.f, UIMax = 89.f))
	float ZiplineCameraMaxYaw = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character | First person | Camera | Zipline | Yaw",
		meta = (UIMin = -89.f, UIMax = 89.f))
	float ZiplineCameraMinYaw = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* FPHardLandingMontage;


	virtual void OnMantle(FMantlingSettings& MantlingSettings, float MantlingAnimStartTime) override;
	virtual void OnDeath() override;

private:
	FTimerHandle FPMontageTimer;
	TWeakObjectPtr<AGCPlayerController> GCPlayerController;

	void OnFPMontageTimerElapsed();
	bool IsFPMontagePlaying() const;
	void SetCameraConstraints(bool bIsMoveStarted, float ViewPitchMin_in,
	                          float ViewPitchMax_in, float ViewYawMin_in, float ViewYawMax_in);
};
