// Fill out your copyright notice in the Description page of Project Settings.


#include "AdrenPlayerController.h"
#include "EnhancedInputSubsystems.h"

void AAdrenPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SwitchToDefaultMappingContext();
}

void AAdrenPlayerController::SwitchToDefaultMappingContext()
{
	if (GetLocalPlayer()) {
		UEnhancedInputLocalPlayerSubsystem* inputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (inputSubsystem && IMC_Default && IMC_WeaponEquipped ) {
			if (inputSubsystem->HasMappingContext(IMC_WeaponEquipped)) {
				inputSubsystem->RemoveMappingContext(IMC_WeaponEquipped);
			}
			inputSubsystem->AddMappingContext(IMC_Default, 0);
		}
	}
}

void AAdrenPlayerController::SwitchToWeaponEquippedMappingContext(){
	if (GetLocalPlayer()) {
		UEnhancedInputLocalPlayerSubsystem* inputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (inputSubsystem && IMC_WeaponEquipped) {
			inputSubsystem->AddMappingContext(IMC_WeaponEquipped, 1);
			GEngine->AddOnScreenDebugMessage(3, 4.f, FColor::Black, "Here");
			
		}
	}
}

