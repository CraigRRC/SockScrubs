// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void UPauseWidget::NativeConstruct(){
	Super::NativeConstruct();

	ResumeButton->OnClicked.AddDynamic(this, &UPauseWidget::OnResumeClicked);
	SettingsButton->OnClicked.AddDynamic(this, &UPauseWidget::OnSettingsClicked);
	QuitButton->OnClicked.AddDynamic(this, &UPauseWidget::OnQuitClicked);
}

void UPauseWidget::NativeDestruct(){
	ResumeButton->OnClicked.RemoveAll(this);
	SettingsButton->OnClicked.RemoveAll(this);
	QuitButton->OnClicked.RemoveAll(this);
}

void UPauseWidget::OnResumeClicked(){
	FInputModeGameOnly GameOnly{};
	GetOwningPlayer()->SetPause(false);
	GetOwningPlayer()->SetShowMouseCursor(false);
	GetOwningPlayer()->SetInputMode(GameOnly);
	RemoveFromParent();
}

void UPauseWidget::OnSettingsClicked(){
	OpenSettingsWidgetDelegate.ExecuteIfBound();
}

void UPauseWidget::OnQuitClicked(){
	NativeDestruct();
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
