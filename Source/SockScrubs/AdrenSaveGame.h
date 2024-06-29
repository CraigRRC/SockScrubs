// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AdrenSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class SOCKSCRUBS_API UAdrenSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UAdrenSaveGame();

	//Convert to struct later.
	UPROPERTY(VisibleAnywhere, Category = Basic)
	float PlayerSensitivity{};

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName{};

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex{};
	
};
