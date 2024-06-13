// Fill out your copyright notice in the Description page of Project Settings.


#include "AdrenCameraManager.h"
#include "AdrenCharacter.h"

AAdrenCameraManager::AAdrenCameraManager(){

}

void AAdrenCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime){
	Super::UpdateViewTarget(OutVT, DeltaTime);

	//Bugged
	/*if (AAdrenCharacter* AdrenCharacter = Cast<AAdrenCharacter>(GetOwningPlayerController()->GetPawn())) {
		FVector TargetCrouchOffset = FVector(0, 0, AdrenCharacter->GetCrouchingCapsuleHalfHeight() - AdrenCharacter->GetStandingCapsuleHalfHeight());
		FVector Offset = FMath::Lerp(FVector(0,0, AdrenCharacter->GetStandingCapsuleHalfHeight()), TargetCrouchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));

		if (AdrenCharacter->GetPlayerMovementState() >= 1) {
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
		}
		else {
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
		}
		OutVT.POV.Location += Offset;
	}*/
}
