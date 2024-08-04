// Fill out your copyright notice in the Description page of Project Settings.


#include "AdrenGameMode.h"
#include "BeginRunWidget.h"
#include "AdrenCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BaseEnemy.h"
#include "PlayerHUDWidget.h"
#include "PauseWidget.h"
#include "SettingsWidget.h"
#include "AdrenGameInstance.h"
#include "AdrenSaveGame.h"

AAdrenGameMode::AAdrenGameMode(){
	PrimaryActorTick.bCanEverTick = true;
}


void AAdrenGameMode::BeginPlay(){
	Super::BeginPlay();

	if (BeginRunWidget != nullptr && GetWorld() != nullptr && bLevelIsRun) {
		BeginRunWidget->SetOwningPlayer(GetWorld()->GetFirstPlayerController());
	}

}

void AAdrenGameMode::Destroyed(){
	Super::Destroyed();
	if (Player) {
		Player->StartRunDelegate.Unbind();
		Player->PauseGameDelegate.Unbind();
	}
	if (PauseWidget) {
		if (PauseWidget->OpenSettingsWidgetDelegate.IsBoundToObject(this)) {
			PauseWidget->OpenSettingsWidgetDelegate.Unbind();
		}
	}
	
}

void AAdrenGameMode::EnemyEliminated(ABaseEnemy* Enemy, float HealthRegain){
	Enemy->EnemyEliminatedDelegate.Unbind();
	GainAdrenalineDelegate.ExecuteIfBound(HealthRegain);
	CurrentCombo++;
	EnemiesRemainingInLevel = FMath::Clamp(EnemiesRemainingInLevel - 1, 0, NumEnemiesInLevel);
	if (CurrentCombo > HighestCombo) {
		HighestCombo = CurrentCombo;
	}
	if (CurrentCombo > 1) {
		PlayerHUDWidget->SetComboCounterVisibility(ESlateVisibility::Visible);
		PlayerHUDWidget->SetComboCounterText(CurrentCombo);
	}
	if (HighestCombo == NumEnemiesInLevel) {
		UltraCombo = true;
	}
	GetWorldTimerManager().SetTimer(ComboResetHandle, this, &AAdrenGameMode::ResetComboCount, ComboTimer, false);

}

void AAdrenGameMode::ResetComboCount(){
	CurrentCombo = 0;
	PlayerHUDWidget->SetComboCounterVisibility(ESlateVisibility::Hidden);
}

void AAdrenGameMode::PassSensitivityToPlayer(float Value){
	OnSensUpdatedDelegate.ExecuteIfBound(Value);
}

void AAdrenGameMode::StartRun() {
	if (bLevelIsRun) {
		BeginRunWidget->RemoveFromParent();
		GetWorld()->GetFirstPlayerController()->SetPause(false);
		PlayerHUDWidget->SetRunTimerVisibility(ESlateVisibility::Visible);
		PlayerHUDWidget->SetHealthBarVisibility(ESlateVisibility::Visible);
		PlayerHUDWidget->SetSloMoBarVisibility(ESlateVisibility::Visible);
		PlayerHUDWidget->SetCrosshairVisibility(ESlateVisibility::Visible);
		bRunStarted = true;
	}
	
}

void AAdrenGameMode::PauseGame(){
	if (PauseWidget != nullptr && GetWorld() != nullptr) {
		FInputModeUIOnly UIOnly{};
		PauseWidget->SetOwningPlayer(GetWorld()->GetFirstPlayerController());
		PauseWidget->AddToPlayerScreen();
		if (!PauseWidget->OpenSettingsWidgetDelegate.IsBoundToObject(this)) {
			PauseWidget->OpenSettingsWidgetDelegate.BindUObject(this, &AAdrenGameMode::OpenSettings);
		}
		GetWorld()->GetFirstPlayerController()->SetPause(true);
		GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
		GetWorld()->GetFirstPlayerController()->SetInputMode(UIOnly);
	}

}

void AAdrenGameMode::OpenSettings(){
	if (SettingsWidget != nullptr && GetWorld() != nullptr) {
		SettingsWidget->SetOwningPlayer(GetWorld()->GetFirstPlayerController());
		SettingsWidget->AddToPlayerScreen();
		SettingsWidget->OnSensUpdatedDelegate.BindUObject(this, &AAdrenGameMode::PassSensitivityToPlayer);
	}
}

void AAdrenGameMode::BindEnemyEliminated(ABaseEnemy* Enemy){
	Enemy->EnemyEliminatedDelegate.BindUObject(this, &AAdrenGameMode::EnemyEliminated);
}

void AAdrenGameMode::Tick(float DeltaTime){
	if (!Player) {
		if (!GetWorldTimerManager().IsTimerActive(FindPlayerHandle)) {
			GetWorldTimerManager().SetTimer(FindPlayerHandle, this, &AAdrenGameMode::CheckForPlayer, 0.1f, false);
		}
	}
	else if (Player && DoOnce) {
		PlayerHUDWidget = Player->HUDWidget;
		AddStartRunWidgetToScreen();
		if (StartRunSound) {
			UGameplayStatics::PlaySound2D(GetWorld(), StartRunSound);
		}
		BindStartRunDelegate();
		Player->PauseGameDelegate.BindUObject(this, &AAdrenGameMode::PauseGame);
		UAdrenSaveGame* SavedGame = Cast<UAdrenSaveGame>(UGameplayStatics::CreateSaveGameObject(UAdrenSaveGame::StaticClass()));
		LoadedSaveGame = Cast<UAdrenSaveGame>(UGameplayStatics::LoadGameFromSlot(SavedGame->SaveSlotName, SavedGame->UserIndex));
		if (UAdrenGameInstance* GameInstance = Cast<UAdrenGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))) {
			/*if (GameInstance->LoadedSensitivity > 0) {
				Player->SetPlayerSensitivity(GameInstance->LoadedSensitivity);
			}*/
			if (LoadedSaveGame->PlayerSensitivity > 0) {
				Player->SetPlayerSensitivity(LoadedSaveGame->PlayerSensitivity);
			}
			else {
				Player->SetPlayerSensitivity(1.0f);
			}
		}
		
		DoOnce = false;
	}

	if (bRunStarted) {
		RunTimer = GetWorld()->GetTimeSeconds();
		PlayerHUDWidget->SetRunTimerText(RunTimer);
	}

	if (CurrentCombo > 1) {
		float CurrentComboTime = GetWorldTimerManager().GetTimerRemaining(ComboResetHandle);
		PlayerHUDWidget->SetComboBarPercent(CurrentComboTime / MaxComboTime);
	}
}

void AAdrenGameMode::CheckForPlayer()
{
	Player = Cast<AAdrenCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
}

void AAdrenGameMode::BindStartRunDelegate()
{
	if (bLevelIsRun) {
		Player->StartRunDelegate.BindUObject(this, &AAdrenGameMode::StartRun);
	}
}
	

void AAdrenGameMode::AddStartRunWidgetToScreen()
{
	if (bLevelIsRun) {
		BeginRunWidget->AddToPlayerScreen();
		GetWorld()->GetFirstPlayerController()->Pause();
	}
}

