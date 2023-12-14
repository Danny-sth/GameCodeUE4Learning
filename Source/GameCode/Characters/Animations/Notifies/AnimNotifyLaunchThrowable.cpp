// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyLaunchThrowable.h"

#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotifyLaunchThrowable::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AGCBaseCharacter* CharacterOwner = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (!CharacterOwner)
	{
		return;
	}
	CharacterOwner->GetCharacterEquipmentComponent_Mutable()->LaunchCurrentThrowable();
}
