// Fill out your copyright notice in the Description page of Project Settings.


#include "GCPlayerController.h"

#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameCode/Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "GameCode/UI/Widget/ThrowableAmmoWidget.h"

void AGCPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	CachedBaseCharacter = Cast<AGCBaseCharacter>(InPawn);
	if (IsLocalController())
	{
		CreateAndInitializeWidgets();
	}
}

void AGCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAxis("MoveForward", this, &AGCPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AGCPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &AGCPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AGCPlayerController::LookUp);
	InputComponent->BindAxis("TurnAtRate", this, &AGCPlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUpAtRate", this, &AGCPlayerController::LookUpAtRate);
	InputComponent->BindAxis("SwimForward", this, &AGCPlayerController::SwimForward);
	InputComponent->BindAxis("SwimRight", this, &AGCPlayerController::SwimRight);
	InputComponent->BindAxis("SwimUp", this, &AGCPlayerController::SwimUp);
	InputComponent->BindAxis("ClimbLadderUp", this, &AGCPlayerController::ClimbLadderUp);
	InputComponent->BindAction("InteractWithLadder", IE_Pressed, this, &AGCPlayerController::InteractWithLadder);
	InputComponent->BindAction("Zipline", IE_Pressed, this, &AGCPlayerController::Zipline);
	InputComponent->BindAction("Mantle", IE_Pressed, this, &AGCPlayerController::Mantle);
	InputComponent->BindAction("Jump", IE_Pressed, this, &AGCPlayerController::Jump);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AGCPlayerController::ChangeCrouchState);
	InputComponent->BindAction("Sprint", IE_Pressed, this, &AGCPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &AGCPlayerController::StopSprint);
	InputComponent->BindAction("Wallrun", IE_Released, this, &AGCPlayerController::WallRun);
	InputComponent->BindAction("Fire", IE_Pressed, this, &AGCPlayerController::PlayerStartFire);
	InputComponent->BindAction("Fire", IE_Released, this, &AGCPlayerController::PlayerStopFire);
	InputComponent->BindAction("Aim", IE_Pressed, this, &AGCPlayerController::StartAiming);
	InputComponent->BindAction("Aim", IE_Released, this, &AGCPlayerController::StopAiming);
	InputComponent->BindAction("Reload", IE_Pressed, this, &AGCPlayerController::Reload);
	InputComponent->BindAction("NextItem", IE_Pressed, this, &AGCPlayerController::NextItem);
	InputComponent->BindAction("PreviousItem", IE_Pressed, this, &AGCPlayerController::PreviousItem);
	InputComponent->BindAction("ThrowPrimaryItem", IE_Pressed, this, &AGCPlayerController::ThrowPrimaryItem);
	InputComponent->BindAction("PrimarymeleeAttack", IE_Pressed, this, &AGCPlayerController::PrimaryMeleeAttack);
	InputComponent->BindAction("SecondaryMeleeAttack", IE_Pressed, this, &AGCPlayerController::SecondaryMeleeAttack);
}

void AGCPlayerController::MoveForward(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->GetBaseCharacterMovementComponent()->SetForwardAxis(Value);
		CachedBaseCharacter->MoveForward(Value);
	}
}

void AGCPlayerController::MoveRight(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->GetBaseCharacterMovementComponent()->SetRightAxis(Value);
		CachedBaseCharacter->MoveRight(Value);
	}
}

void AGCPlayerController::Turn(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Turn(Value);
	}
}

void AGCPlayerController::LookUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUp(Value);
	}
}

void AGCPlayerController::TurnAtRate(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->TurnAtRate(Value);
	}
}

void AGCPlayerController::LookUpAtRate(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUpAtRate(Value);
	}
}

void AGCPlayerController::ChangeCrouchState()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ChangeCrouchState();
	}
}

void AGCPlayerController::Mantle()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Mantle();
	}
}

void AGCPlayerController::Zipline()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Zipline();
	}
}

void AGCPlayerController::WallRun()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartWallRun();
	}
}

void AGCPlayerController::ThrowPrimaryItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ThrowPrimaryItem();
	}
}

void AGCPlayerController::Jump()
{
	if (CachedBaseCharacter.IsValid())
	{
		if (!CachedBaseCharacter->GetBaseCharacterMovementComponent()->IsMantling() &&
			!CachedBaseCharacter->GetBaseCharacterMovementComponent()->IsOnLadder() &&
			!CachedBaseCharacter->GetBaseCharacterMovementComponent()->IsOnZipline() &&
			!CachedBaseCharacter->GetBaseCharacterMovementComponent()->IsFalling())
		{
			CachedBaseCharacter->Jump();
		}
	}
}

void AGCPlayerController::SwimForward(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimForward(Value);
	}
}

void AGCPlayerController::SwimRight(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimRight(Value);
	}
}

void AGCPlayerController::SwimUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimUp(Value);
	}
}

void AGCPlayerController::ClimbLadderUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ClimbLadderUp(Value);
	}
}

void AGCPlayerController::InteractWithLadder()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->InteractWithLadder();
	}
}

void AGCPlayerController::StartSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartSprint();
	}
}

void AGCPlayerController::StopSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopSprint();
	}
}

void AGCPlayerController::PlayerStartFire()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartFire();
	}
}

void AGCPlayerController::PlayerStopFire()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopFire();
	}
}

void AGCPlayerController::StartAiming()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartAiming();
	}
}

void AGCPlayerController::StopAiming()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopAiming();
	}
}

void AGCPlayerController::Reload()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Reload();
	}
}

void AGCPlayerController::NextItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->NextItem();
	}
}

void AGCPlayerController::PreviousItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->PreviousItem();
	}
}

void AGCPlayerController::PrimaryMeleeAttack()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->PrimaryMeleeAttack();
	}
}

void AGCPlayerController::SecondaryMeleeAttack()
{
	if (CachedBaseCharacter)
	{
		CachedBaseCharacter->SecondaryMeleeAttack();
	}
}

void AGCPlayerController::CreateAndInitializeWidgets()
{
	if (!PlayerHUDWidget)
	{
		PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass);
		PlayerHUDWidget->AddToViewport();
	}
	if (CachedBaseCharacter)
	{
		auto CharacterEquipment = CachedBaseCharacter->GetCharacterEquipmentComponent_Mutable();

		UReticleWidget* ReticleWidget = PlayerHUDWidget->GetReticleWidget();
		if (ReticleWidget && CachedBaseCharacter.IsValid())
		{
			CachedBaseCharacter->OnAimingStateChanged.AddUFunction(ReticleWidget, FName("OnAimingStateChanged"));
			CharacterEquipment->OnEquippedItemChangedEvent.AddUFunction(ReticleWidget, FName("OnEquippedItemChanged"));
		}
		UAmmoWidget* AmmoWidget = PlayerHUDWidget->GetAmmoWidget();
		if (AmmoWidget)
		{
			CharacterEquipment->OnCurrentWeaponAmmoChangedEvent.AddUFunction(AmmoWidget, FName("UpdateAmmoCount"));
		}
		UWidgetCharacterAttributes* WidgetCharacterAttributes = PlayerHUDWidget->GetWidgetCharacterAttributes();
		if (WidgetCharacterAttributes)
		{
			UCharacterAttributeComponent* CharacterAttributeComponent = CachedBaseCharacter->
				GetCharacterAttributeComponent_Mutable();
			CharacterAttributeComponent->OnHealthChangedEvent.AddDynamic(WidgetCharacterAttributes,
			                                                        &UWidgetCharacterAttributes::UpdateHealthPercent);
			CharacterAttributeComponent->OnStaminaChanged.AddDynamic(WidgetCharacterAttributes,
			                                                         &UWidgetCharacterAttributes::UpdateStaminaPercent);
			CharacterAttributeComponent->OnOxygenChanged.AddDynamic(WidgetCharacterAttributes,
			                                                        &UWidgetCharacterAttributes::UpdateOxygenPercent);
		}
		UThrowableAmmoWidget* ThrowableAmmoWidget = PlayerHUDWidget->GetThrowableAmmoWidget();
		if (ThrowableAmmoWidget)
		{
			CharacterEquipment->OnCurrentThrowableAmmoChangedEvent.AddDynamic(
				ThrowableAmmoWidget, &UThrowableAmmoWidget::UpdateThrowableAmmoCount);
		}
	}
}
