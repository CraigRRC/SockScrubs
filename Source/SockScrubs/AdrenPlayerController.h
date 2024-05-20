// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AdrenPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SOCKSCRUBS_API AAdrenPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class UInputMappingContext* IMC_Default;

	UPROPERTY(EditAnywhere)
	class UInputMappingContext* IMC_WeaponEquipped;

public:
	void SwitchToDefaultMappingContext();
	void SwitchToWeaponEquippedMappingContext();
	
};
