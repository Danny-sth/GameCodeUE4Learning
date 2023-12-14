// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WidgetCharacterAttributes.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UWidgetCharacterAttributes : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void UpdateHealthPercent(float NewHealthPercent);

	UFUNCTION()
	void UpdateStaminaPercent(float NewStaminaPercent);

	UFUNCTION()
	void UpdateOxygenPercent(float NewOxygenPercent);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	float HealthPercent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	float StaminaPercent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	float OxygenPercent;
};
