// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerAnimInstance.h"

void UFPPlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AFPPlayerCharacter>(), TEXT("Only AFPPlayerCharacter can use this AnimInstance"))
	CachedFirstPersonCharacterOwner = StaticCast<AFPPlayerCharacter*>(TryGetPawnOwner());
}

void UFPPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedFirstPersonCharacterOwner.IsValid())
	{
		return;
	}
	PlayerCameraPitchAngle = CalculateCameraPitchAngle();
}

float UFPPlayerAnimInstance::CalculateCameraPitchAngle() const
{
	float Result = 0;
	AGCPlayerController* Controller = CachedFirstPersonCharacterOwner->GetController<AGCPlayerController>();
	if (IsValid(Controller) && !Controller->GetIgnoreCameraPitch())
	{
		Result = Controller->GetControlRotation().Pitch;
	}
	return Result;
}
