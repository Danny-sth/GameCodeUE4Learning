// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterAnimInstance.h"

#include "GameCode/Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameCode/Components/MovementComponents/GCBaseCharacterMovementComponent.h"


void UGCBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AGCBaseCharacter>(),
	       TEXT(
		       "UGCBaseCharacterAnimInstance::NativeBeginPlay() UGCBaseCharacterAnimInstance can be used only with AGCBaseCharacter"
	       ));
	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(TryGetPawnOwner());
}

void UGCBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedBaseCharacter.IsValid())
	{
		return;
	}
	bIsAiming = CachedBaseCharacter->IsAiming();
	UGCBaseCharacterMovementComponent* CharacterMovement = CachedBaseCharacter->GetBaseCharacterMovementComponent();
	Speed = CharacterMovement->Velocity.Size();
	bIsFalling = CharacterMovement->IsFalling();
	bIsCrouching = CharacterMovement->IsCrouching();
	bIsSprining = CharacterMovement->IsSprinting();
	bIsOutOfStamina = CharacterMovement->IsOutOfStamina();
	bIsSwimming = CharacterMovement->IsSwimming();
	bIsOnLadder = CharacterMovement->IsOnLadder();
	if (bIsOnLadder)
	{
		LadderSpeedRatio = CharacterMovement->GetLadderSpeedRatio();
	}
	bIsStrafing = !CharacterMovement->bOrientRotationToMovement;
	Direction = CalculateDirection(CharacterMovement->Velocity, CachedBaseCharacter->GetActorRotation());

	IKRightFootOffset = FVector(
		(CachedBaseCharacter->GetIKRightFootOffset() + CachedBaseCharacter->GetIKPelvisOffset()), 0.0f, 0.0f);
	IKLeftFootOffset = FVector(-(CachedBaseCharacter->GetIKLeftFootOffset() + CachedBaseCharacter->GetIKPelvisOffset()),
	                           0.0f, 0.0f);
	IKPelvisBoneOffset = FVector(0.0f, 0.0f, CachedBaseCharacter->GetIKPelvisOffset());
	bIsOnZipline = CharacterMovement->IsOnZipline();
	bIsWallRunning = CharacterMovement->IsWallRunning();
	DetectedWallRunSide = CharacterMovement->GetDetectedSide();

	const UCharacterEquipmentComponent* CharacterEquipmentComponent = CachedBaseCharacter->
		GetCharacterEquipmentComponent();
	CurrentEquippedItemType = CharacterEquipmentComponent->GetCurrentEquippedItemType();
	AimRotationPitch = CachedBaseCharacter->GetAimOffset().Pitch;
	AimRotationYaw = CachedBaseCharacter->GetAimOffset().Yaw;
	ARangeWeaponItem* CurrentWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (CurrentWeapon)
	{
		ForeGripSocketTransform = CurrentWeapon->GetForeGripTransform();
	}
}
