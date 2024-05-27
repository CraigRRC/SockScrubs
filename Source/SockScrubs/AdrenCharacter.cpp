// Fill out your copyright notice in the Description page of Project Settings.

#include "AdrenCharacter.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "BaseWeapon.h"
#include "AdrenPlayerController.h"
#include "Camera/CameraShakeSourceComponent.h"




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
	FireCameraShake = CreateDefaultSubobject<UCameraShakeSourceComponent>(TEXT("FireCameraShakeSource"));
	SlideCameraShake = CreateDefaultSubobject<UCameraShakeSourceComponent>(TEXT("SlideCameraShakeSource"));
	
}

// Called when the game starts or when spawned
void AAdrenCharacter::BeginPlay()
{
	Super::BeginPlay();
	AdrenPlayerController = CastChecked<AAdrenPlayerController>(GetLocalViewingPlayerController());
	MovementStateDelegate.BindUObject(this, &AAdrenCharacter::UpdateMovementState);
	CamManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
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
			CamManager->StopAllCameraShakesFromSource(SlideCameraShake, false);
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
			bCanFire = true;
			PlayerWeaponStatus = EPlayerWeaponState::HasRifle;
			EquippedWeapon->GetGunMesh()->SetSimulatePhysics(false);
			EquippedWeapon->GetGunMesh()->AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, "GripPoint");
			EquippedWeapon->GetGunMesh()->CastShadow = false;
			EquippedWeapon->GetPickupCollider()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			EquippedWeapon->GetStunCollider()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Ammo = EquippedWeapon->GetClipSize();
			FireCameraShake->CameraShake = EquippedWeapon->GetCameraShakeBase().Get();
			GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, EquippedWeapon->GetCameraShakeBase().Get()->GetName());
			FullAutoTriggerCooldown = EquippedWeapon->GetFireRate();
			AdrenPlayerController->SwitchToWeaponEquippedMappingContext();
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
	CamManager->StartCameraShake(SlideCameraShake->CameraShake, 1.0f);
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
	Input->BindAction(IA_Shoot, ETriggerEvent::Ongoing, this, &AAdrenCharacter::ShootFullAuto);
	Input->BindAction(IA_Shoot, ETriggerEvent::Triggered, this, &AAdrenCharacter::ShootFullAuto);
	Input->BindAction(IA_Shoot, ETriggerEvent::Canceled, this, &AAdrenCharacter::FinishShootingFullAuto);
	Input->BindAction(IA_Shoot, ETriggerEvent::Completed, this, &AAdrenCharacter::FinishShootingFullAuto);
	Input->BindAction(IA_Throw, ETriggerEvent::Triggered, this, &AAdrenCharacter::Throw);
}

void AAdrenCharacter::Look(const FInputActionInstance& Instance) {
	FVector2D AxisValue2D = Instance.GetValue().Get<FVector2D>();
	AddControllerYawInput(AxisValue2D.X);
	AddControllerPitchInput(AxisValue2D.Y);
}

void AAdrenCharacter::Throw(const FInputActionInstance& Instance){
	if (EquippedWeapon == nullptr) return;
	//Unsocket
	EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//Simulate physics on it if not already simulating
	EquippedWeapon->GetGunMesh()->SetSimulatePhysics(true);
	//Tell the anim instance that we don't have a weapon.
	PlayerWeaponStatus = EPlayerWeaponState::Unarmed;
	//Maybe move it to the front
	//Switch mapping context back to default.
	//Add impulse to it
	FVector Throw = (PlayerCam->GetForwardVector() + PlayerCam->GetUpVector() * 0.1f) * 3000.f;
	EquippedWeapon->GetGunMesh()->AddImpulse(Throw, NAME_None, true);
	//Turn back on the stun collider
	EquippedWeapon->GetStunCollider()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EquippedWeapon->GetPickupCollider()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//Turn back on the pickup collider after a bit.
	//Nullify EquippedWeapon after a bit.
	CamManager->StopAllCameraShakes(true);
	FTimerHandle UnequipTimerHandle{};
	GetWorldTimerManager().SetTimer(UnequipTimerHandle, this, &AAdrenCharacter::UnequipWeapon, 0.2f, false);
}

void AAdrenCharacter::UnequipWeapon(){
	EquippedWeapon = nullptr;
}

void AAdrenCharacter::ShootFullAuto(const FInputActionInstance& Instance) {
	if (!bCanFire || PlayerWeaponStatus == EPlayerWeaponState::Unarmed) return;
	EquippedWeapon->FireAsLineTrace(PlayerCam->GetComponentLocation(), PlayerCam->GetComponentLocation() + PlayerCam->GetForwardVector() * 5000.f);
	PlayerMesh->GetAnimInstance()->Montage_Play(FireMontage);
	CamManager->StartCameraShake(FireCameraShake->CameraShake, 1.0f);
	Ammo--;
	GetWorldTimerManager().SetTimer(WeaponHandle, this, &AAdrenCharacter::ResetTrigger, FullAutoTriggerCooldown, false);
	bCanFire = false;

}

void AAdrenCharacter::FinishShootingFullAuto(const FInputActionInstance& Instance){
	CamManager->StopAllCameraShakes(false);
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Red, TEXT("Hey"));
}

void AAdrenCharacter::ResetTrigger(){
	
	bCanFire = true;
	if (Ammo == 0) {
		bCanFire = false;
		CamManager->StopAllCameraShakes(false);
	}
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

