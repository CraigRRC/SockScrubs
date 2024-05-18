// Fill out your copyright notice in the Description page of Project Settings.

#include "AdrenCharacter.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


enum PlayerMovementState {
	Running,
	Crouching,
	Sliding,
	WallRunning,
};

// Sets default values
AAdrenCharacter::AAdrenCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MovementState = PlayerMovementState::Running;
	PlayerCapsule = GetCapsuleComponent();
	CapsuleHalfHeight = PlayerCapsule->GetScaledCapsuleHalfHeight();
	CrouchedCapsuleHalfHeight = CapsuleHalfHeight / 2;
	PlayerMovementComp = GetCharacterMovement();
}

// Called when the game starts or when spawned
void AAdrenCharacter::BeginPlay()
{
	Super::BeginPlay();
	MovementStateDelegate.BindUObject(this, &AAdrenCharacter::UpdateMovementState);
}

void AAdrenCharacter::Destroyed()
{
	MovementStateDelegate.Unbind();
}


void AAdrenCharacter::UpdateMovementState()
{
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Red, "Updated");
	switch (MovementState)
	{
	case Running:
		MaxPlayerSpeed = 950.f;
		break;
	case Crouching:
		MaxPlayerSpeed = 300.f;
		break;
	case Sliding:
		break;
	case WallRunning:
		break;
	default:
		break;
	}

	PlayerMovementComp->MaxWalkSpeed = MaxPlayerSpeed;
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
	Input->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
	Input->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	Input->BindAction(IA_Crouch, ETriggerEvent::Triggered, this, &AAdrenCharacter::Crouch);
	//Input->BindAction(IA_Crouch, ETriggerEvent::Completed, this, &AAdrenCharacter::UnCrouch);
}

void AAdrenCharacter::Look(const FInputActionInstance& Instance) {
	FVector2D AxisValue2D = Instance.GetValue().Get<FVector2D>();
	AddControllerYawInput(AxisValue2D.X);
	AddControllerPitchInput(AxisValue2D.Y);
}

void AAdrenCharacter::Crouch(const FInputActionInstance& Instance)
{
	if (PlayerCapsule->GetScaledCapsuleHalfHeight() == CapsuleHalfHeight) {
		MovementState = PlayerMovementState::Crouching;
		MovementStateDelegate.ExecuteIfBound();
		PlayerCapsule->SetCapsuleHalfHeight(CrouchedCapsuleHalfHeight);
	}
	else {
		MovementState = PlayerMovementState::Running;
		MovementStateDelegate.ExecuteIfBound();
		PlayerCapsule->SetCapsuleHalfHeight(CapsuleHalfHeight);
	}
}

void AAdrenCharacter::UnCrouch(const FInputActionInstance& Instance)
{
	
}

void AAdrenCharacter::Move(const FInputActionInstance& Instance) {
	FVector2D AxisValue2D = Instance.GetValue().Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), AxisValue2D.Y);
	AddMovementInput(GetActorRightVector(), AxisValue2D.X);
	
}

