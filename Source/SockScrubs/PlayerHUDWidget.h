// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class SOCKSCRUBS_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* Timer{};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* AmmoCounter{};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ComboCounter{};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* AdrenalineBar{};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* ComboBar{};

public:
	void SetAmmoCounter(uint8 CurrentAmmo);

	void SetAmmoCounterVisibility(ESlateVisibility V);

	void SetAdrenalineBarPercent(float MaxHealth);

	void SetComboBarPercent(float Percent);

	void SetComboCounterVisibility(ESlateVisibility V);

	void SetComboCounterText(uint8 Combo);

	void SetRunTimerText(double time);

	void SetRunTimerVisibility(ESlateVisibility V);
	
};
