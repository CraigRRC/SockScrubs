// Fill out your copyright notice in the Description page of Project Settings.


#include "AdrenGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "AdrenSaveGame.h"


void UAdrenGameInstance::Init(){
	FAsyncLoadGameFromSlotDelegate LoadedDelegate{};

	LoadedDelegate.BindUObject(this, &UAdrenGameInstance::RetrieveLoadedData);

	UGameplayStatics::AsyncLoadGameFromSlot("SaveSlot1", 0, LoadedDelegate);
}

void UAdrenGameInstance::RetrieveLoadedData(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData){
	if (UAdrenSaveGame* AdrenSave = Cast<UAdrenSaveGame>(LoadedGameData)) {
		LoadedSensitivity = AdrenSave->PlayerSensitivity;
		GEngine->AddOnScreenDebugMessage(5, 5.f, FColor::Blue, FString::SanitizeFloat(LoadedSensitivity));

	}

}
