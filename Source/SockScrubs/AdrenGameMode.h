// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AdrenGameMode.generated.h"

/**
 * 
 */

DECLARE_DELEGATE_OneParam(GainAdrenalineDelegate, float)

DECLARE_DELEGATE_OneParam(OnSensUpdatedDelegate, float)


UCLASS()
class SOCKSCRUBS_API AAdrenGameMode : public AGameModeBase
{
	GENERATED_BODY()


public:

	AAdrenGameMode();

	GainAdrenalineDelegate GainAdrenalineDelegate{};
	
	OnSensUpdatedDelegate OnSensUpdatedDelegate{};

	class UAdrenSaveGame* LoadedSaveGame{};

	UPROPERTY(BlueprintReadWrite)
	bool CollectEnemies{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Run)
	bool bLevelIsRun{ true };

	UFUNCTION()
	void StartRun();

	UFUNCTION()
	void PauseGame();

	UFUNCTION()
	void OpenSettings();

	void BindEnemyEliminated(class ABaseEnemy* Enemy);

	virtual void Tick(float DeltaTime) override;

	void CheckForPlayer();

	void BindStartRunDelegate();

	void AddStartRunWidgetToScreen();

	FTimerHandle FindPlayerHandle{};

	bool DoOnce{ true };

	void ResetComboCount();

	UFUNCTION()
	void PassSensitivityToPlayer(float Value);

protected:

	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	float LoadedSensitivity{};

	UFUNCTION()
	void RetrieveLoadedData(const FString& SlotName, const int32 UserIndex, class USaveGame* LoadedGameData);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	class UBeginRunWidget* BeginRunWidget{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	class UPauseWidget* PauseWidget{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	class USettingsWidget* SettingsWidget{};

	class UPlayerHUDWidget* PlayerHUDWidget{};

	class AAdrenCharacter* Player{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* StartRunSound{};

	UPROPERTY(EditDefaultsOnly, Category = Enemy)
	TSubclassOf<class AActor> EnemyClass{};

	UPROPERTY(EditDefaultsOnly, Category = Enemy)
	TArray<class AActor*> EnemyArray{};

	void EnemyEliminated(class ABaseEnemy* Enemy, float HealthRegain);

	UPROPERTY(BlueprintReadOnly, Category = Stats)
	uint8 CurrentCombo{};

	UPROPERTY(BlueprintReadOnly, Category = Stats)
	uint8 HighestCombo{};

	UPROPERTY(BlueprintReadWrite, Category = Stats)
	uint8 NumEnemiesInLevel{};

	UPROPERTY(BlueprintReadWrite, Category = Stats)
	uint8 EnemiesRemainingInLevel{}; 

	UPROPERTY(BlueprintReadOnly, Category = Stats)
	bool UltraCombo{ false };
	
	FTimerHandle ComboResetHandle{};

	float ComboTimer{ 5.f };

	float MaxComboTime{ ComboTimer };

	UPROPERTY(BlueprintReadOnly, Category = Stats)
	double RunTimer{};

	bool bRunStarted{ false };

	
	public:
		FORCEINLINE float GetLoadedSensitivity() { return LoadedSensitivity; }
};
