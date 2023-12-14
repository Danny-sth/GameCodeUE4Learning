// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetCharacterAttributes.h"

void UWidgetCharacterAttributes::UpdateHealthPercent(float NewHealthPercent)
{
	HealthPercent = NewHealthPercent;
}

void UWidgetCharacterAttributes::UpdateStaminaPercent(float NewStaminaPercent)
{
	StaminaPercent = NewStaminaPercent;
}

void UWidgetCharacterAttributes::UpdateOxygenPercent(float NewOxygenPercent)
{
	OxygenPercent = NewOxygenPercent;
}
