// Fill out your copyright notice in the Description page of Project Settings.


#include "AdrenGameMode.h"
#include "BeginRunWidget.h"
#include "AdrenCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BaseEnemy.h"
#include "PlayerHUDWidget.h"

AAdrenGameMode::AAdrenGameMode(){
	PrimaryActorTick.bCanEverTick = true;
}


void AAdrenGameMode::BeginPlay(){
	Super::BeginPlay();
	if (BeginRunWidget != nullptr && GetWorld() != nullptr) {
		BeginRunWidget->SetOwningPlayer(GetWorld()->GetFirstPlayerController());
	}

}

void AAdrenGameMode::Destroyed(){
	Super::Destroyed();
	if (Player) {
		Player->StartRunDelegate.Unbind();
	}
}

void AAdrenGameMode::EnemyEliminated(ABaseEnemy* Enemy, float HealthRegain){
	Enemy->EnemyEliminatedDelegate.Unbind();
	GainAdrenalineDelegate.ExecuteIfBound(HealthRegain);
	CurrentCombo++;
	if (CurrentCombo > HighestCombo) {
		HighestCombo = CurrentCombo;
	}
	if (CurrentCombo > 1) {
		PlayerHUDWidget->SetComboCounterVisibility(ESlateVisibility::Visible);
		PlayerHUDWidget->SetComboCounterText(CurrentCombo);
	}
	GetWorldTimerManager().SetTimer(ComboResetHandle, this, &AAdrenGameMode::ResetComboCount, ComboTimer, false);

}

void AAdrenGameMode::ResetComboCount(){
	CurrentCombo = 0;
	PlayerHUDWidget->SetComboCounterVisibility(ESlateVisibility::Hidden);
}

void AAdrenGameMode::StartRun() {
	BeginRunWidget->RemoveFromParent();
	GetWorld()->GetFirstPlayerController()->SetPause(false);
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
		BindStartRunDelegate();
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), EnemyClass, EnemyArray);
		for (AActor* Enemy : EnemyArray) {
			ABaseEnemy* TempEnemy = Cast<ABaseEnemy>(Enemy);
			TempEnemy->EnemyEliminatedDelegate.BindUObject(this, &AAdrenGameMode::EnemyEliminated);
		}

		DoOnce = false;
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
	Player->StartRunDelegate.BindUObject(this, &AAdrenGameMode::StartRun);
}

void AAdrenGameMode::AddStartRunWidgetToScreen()
{
	BeginRunWidget->AddToPlayerScreen();
	GetWorld()->GetFirstPlayerController()->Pause();
}

