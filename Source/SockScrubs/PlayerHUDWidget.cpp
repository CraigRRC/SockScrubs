// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
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

	uint8 Minutes = static_cast<uint8>(time / 60);
	uint8 Seconds = static_cast<uint8>(time) % 60;
	uint8 Milliseconds = static_cast<uint8>(time - FMath::FloorToInt(time * 1000.f));

	//FString TimerString = FString::Printf(TEXT("%02d:%02d.%03d"), Minutes, Seconds, Milliseconds);
	FString TimerString = UKismetStringLibrary::TimeSecondsToString(time);
	FText TimerText = FText::FromString(TimerString);
	Timer->SetText(TimerText);
}

void UPlayerHUDWidget::SetRunTimerVisibility(ESlateVisibility V){
	Timer->SetVisibility(V);
}
