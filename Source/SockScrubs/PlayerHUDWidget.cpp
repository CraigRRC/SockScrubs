// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Kismet/KismetStringLibrary.h"

void UPlayerHUDWidget::SetAmmoCounter(uint8 CurrentAmmo){
	FString AmmoString = FString::FromInt(CurrentAmmo);
	FText AmmoText = FText::FromString(AmmoString);
	AmmoCounter->SetText(AmmoText);
}

void UPlayerHUDWidget::SetAmmoCounterVisibility(ESlateVisibility V){
	AmmoCounter->SetVisibility(V);
}

void UPlayerHUDWidget::SetAdrenalineBarPercent(float HealthAsPercent){
	AdrenalineBar->SetPercent(HealthAsPercent);
}

void UPlayerHUDWidget::SetSloMoBarPercent(float Percent){
	SloMoBar->SetPercent(Percent);
}

void UPlayerHUDWidget::SetComboBarPercent(float Percent){
	ComboBar->SetPercent(Percent);
}

void UPlayerHUDWidget::SetComboCounterVisibility(ESlateVisibility V){
	ComboCounter->SetVisibility(V);
	ComboBar->SetVisibility(V);
}

void UPlayerHUDWidget::SetComboCounterText(uint8 Combo){
	FString ComboString = FString::FromInt(Combo);
	FText ComboText = FText::FromString(ComboString);
	ComboCounter->SetText(ComboText);
}

void UPlayerHUDWidget::SetRunTimerText(double time){
	FString TimerString = UKismetStringLibrary::TimeSecondsToString(time);
	FText TimerText = FText::FromString(TimerString);
	Timer->SetText(TimerText);
}

void UPlayerHUDWidget::SetRunTimerVisibility(ESlateVisibility V){
	Timer->SetVisibility(V);
}

void UPlayerHUDWidget::SetOutOfAmmoVisiblilty(ESlateVisibility V){
	//Temp
	OutOfAmmo1->SetVisibility(V);
	OutOfAmmo2->SetVisibility(V);
	OutOfAmmo3->SetVisibility(V);
	OutOfAmmo4->SetVisibility(V);
}
