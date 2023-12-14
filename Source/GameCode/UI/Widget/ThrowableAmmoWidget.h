// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ThrowableAmmoWidget.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UThrowableAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void UpdateThrowableAmmoCount(int32 NewAmmo);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ammo")
	int32 CurrentAmmo;
};
