// Fill out your copyright notice in the Description page of Project Settings.


#include "ReticleWidget.h"

void UReticleWidget::OnAimingStateChanged_Implementation(bool bIsAiming)
{
	SetupCurrentReticle();
}

void UReticleWidget::OnEquippedItemChanged_Implementation(const AEquipableItem* NewEquippedItem)
{
	CurrentEquippedItem = NewEquippedItem;
	SetupCurrentReticle();
}

void UReticleWidget::SetupCurrentReticle()
{
	CurrentReticleType = CurrentEquippedItem.IsValid() ? CurrentEquippedItem->GetReticleType() : EReticleType::None;
}
