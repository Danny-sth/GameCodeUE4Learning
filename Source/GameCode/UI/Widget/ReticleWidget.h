// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameCode/GameCodeTypes.h"
#include "GameCode/Actors/Equipment/EquipableItem.h"
#include "ReticleWidget.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UReticleWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnAimingStateChanged(bool bIsAiming);

	UFUNCTION(BlueprintNativeEvent)
	void OnEquippedItemChanged(const AEquipableItem* NewEquippedItem);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Reticle")
	EReticleType CurrentReticleType = EReticleType::None;

private:
	TWeakObjectPtr<const AEquipableItem> CurrentEquippedItem;

	void SetupCurrentReticle();
};
