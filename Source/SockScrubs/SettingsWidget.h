// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

DECLARE_DELEGATE_OneParam(OnSensUpdatedDelegate, float)

DECLARE_DELEGATE(OnReturnClickedDelegate)

/**
 * 
 */
UCLASS()
class SOCKSCRUBS_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	float TempSens{};

	float TempSliderValue{};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ApplyButton{};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ReturnButton{};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* Slider{};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UEditableTextBox* SenValue{};

	float Sens{1.0f};

public:
	OnSensUpdatedDelegate OnSensUpdatedDelegate;

	OnReturnClickedDelegate OnReturnClickedDelegate;

	UFUNCTION()
	void SetSensitivityValue();

	UFUNCTION()
	void FilterSensitivityText(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void Return();

	UFUNCTION()
	void UpdateValueText(float Value);

};
