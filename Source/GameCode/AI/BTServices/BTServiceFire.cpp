// Fill out your copyright notice in the Description page of Project Settings.


#include "BTServiceFire.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameCode/Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Components/CharacterComponents/CharacterEquipmentComponent.h"

UBTServiceFire::UBTServiceFire()
{
	NodeName = "Fire";
}

void UBTServiceFire::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	AAIController* AaiController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();

	if (!AaiController || !BlackboardComponent)
	{
		return;
	}

	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(AaiController->GetPawn());
	if (!Character)
	{
		return;
	}

	const UCharacterEquipmentComponent* CharacterEquipmentComponent = Character->GetCharacterEquipmentComponent();
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();

	if (!CurrentRangeWeapon)
	{
		return;
	}

	AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!CurrentTarget)
	{
		Character->StopFire();
		return;
	}

	float DistSq = FVector::DistSquared(CurrentTarget->GetActorLocation(), Character->GetActorLocation());
	if (DistSq > FMath::Square(MaxFireDistance))
	{
		Character->StopFire();
		return;
	}

	if (!CurrentRangeWeapon->IsReloading() || !CurrentRangeWeapon->IsFiring())
	{
		Character->StartFire();
	}
}
