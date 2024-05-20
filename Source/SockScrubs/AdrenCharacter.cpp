// Fill out your copyright notice in the Description page of Project Settings.

#include "AdrenCharacter.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "BaseWeapon.h"



// Sets default values
AAdrenCharacter::AAdrenCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MovementState = EPlayerMovementState::Running;
	PlayerCapsule = GetCapsuleComponent();
	CapsuleHalfHeight = PlayerCapsule->GetScaledCapsuleHalfHeight();
	CrouchedCapsuleHalfHeight = CapsuleHalfHeight / 2;
	PlayerMovementComp = GetCharacterMovement();
	PlayerMesh = GetMesh();
	PlayerCam = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCam"));
	PlayerCam->SetupAttachment(RootComponent);
	PlayerMesh->SetupAttachment(PlayerCam);
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
		PlayerMovementComp->GroundFriction = 8.f;
		break;
	case Crouching:
		MaxPlayerSpeed = 500.f;
		PlayerMovementComp->GroundFriction = 8.f;
		break;
	case Sliding:
		PlayerMovementComp->GroundFriction = 0.f;
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
	//GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Red, FString::Printf(TEXT("State: %f"), GetVelocity().SquaredLength()));
	if (MovementState == EPlayerMovementState::Crouching && GetVelocity().SquaredLength() > 400000.f && PlayerMovementComp->IsMovingOnGround()) {
		StartSlide();
	}

	if (MovementState == EPlayerMovementState::Sliding) {
		CalcFloorInfluence();
		//Check if the velocity is greater than our crouch max speed squared.
		if (GetVelocity().SquaredLength() < 250000.f) {
			StopCrouching();
		}
	}
}

void AAdrenCharacter::PickupWeapon(AActor* Weapon, WeaponType WeaponType)
{
	if (EquippedWeapon) return;

	if(Weapon != nullptr){
		EquippedWeapon = Cast<ABaseWeapon>(Weapon);
		
		switch (WeaponType)
		{
		case WeaponType::Rifle:
			PlayerWeaponStatus = EPlayerWeaponState::HasRifle;
			EquippedWeapon->GetGunMesh()->AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, "GripPoint");
			break;
		default:
			break;
		}
	}

	
}

void AAdrenCharacter::StartSlide()
{
	FVector SlideImpulse = GetVelocity() * SlideImpulseForce;
	PlayerMovementComp->AddImpulse(SlideImpulse);
	MovementState = EPlayerMovementState::Sliding;
	MovementStateDelegate.ExecuteIfBound();
}

void AAdrenCharacter::CalcFloorInfluence()
{
	FVector FirstCross = FVector::CrossProduct(PlayerMovementComp->CurrentFloor.HitResult.Normal, FVector::UpVector);
	FVector DirectionToAddForce = FVector::CrossProduct(PlayerMovementComp->CurrentFloor.HitResult.Normal, FirstCross).GetSafeNormal();
	if (FirstCross.IsNearlyZero()) {
		DirectionToAddForce = FVector::Zero();
	}
	ClampSlideVelocity();
	PlayerMovementComp->AddForce(DirectionToAddForce * DownhillForce);

}

void AAdrenCharacter::ClampSlideVelocity()
{
	if (GetVelocity().SquaredLength() > MaxSlideSpeed) {
		DownhillForce = 0.f;
	}
	else {
		DownhillForce = 400000.f;
	}
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
	Input->BindAction(IA_Crouch, ETriggerEvent::Triggered, this, &AAdrenCharacter::WantsToCrouch);
}

void AAdrenCharacter::Look(const FInputActionInstance& Instance) {
	FVector2D AxisValue2D = Instance.GetValue().Get<FVector2D>();
	AddControllerYawInput(AxisValue2D.X);
	AddControllerPitchInput(AxisValue2D.Y);
}

void AAdrenCharacter::WantsToCrouch(const FInputActionInstance& Instance)
{
	if (PlayerCapsule->GetScaledCapsuleHalfHeight() == CapsuleHalfHeight) {
		BeginCrouch();
		
	}
	else {
		StopCrouching();
	}
}

void AAdrenCharacter::BeginCrouch()
{
	MovementState = EPlayerMovementState::Crouching;
	MovementStateDelegate.ExecuteIfBound();
	PlayerCapsule->SetCapsuleHalfHeight(CrouchedCapsuleHalfHeight);
}

void AAdrenCharacter::StopCrouching(){
	MovementState = EPlayerMovementState::Running;
	MovementStateDelegate.ExecuteIfBound();
	PlayerCapsule->SetCapsuleHalfHeight(CapsuleHalfHeight);
}

void AAdrenCharacter::Move(const FInputActionInstance& Instance) {
	FVector2D AxisValue2D = Instance.GetValue().Get<FVector2D>();
	AddMovementInput(GetActorRightVector(), AxisValue2D.X);
	//Dont take any forward or backward input when sliding.
	if (MovementState == EPlayerMovementState::Sliding) return;
	AddMovementInput(GetActorForwardVector(), AxisValue2D.Y);
	
}

