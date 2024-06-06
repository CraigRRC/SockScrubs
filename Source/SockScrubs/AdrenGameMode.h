// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AdrenGameMode.generated.h"

/**
 * 
 */

DECLARE_DELEGATE_OneParam(GainAdrenalineDelegate, float)

UCLASS()
class SOCKSCRUBS_API AAdrenGameMode : public AGameModeBase
{
	GENERATED_BODY()


public:

	AAdrenGameMode();

	GainAdrenalineDelegate GainAdrenalineDelegate{};

	UFUNCTION()
	void StartRun();

	virtual void Tick(float DeltaTime) override;

	void CheckForPlayer();

	void BindStartRunDelegate();

	void AddStartRunWidgetToScreen();

	FTimerHandle FindPlayerHandle{};

	bool DoOnce{ true };

protected:

	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	class UBeginRunWidget* BeginRunWidget{};

	class UPlayerHUDWidget* PlayerHUDWidget{};

	class AAdrenCharacter* Player{};

	UPROPERTY(EditDefaultsOnly, Category = Enemy)
	TSubclassOf<class AActor> EnemyClass{};

	UPROPERTY(EditDefaultsOnly, Category = Enemy)
	TArray<class AActor*> EnemyArray{};

	void EnemyEliminated(class ABaseEnemy* Enemy, float HealthRegain);

	void ResetComboCount();

	uint8 CurrentCombo{};

	uint8 HighestCombo{};
	
	FTimerHandle ComboResetHandle{};

	float ComboTimer{ 4.f };

	float MaxComboTime{ ComboTimer };

	
};
