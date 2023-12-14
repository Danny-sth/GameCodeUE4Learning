// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SetMeleeHitRegEnabled.h"

#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotify_SetMeleeHitRegEnabled::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AGCBaseCharacter* CharacterOwner = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (!CharacterOwner)
	{
		return;
	}
	auto CurrentMeleeWeapon = CharacterOwner->GetCharacterEquipmentComponent()->GetCurrentMeleeWeapon();
	if (CurrentMeleeWeapon)
	{
		CurrentMeleeWeapon->SetHitRegistrationActive(bIsHitRegistratorActive);
	}
}
