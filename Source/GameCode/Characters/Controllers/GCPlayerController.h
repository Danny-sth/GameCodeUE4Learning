// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCode/UI/Widget/PlayerHUDWidget.h"
#include "GameFramework/PlayerController.h"
#include "GCPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API AGCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;
	bool GetIgnoreCameraPitch() const { return bIgnoreCameraPitch; }
	void SetIgnoreCameraPitch(const bool bIgnoreCameraPitch_In) { bIgnoreCameraPitch = bIgnoreCameraPitch_In; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widgets")
	TSubclassOf<UPlayerHUDWidget> PlayerHUDWidgetClass;

	void Zipline();
	void WallRun();
	void ThrowPrimaryItem();
	virtual void SetupInputComponent() override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void ChangeCrouchState();
	void Mantle();
	void Jump();

	void SwimForward(float Value);
	void SwimRight(float Value);
	void SwimUp(float Value);

	void ClimbLadderUp(float Value);
	void InteractWithLadder();

	void StartSprint();
	void StopSprint();

	void PlayerStartFire();
	void PlayerStopFire();
	void StartAiming();
	void StopAiming();
	void Reload();
	void NextItem();
	void PreviousItem();

	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();

	TSoftObjectPtr<class AGCBaseCharacter> CachedBaseCharacter;

private:
	void CreateAndInitializeWidgets();
	UPlayerHUDWidget* PlayerHUDWidget = nullptr;
	bool bIgnoreCameraPitch = false;
};
