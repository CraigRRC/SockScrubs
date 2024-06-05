// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"
#include "Components/TextBlock.h"

void UPlayerHUDWidget::SetAmmoCounter(uint8 CurrentAmmo){
	FString AmmoString = FString::FromInt(CurrentAmmo);
	FText AmmoText = FText::FromString(AmmoString);
	AmmoCounter->SetText(AmmoText);
}

void UPlayerHUDWidget::SetAmmoCounterVisibility(ESlateVisibility Visiblity){
	AmmoCounter->SetVisibility(Visiblity);
}
