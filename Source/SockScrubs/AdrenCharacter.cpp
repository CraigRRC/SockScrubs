// Fill out your copyright notice in the Description page of Project Settings.

#include "AdrenCharacter.h"
#include "AdrenCharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"


// Sets default values
AAdrenCharacter::AAdrenCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UAdrenCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AdrenCharacterMovementComponent = Cast<UAdrenCharacterMovementComponent>(GetCharacterMovement());

}

// Called when the game starts or when spawned
void AAdrenCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAdrenCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAdrenCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	Input->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AAdrenCharacter::Look);
	Input->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AAdrenCharacter::Move);

}

void AAdrenCharacter::Look(const FInputActionInstance& Instance) {
	FVector2D AxisValue2D = Instance.GetValue().Get<FVector2D>();
	AddControllerYawInput(AxisValue2D.X);
	AddControllerPitchInput(AxisValue2D.Y);
}

void AAdrenCharacter::Move(const FInputActionInstance& Instance) {
	FVector2D AxisValue2D = Instance.GetValue().Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), AxisValue2D.Y);
	AddMovementInput(GetActorRightVector(), AxisValue2D.X);
}

