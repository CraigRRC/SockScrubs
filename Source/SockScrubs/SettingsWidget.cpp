// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingsWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void USettingsWidget::NativeConstruct(){
	Super::NativeConstruct();
	if (Slider) {
		Slider->OnValueChanged.AddDynamic(this, &USettingsWidget::UpdateValueText);
		if (SensValue) {
			FString SliderString = FString::SanitizeFloat(Slider->GetValue());
			FText SliderText = FText::FromString(SliderString);
			SensValue->SetText(SliderText);
		}
	}
	if (ApplyButton) {
		ApplyButton->OnClicked.AddDynamic(this, &USettingsWidget::SetSensitivityValue);
	}
	if (ReturnButton) {
		ReturnButton->OnClicked.AddDynamic(this, &USettingsWidget::Return);
	}

}

void USettingsWidget::NativeDestruct(){
	Super::NativeDestruct();
}

void USettingsWidget::SetSensitivityValue(){
	Sens = Slider->GetValue();
	OnSensUpdatedDelegate.ExecuteIfBound(Sens);
}

void USettingsWidget::Return(){
	RemoveFromParent();
}

void USettingsWidget::UpdateValueText(float Value){
	FString SliderString = FString::SanitizeFloat(Value);
	FText SliderText = FText::FromString(SliderString);
	SensValue->SetText(SliderText);
}
