// Fill out your copyright notice in the Description page of Project Settings.


#include "AdrenGameMode.h"
#include "BeginRunWidget.h"
#include "AdrenCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BaseEnemy.h"

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
	GEngine->AddOnScreenDebugMessage(6, 3.f, FColor::Green, "Worked", false);
	Enemy->EnemyEliminatedDelegate.Unbind();
	GainAdrenalineDelegate.ExecuteIfBound(HealthRegain);
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
		AddStartRunWidgetToScreen();
		BindStartRunDelegate();
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), EnemyClass, EnemyArray);
		for (AActor* Enemy : EnemyArray) {
			ABaseEnemy* TempEnemy = Cast<ABaseEnemy>(Enemy);
			TempEnemy->EnemyEliminatedDelegate.BindUObject(this, &AAdrenGameMode::EnemyEliminated);
		}

		DoOnce = false;
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

