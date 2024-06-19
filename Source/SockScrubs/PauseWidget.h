// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseWidget.generated.h"

DECLARE_DELEGATE(OpenSettingsWidgetDelegate)

/**
 * 
 */
UCLASS()
class SOCKSCRUBS_API UPauseWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ResumeButton{};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SettingsButton{};

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* QuitButton{};

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

public:
	OpenSettingsWidgetDelegate OpenSettingsWidgetDelegate{};

	UFUNCTION()
	void OnResumeClicked();
	UFUNCTION()
	void OnSettingsClicked();
	UFUNCTION()
	void OnQuitClicked();

};
