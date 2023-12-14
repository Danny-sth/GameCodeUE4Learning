// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AmmoWidget.h"
#include "ReticleWidget.h"
#include "WidgetCharacterAttributes.h"
#include "ThrowableAmmoWidget.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UReticleWidget* GetReticleWidget() const;
	UAmmoWidget* GetAmmoWidget() const;
	UWidgetCharacterAttributes* GetWidgetCharacterAttributes() const;
	UThrowableAmmoWidget* GetThrowableAmmoWidget() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widget names")
	FName ReticleWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widget names")
	FName AmmoWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widget names")
	FName CharacterAttributesWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widget names")
	FName ThrowableAmmoWidgetName;
};
