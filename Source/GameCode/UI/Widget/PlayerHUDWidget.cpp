// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"
#include "Blueprint/WidgetTree.h"

UReticleWidget* UPlayerHUDWidget::GetReticleWidget() const
{
	return WidgetTree->FindWidget<UReticleWidget>(ReticleWidgetName);
}

UAmmoWidget* UPlayerHUDWidget::GetAmmoWidget() const
{
	return WidgetTree->FindWidget<UAmmoWidget>(AmmoWidgetName);
}

UWidgetCharacterAttributes* UPlayerHUDWidget::GetWidgetCharacterAttributes() const
{
	return WidgetTree->FindWidget<UWidgetCharacterAttributes>(CharacterAttributesWidgetName);
}

UThrowableAmmoWidget* UPlayerHUDWidget::GetThrowableAmmoWidget() const
{
	return WidgetTree->FindWidget<UThrowableAmmoWidget>(ThrowableAmmoWidgetName);
}
