// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Controllers/GCPlayerController.h"
#include "GameCode/Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

AFPPlayerCharacter::AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal mesh component"));
	FirstPersonMeshComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -86.f));
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bCastDynamicShadow = false;
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First person camera"));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent);

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;

	CameraComponent->SetAutoActivate(false);
	SpringArmComponent->SetAutoActivate(false);
	SpringArmComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
}

void AFPPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	FirstPersonCameraComponent->AttachToComponent(FirstPersonMeshComponent,
	                                              FAttachmentTransformRules::KeepWorldTransform, SocketFPCameraName);
}

void AFPPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsFPMontagePlaying() && GCPlayerController.IsValid())
	{
		FRotator TargetControlRotation = GCPlayerController->GetControlRotation();
		TargetControlRotation.Pitch = 0.f;
		float BlendSpeed = 300.f;
		TargetControlRotation = FMath::RInterpTo(GCPlayerController->GetControlRotation(),
		                                         TargetControlRotation, DeltaSeconds, BlendSpeed);
		GCPlayerController->SetControlRotation(TargetControlRotation);
	}
}

void AFPPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	GCPlayerController = Cast<AGCPlayerController>(NewController);
}

bool AFPPlayerCharacter::IsFPMontagePlaying() const
{
	UAnimInstance* AnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	return IsValid(AnimInstance) && AnimInstance->IsAnyMontagePlaying();
}

FRotator AFPPlayerCharacter::GetViewRotation() const
{
	FRotator ViewRotation = Super::GetViewRotation();
	if (IsFPMontagePlaying())
	{
		FRotator SocketRotation = FirstPersonMeshComponent->GetSocketRotation(SocketFPCameraName);
		ViewRotation.Yaw = SocketRotation.Yaw;
		ViewRotation.Roll = SocketRotation.Roll;
		ViewRotation.Pitch += SocketRotation.Pitch;
	}
	return ViewRotation;
}

void AFPPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->
	                                    FirstPersonMeshComponent->GetRelativeLocation().Z + HalfHeightAdjust;
}

void AFPPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->
	                                    FirstPersonMeshComponent->GetRelativeLocation().Z;
}

void AFPPlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	if (GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		SetCameraConstraints(true, LadderCameraMinPitch, LadderCameraMaxPitch,
		                     LadderCameraMinYaw, LadderCameraMaxYaw);
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		SetCameraConstraints(false, LadderCameraMinPitch, LadderCameraMaxPitch,
		                     LadderCameraMinYaw, LadderCameraMaxYaw);
	}

	if (GetBaseCharacterMovementComponent()->IsOnZipline())
	{
		SetCameraConstraints(true, ZiplineCameraMinPitch, ZiplineCameraMaxPitch,
		                     ZiplineCameraMinYaw, ZiplineCameraMaxYaw);
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
	{
		SetCameraConstraints(false, ZiplineCameraMinPitch, ZiplineCameraMaxPitch,
		                     ZiplineCameraMinYaw, ZiplineCameraMaxYaw);
	}
}

void AFPPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (GetActorLocation().Z <= DropAltitude.Z)
	{
		if ((DropAltitude - GetActorLocation()).Z > MaxSafeAltitude)
		{
			GCPlayerController->SetIgnoreLookInput(true);
			GCPlayerController->SetIgnoreMoveInput(true);
			const float Duration = FirstPersonMeshComponent->GetAnimInstance()->Montage_Play(FPHardLandingMontage, 1.f,
				EMontagePlayReturnType::Duration, 0, false);
			GetWorld()->GetTimerManager().SetTimer(FPMontageTimer, this,
			                                       &AFPPlayerCharacter::OnFPMontageTimerElapsed, Duration, false);
		}
	}
}


void AFPPlayerCharacter::SetCameraConstraints(bool bIsMoveStarted, float ViewPitchMin_in,
                                              float ViewPitchMax_in, float ViewYawMin_in, float ViewYawMax_in)
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreCameraPitch(bIsMoveStarted);
		bUseControllerRotationYaw = !bIsMoveStarted;
		APlayerCameraManager* DefaultCameraManager = GCPlayerController->PlayerCameraManager->GetClass()->
		                                                                 GetDefaultObject<APlayerCameraManager>();
		GCPlayerController->PlayerCameraManager->ViewPitchMin = bIsMoveStarted
			                                                        ? ViewPitchMin_in
			                                                        : DefaultCameraManager->ViewPitchMin;
		GCPlayerController->PlayerCameraManager->ViewPitchMax = bIsMoveStarted
			                                                        ? ViewPitchMax_in
			                                                        : DefaultCameraManager->ViewPitchMax;
		GCPlayerController->PlayerCameraManager->ViewYawMin = bIsMoveStarted
			                                                      ? ViewYawMin_in
			                                                      : DefaultCameraManager->ViewYawMin;
		GCPlayerController->PlayerCameraManager->ViewYawMax = bIsMoveStarted
			                                                      ? ViewYawMax_in
			                                                      : DefaultCameraManager->ViewYawMax;
	}
}

void AFPPlayerCharacter::OnMantle(FMantlingSettings& MantlingSettings, float MantlingAnimStartTime)
{
	Super::OnMantle(MantlingSettings, MantlingAnimStartTime);
	UAnimInstance* AnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	if (IsValid(AnimInstance) && MantlingSettings.MantlingMontage)
	{
		if (GCPlayerController.IsValid())
		{
			GCPlayerController->SetIgnoreLookInput(true);
			GCPlayerController->SetIgnoreMoveInput(true);
		}
		float Duration = AnimInstance->Montage_Play(MantlingSettings.FPMantlingMontage, 1.f,
		                                            EMontagePlayReturnType::Duration, MantlingAnimStartTime);
		GetWorld()->GetTimerManager().SetTimer(MontageTimer, this,
		                                       &AFPPlayerCharacter::OnFPMontageTimerElapsed, Duration, false);
	}
}

void AFPPlayerCharacter::OnDeath()
{
	Super::OnDeath();
	FirstPersonMeshComponent->SetCollisionProfileName(CollisionProfileRagdoll);
	FirstPersonMeshComponent->SetSimulatePhysics(true);
}

void AFPPlayerCharacter::OnFPMontageTimerElapsed()
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreLookInput(false);
		GCPlayerController->SetIgnoreMoveInput(false);
	}
}
