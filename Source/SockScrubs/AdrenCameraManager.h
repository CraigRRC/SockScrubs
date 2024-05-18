// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "AdrenCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class SOCKSCRUBS_API AAdrenCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float CrouchBlendDuration{.2f};

	float CrouchBlendTime{};

public:
	AAdrenCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
};
