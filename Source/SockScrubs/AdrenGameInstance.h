// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AdrenGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SOCKSCRUBS_API UAdrenGameInstance : public UGameInstance
{
	GENERATED_BODY()

protected:
	virtual void Init() override;

	UFUNCTION()
	void RetrieveLoadedData(const FString& SlotName, const int32 UserIndex, class USaveGame* LoadedGameData);


public:
	float LoadedSensitivity{};
};
