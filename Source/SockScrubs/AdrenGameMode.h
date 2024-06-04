// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AdrenGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SOCKSCRUBS_API AAdrenGameMode : public AGameModeBase
{
	GENERATED_BODY()


public:

	AAdrenGameMode();

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

	class AAdrenCharacter* Player{};
	

	
};
