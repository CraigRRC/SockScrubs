// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingsWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"



void USettingsWidget::NativeConstruct(){
	Super::NativeConstruct();
	if (Slider) {
		Slider->OnValueChanged.AddDynamic(this, &USettingsWidget::UpdateValueText);
		if (SenValue) {
			UpdateValueText(Sens);
			SenValue->OnTextCommitted.AddDynamic(this, &USettingsWidget::FilterSensitivityText);
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

void USettingsWidget::FilterSensitivityText(const FText& Text, ETextCommit::Type CommitMethod){
	
}

void USettingsWidget::Return(){
	RemoveFromParent();
}

void USettingsWidget::UpdateValueText(float Value){
	FString SliderString = FString::SanitizeFloat(Value);
	FText SliderText = FText::FromString(SliderString);
	SenValue->SetText(SliderText);
}
