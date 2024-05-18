// Fill out your copyright notice in the Description page of Project Settings.


#include "AdrenPlayerController.h"
#include "EnhancedInputSubsystems.h"

void AAdrenPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (GetLocalPlayer()) {
		UEnhancedInputLocalPlayerSubsystem* inputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (inputSubsystem) {
			inputSubsystem->AddMappingContext(IMC_Default, 0);
		}
	}
}
