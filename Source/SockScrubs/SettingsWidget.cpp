// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingsWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "AdrenSaveGame.h"
#include "Kismet/GameplayStatics.h"



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
	if (UAdrenSaveGame* SaveSettingsInstance = Cast<UAdrenSaveGame>(UGameplayStatics::CreateSaveGameObject(UAdrenSaveGame::StaticClass()))) {
		SaveSettingsInstance->PlayerSensitivity = Sens;
		if (UGameplayStatics::SaveGameToSlot(SaveSettingsInstance, SaveSettingsInstance->SaveSlotName, SaveSettingsInstance->UserIndex)) {
			GEngine->AddOnScreenDebugMessage(4, 5.f, FColor::Red, "SaveSuccess");
		}
	}
}

void USettingsWidget::FilterSensitivityText(const FText& Text, ETextCommit::Type CommitMethod){
	if (ETextCommit::OnEnter) {
		SenValue->SetText(Text);
		FString TextAsString = Text.ToString();
		float ConvertedFloat = FCString::Atof(*TextAsString);
		Slider->SetValue(ConvertedFloat);
		SetSensitivityValue();
	}
	
}

void USettingsWidget::Return(){
	RemoveFromParent();
}

void USettingsWidget::UpdateValueText(float Value){
	FString SliderString = FString::Printf(TEXT("%.2f"), Value);
	//FString SliderString = FString::SanitizeFloat(Value);
	FText SliderText = FText::FromString(SliderString);
	SenValue->SetText(SliderText);
}
