// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCBaseCharacterAnimInstance.h"
#include "GameCode/Characters/FPPlayerCharacter.h"
#include "FPPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UFPPlayerAnimInstance : public UGCBaseCharacterAnimInstance
{
	GENERATED_BODY()

	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character | Animations | FirstPerson")
	float PlayerCameraPitchAngle = 0.f;

	TWeakObjectPtr<AFPPlayerCharacter> CachedFirstPersonCharacterOwner;

private:
	float CalculateCameraPitchAngle() const;
};
