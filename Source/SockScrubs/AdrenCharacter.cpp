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
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerHUDWidget.h"
#include "AdrenGameMode.h"


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
	KickHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("TempKickHitbox"));
	KickHitbox->SetupAttachment(PlayerCam);
	KickHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FireCameraShake = CreateDefaultSubobject<UCameraShakeSourceComponent>(TEXT("FireCameraShakeSource"));
	SlideCameraShake = CreateDefaultSubobject<UCameraShakeSourceComponent>(TEXT("SlideCameraShakeSource"));
	PlayerCameraShake = CreateDefaultSubobject<UCameraShakeSourceComponent>(TEXT("PlayerCameraShakeSource"));
	KickHitbox->OnComponentBeginOverlap.AddDynamic(this, &AAdrenCharacter::OnKickHitboxBeginOverlap);

}



// Called when the game starts or when spawned
void AAdrenCharacter::BeginPlay()
{
	Super::BeginPlay();
	AdrenPlayerController = CastChecked<AAdrenPlayerController>(GetLocalViewingPlayerController());
	MovementStateDelegate.BindUObject(this, &AAdrenCharacter::UpdateMovementState);
	CamManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	HUDWidget->SetOwningPlayer(AdrenPlayerController);
	HUDWidget->AddToPlayerScreen();
	HUDWidget->SetAmmoCounterVisibility(ESlateVisibility::Hidden);
	HUDWidget->SetAdrenalineBarPercent(ConvertHealthToPercent(Health));
	AGameModeBase* BaseGameMode = UGameplayStatics::GetGameMode(GetWorld());
	GameMode = Cast<AAdrenGameMode>(BaseGameMode);
	GameMode->GainAdrenalineDelegate.BindUObject(this, &AAdrenCharacter::GainLife);
	GameMode->OnSensUpdatedDelegate.BindUObject(this, &AAdrenCharacter::UpdateSensitivity);
}

void AAdrenCharacter::Destroyed()
{
	Super::Destroyed();
	if (MovementStateDelegate.IsBound()) {
		MovementStateDelegate.Unbind();
	}
	if (GameMode != nullptr) {
		if (GameMode->GainAdrenalineDelegate.IsBound()) {
			GameMode->GainAdrenalineDelegate.Unbind();
		}
	}
}

void AAdrenCharacter::Jump(){
	if (MovementState == EPlayerMovementState::WallRunning) {
		Super::Jump();
		GetWorld()->LineTraceSingleByChannel(LeftOfPlayerHit, GetActorLocation() + FVector::UpVector * 25.f, GetActorLocation() + GetActorRightVector() * -50.f, ECollisionChannel::ECC_Camera);
		GetWorld()->LineTraceSingleByChannel(RightOfPlayerHit, GetActorLocation() + FVector::UpVector * 25.f, GetActorLocation() + GetActorRightVector() * 50.f, ECollisionChannel::ECC_Camera);
		if (LeftOfPlayerHit.bBlockingHit) {
			
			PlayerMovementComp->AddImpulse((LeftOfPlayerHit.Normal + GetActorForwardVector() + FVector::UpVector * 2.f) * WallJumpForce , true);
			GetWorldTimerManager().ClearTimer(WallRunningHandle);
		}
		if (RightOfPlayerHit.bBlockingHit) {

			PlayerMovementComp->AddImpulse((RightOfPlayerHit.Normal + GetActorForwardVector() + FVector::UpVector * 2.f) * WallJumpForce, true);
			GetWorldTimerManager().ClearTimer(WallRunningHandle);
		}
		
		FellOffWall();
	}
	else {
		if (MovementState == EPlayerMovementState::Sliding || MovementState == EPlayerMovementState::Crouching) {
			StopCrouching();
		}
		Super::Jump();

		if (!PlayerMovementComp->IsMovingOnGround()) {
			
			GetWorld()->LineTraceSingleByChannel(LeftOfPlayerHit, GetActorLocation() + FVector::UpVector * 25.f, GetActorLocation() + GetActorRightVector() * -50.f, ECollisionChannel::ECC_Camera);
			GetWorld()->LineTraceSingleByChannel(RightOfPlayerHit, GetActorLocation() + FVector::UpVector * 25.f, GetActorLocation() + GetActorRightVector() * 50.f, ECollisionChannel::ECC_Camera);
			if (LeftOfPlayerHit.bBlockingHit) {
				PlayerMovementComp->AddImpulse(GetActorRightVector() * -800.f, true);
				PlayerMovementComp->AddImpulse(GetActorForwardVector() * 1000.f, true);
				MovementState = EPlayerMovementState::WallRunning;
				MovementStateDelegate.ExecuteIfBound();
				PlayerMovementComp->AddImpulse(FVector::UpVector * 250.f, true);
				GetWorldTimerManager().SetTimer(WallRunningHandle, this, &AAdrenCharacter::FellOffWall, WallRunningDuration, false);
				
			}
			if (RightOfPlayerHit.bBlockingHit) {
				PlayerMovementComp->AddImpulse(GetActorRightVector() * 800.f, true);
				PlayerMovementComp->AddImpulse(GetActorForwardVector() * 1000.f, true);
				PlayerMovementComp->AddImpulse(GetActorForwardVector() * 0.5f, true);
				MovementState = EPlayerMovementState::WallRunning;
				MovementStateDelegate.ExecuteIfBound();
				PlayerMovementComp->AddImpulse(FVector::UpVector * 250.f, true);
				GetWorldTimerManager().SetTimer(WallRunningHandle, this, &AAdrenCharacter::FellOffWall, WallRunningDuration, false);
			}
			
		}
	}
}

void AAdrenCharacter::FellOffWall() {
	MovementState = EPlayerMovementState::Running;
	MovementStateDelegate.ExecuteIfBound();
}

void AAdrenCharacter::UpdateMovementState()
{
	//GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Red, "Updated");
	
	switch (MovementState)
	{
	case Running:
		MaxPlayerSpeed = 950.f;
		PlayerMovementComp->GroundFriction = 8.f;
		PlayerMovementComp->GravityScale = 3.f;
		PlayerCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);
		StopKicking();
		break;
	case Crouching:
		MaxPlayerSpeed = 300.f;
		CrouchSpeedSquared = MaxPlayerSpeed * MaxPlayerSpeed + 5.f;
		PlayerMovementComp->GroundFriction = 8.f;
		PlayerMovementComp->GravityScale = 3.f;
		PlayerCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);
		StopKicking();
		break;
	case Sliding:
		PlayerMovementComp->GroundFriction = 0.f;
		PlayerMovementComp->GravityScale = 3.f;
		PlayerCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);
		EnableKickHitbox();
		break;
	case Dashing:
		PlayerMovementComp->GroundFriction = 8.f;
		PlayerMovementComp->GravityScale = 4.0f;
		PlayerCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
		EnableKickHitbox();
		break;
	case WallRunning:
		PlayerMovementComp->GravityScale = 0;
		PlayerMovementComp->Velocity.Z = 0;
		PlayerCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);
		StopKicking();
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

	DrainLife(ShouldDrainHealth, DeltaTime);

	if (GetVelocity().SizeSquared2D() > MaxSpeed) {
		PlayerMovementComp->Velocity = GetVelocity().GetClampedToMaxSize2D(2500);
	}

	if (PlayerMovementComp->IsMovingOnGround() || MovementState == EPlayerMovementState::WallRunning) {
		bCanDash = true;
	}

	if (PlayerMovementComp->IsMovingOnGround()) {
		if (EndDashOnce) {
			EndDash();
			EndDashOnce = false;
		}
	}


	if (!PlayerMovementComp->IsMovingOnGround()) {
		EndDashOnce = true;
	}
	
	if (MovementState == EPlayerMovementState::WallRunning) {
		if (GetWorldTimerManager().IsTimerActive(WallRunningHandle)) {
			if (GetWorldTimerManager().GetTimerElapsed(WallRunningHandle) <= WallRunningDuration / 1.5) {
				PlayerMovementComp->GravityScale = 0.3;
			}
		}

		if (LeftOfPlayerHit.bBlockingHit) {
			GetWorld()->LineTraceSingleByChannel(LeftOfPlayerHit, GetActorLocation() + FVector::UpVector * 25.f, GetActorLocation() + GetActorRightVector() * -50.f, ECollisionChannel::ECC_Camera);
			if (LeftOfPlayerHit.bBlockingHit) {
				PlayerMovementComp->AddForce(LeftOfPlayerHit.Normal * -500000.f);
			}
			else{
				GetWorldTimerManager().ClearTimer(WallRunningHandle);
				PlayerMovementComp->AddImpulse((LeftOfPlayerHit.Normal + GetActorForwardVector() + FVector::UpVector * 2.f) * WallJumpForce, true);
				FellOffWall();
			}
		}
		else if (RightOfPlayerHit.bBlockingHit) {
			GetWorld()->LineTraceSingleByChannel(RightOfPlayerHit, GetActorLocation() + FVector::UpVector * 25.f, GetActorLocation() + GetActorRightVector() * 50.f, ECollisionChannel::ECC_Camera);
			if (RightOfPlayerHit.bBlockingHit) {
				PlayerMovementComp->AddForce(RightOfPlayerHit.Normal * -500000.f);
			}
			else {
				GetWorldTimerManager().ClearTimer(WallRunningHandle);
				PlayerMovementComp->AddImpulse((RightOfPlayerHit.Normal + GetActorForwardVector() + FVector::UpVector * 2.f) * WallJumpForce, true);
				FellOffWall();
			}
		}
		
		
	}

	if (bSloMo) {
		SloMo = FMath::Clamp(SloMo - DeltaTime, 0, SloMo);
		HUDWidget->SetSloMoBarPercent(ConvertSloMoToPercent(SloMo));
		if (SloMo == 0.f) {
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
			CustomTimeDilation = 1.0f;
			bCanGenerateSloMo = true;
			bSloMo = false;
		}
	}

	if (bCanGenerateSloMo) {
		//10 seconds
		SloMo = FMath::Clamp(SloMo + DeltaTime / 5, 0, MaxSloMo);
		HUDWidget->SetSloMoBarPercent(ConvertSloMoToPercent(SloMo));
		if (SloMo == MaxSloMo) {
			bCanGenerateSloMo = false;
		}
	}

	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Red, FString::Printf(TEXT("State: %d"), MovementState));
	/*if (MovementState == EPlayerMovementState::Crouching && GetVelocity().SquaredLength() > CrouchSpeedSquared && PlayerMovementComp->IsMovingOnGround()) {
		StartSlide();
	}*/

	if (MovementState == EPlayerMovementState::Sliding) {
		CalcFloorInfluence();
		//Check if the velocity is greater than our crouch max speed squared.
		if (GetVelocity().SquaredLength() < CrouchSpeedSquared) {
			StopSliding();
		}
	}
}

void AAdrenCharacter::StopSliding()
{
	CamManager->StopAllCameraShakesFromSource(SlideCameraShake, false);
	if (MovementState == EPlayerMovementState::Dashing) return;
	StopCrouching();
	
}

void AAdrenCharacter::PickupWeapon(AActor* Weapon, WeaponType WeaponType)
{
	if (EquippedWeapon) return;

	if(Weapon != nullptr){
		EquippedWeapon = Cast<ABaseWeapon>(Weapon);
		EquippedWeapon->SetOwningActor(this);
		EquippedWeapon->PlayPickupSound();
		if (HUDWidget) {
			HUDWidget->SetAmmoCounterVisibility(ESlateVisibility::Visible);
		}

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
			EquippedWeapon->ResetLifeTime();
			Ammo = EquippedWeapon->GetClipSize();
			if (HUDWidget) {
				HUDWidget->SetAmmoCounter(Ammo);
			}
			FireCameraShake->CameraShake = EquippedWeapon->GetCameraShakeBase().Get();
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
	
	PlayerMovementComp->AddForce(DirectionToAddForce * DownhillForce);
}


// Called to bind functionality to input
void AAdrenCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	Input->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AAdrenCharacter::Look);
	Input->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AAdrenCharacter::Move);
	Input->BindAction(IA_Jump, ETriggerEvent::Started, this, &AAdrenCharacter::Jump);
	Input->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	Input->BindAction(IA_Shoot, ETriggerEvent::Ongoing, this, &AAdrenCharacter::ShootFullAuto);
	Input->BindAction(IA_Shoot, ETriggerEvent::Triggered, this, &AAdrenCharacter::ShootFullAuto);
	Input->BindAction(IA_Shoot, ETriggerEvent::Canceled, this, &AAdrenCharacter::FinishShootingFullAuto);
	Input->BindAction(IA_Shoot, ETriggerEvent::Completed, this, &AAdrenCharacter::FinishShootingFullAuto);
	Input->BindAction(IA_Shoot, ETriggerEvent::Started, this, &AAdrenCharacter::ThrowWhenEmpty);
	Input->BindAction(IA_Throw, ETriggerEvent::Triggered, this, &AAdrenCharacter::Throw);
	Input->BindAction(IA_Kick, ETriggerEvent::Triggered, this, &AAdrenCharacter::Kick);
	Input->BindAction(IA_AirDash, ETriggerEvent::Triggered, this, &AAdrenCharacter::AirDash);
	Input->BindAction(IA_StartRun, ETriggerEvent::Triggered, this, &AAdrenCharacter::StartRun);
	Input->BindAction(IA_ActivateSloMo, ETriggerEvent::Triggered, this, &AAdrenCharacter::ActivateSloMo);
	Input->BindAction(IA_Restart, ETriggerEvent::Triggered, this, &AAdrenCharacter::PlayerDie);
	Input->BindAction(IA_Slide, ETriggerEvent::Ongoing, this, &AAdrenCharacter::WantsToCrouch);
	Input->BindAction(IA_Slide, ETriggerEvent::Canceled, this, &AAdrenCharacter::StopSliding);
	Input->BindAction(IA_Slide, ETriggerEvent::Completed, this, &AAdrenCharacter::StopSliding);
	Input->BindAction(IA_Pause, ETriggerEvent::Triggered, this, &AAdrenCharacter::PauseGame);
}

void AAdrenCharacter::DamageTaken(bool Stun, float DamageDelta, AActor* DamageDealer, FVector ImpactPoint, FName BoneName, bool Headshot, bool Tripped, bool Kicked) {
	GameMode->ResetComboCount();
	Health -= DamageDelta;
	float ClampedHealth = FMath::Clamp(Health, 0.f, MaxHealth);
	HUDWidget->SetAdrenalineBarPercent(ConvertHealthToPercent(ClampedHealth));
	if (ClampedHealth <= 0.f) {
		PlayerDie();
	}
	else {
		if (PlayerCameraShake->CameraShake) {
			CamManager->StartCameraShake(PlayerCameraShake->CameraShake, 1.0f);
		}
		
	}
}

void AAdrenCharacter::PlayerDie()
{
	FString LevelString = UGameplayStatics::GetCurrentLevelName(GetWorld());
	FName LevelName = FName(LevelString);
	UGameplayStatics::OpenLevel(GetWorld(), LevelName);
	//Trigger Event that tells the GameMode that the player died.
}

void AAdrenCharacter::Look(const FInputActionInstance& Instance) {
	FVector2D AxisValue2D = Instance.GetValue().Get<FVector2D>();
	AddControllerYawInput(AxisValue2D.X * Sensitivity);
	AddControllerPitchInput(AxisValue2D.Y * Sensitivity);
}

void AAdrenCharacter::AirDash(const FInputActionInstance& Instance){
	if (!PlayerMovementComp->IsMovingOnGround() && bCanDash) {
		PlayerMovementComp->Velocity = FVector::ZeroVector;
		PlayerMovementComp->AddImpulse((PlayerCam->GetForwardVector() + FVector::UpVector * 0.2f) * DashImpulseForce);
		MovementState = EPlayerMovementState::Dashing;
		MovementStateDelegate.ExecuteIfBound();
		bCanDash = false;
	}
}

void AAdrenCharacter::Throw(const FInputActionInstance& Instance){
	if (EquippedWeapon == nullptr) return;
	//Unsocket
	if (HUDWidget) {
		HUDWidget->SetAmmoCounterVisibility(ESlateVisibility::Hidden);
	}
	EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	EquippedWeapon->SetLifeTimer();
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
	AdrenPlayerController->SwitchToDefaultMappingContext();
	
	CamManager->StopAllCameraShakes(true);
	FTimerHandle UnequipTimerHandle{};
	GetWorldTimerManager().SetTimer(UnequipTimerHandle, this, &AAdrenCharacter::UnequipWeapon, 0.2f, false);
}

void AAdrenCharacter::ThrowWhenEmpty(const FInputActionInstance& Instance){
	if (Ammo == 0) {
		Throw(Instance);
	}
}

void AAdrenCharacter::UnequipWeapon(){
	EquippedWeapon->SetOwningActor(nullptr);
	EquippedWeapon = nullptr;
}

void AAdrenCharacter::StartRun(){
	if (AdrenPlayerController->IsPaused() && !RunStarted) {
		StartRunDelegate.ExecuteIfBound();
		SetRunStarted(true);
		ShouldDrainHealth = true;
		StopCrouching();
	}
}

void AAdrenCharacter::PauseGame(const struct FInputActionInstance& Instance){
	PauseGameDelegate.ExecuteIfBound();
}

void AAdrenCharacter::ShootFullAuto(const FInputActionInstance& Instance) {
	if (!bCanFire || PlayerWeaponStatus == EPlayerWeaponState::Unarmed) return;
	EquippedWeapon->FireAsLineTrace(PlayerCam->GetComponentLocation(), PlayerCam->GetComponentLocation() + PlayerCam->GetForwardVector() * 10000.f);
	PlayerMesh->GetAnimInstance()->Montage_Play(FireMontage);
	CamManager->StartCameraShake(FireCameraShake->CameraShake, 1.0f);
	Ammo--;
	if (HUDWidget) {
		HUDWidget->SetAmmoCounter(Ammo);
	}
	GetWorldTimerManager().SetTimer(WeaponHandle, this, &AAdrenCharacter::ResetTrigger, FullAutoTriggerCooldown, false);
	bCanFire = false;

}

void AAdrenCharacter::ActivateSloMo(const FInputActionInstance& Instance){
	if (SloMo != MaxSloMo) return;
	bSloMo = true;
	bCanGenerateSloMo = false;
	
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.5f);
	CustomTimeDilation = 1.2f;

}

void AAdrenCharacter::Kick(const FInputActionInstance& Instance) {
	if (GetWorldTimerManager().IsTimerActive(KickTimerHandle)) return;
	if (MovementState == EPlayerMovementState::WallRunning) return;
	EnableKickHitbox();
	//GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, "Kicking", false);
	GetWorldTimerManager().SetTimer(KickTimerHandle, this, &AAdrenCharacter::StopKicking, KickDuration, false);
}

void AAdrenCharacter::HitStun(){
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
}
	

void AAdrenCharacter::EnableKickHitbox(){
	KickHitbox->bHiddenInGame = false;
	KickHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AAdrenCharacter::StopKicking(){
	KickHitbox->bHiddenInGame = true;
	KickHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, "Stop Kicking", false);
}

void AAdrenCharacter::EndDash(){
	StopKicking();
	if (MovementState == EPlayerMovementState::Crouching) {
		StopCrouching();
	}
	else {
		MovementState = EPlayerMovementState::Running;
		MovementStateDelegate.ExecuteIfBound();
	}
}

void AAdrenCharacter::OnKickHitboxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult){
	GEngine->AddOnScreenDebugMessage(4, 5.f, FColor::Red, OtherComp->GetName());
	IDamage* HitActor = Cast<IDamage>(OtherActor);
	if (KickOnce && HitActor) {
		if (MovementState == EPlayerMovementState::Sliding || MovementState == EPlayerMovementState::Dashing) {
			HitActor->DamageTaken(true, SlideKickDamage, this, FVector::ZeroVector, NAME_None, false, true);
		}
		else {
			HitActor->DamageTaken(true, KickDamage, this, FVector::ZeroVector, NAME_None, false, false, true);
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
			FTimerHandle HitStun{};
			GetWorldTimerManager().SetTimer(HitStun, this, &AAdrenCharacter::HitStun, DashHitStunDuration, false);
		}
		if (MovementState == EPlayerMovementState::Dashing) {
			EndDash();
			PlayerMovementComp->Velocity = GetVelocity() * 0.5f; 
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
			FTimerHandle HitStun{};
			GetWorldTimerManager().SetTimer(HitStun, this, &AAdrenCharacter::HitStun, DashHitStunDuration, false);
			

		}
		if (MovementState == EPlayerMovementState::Sliding) {
			PlayerMovementComp->Velocity = GetVelocity() * 0.9f;
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
			FTimerHandle HitStun{};
			GetWorldTimerManager().SetTimer(HitStun, this, &AAdrenCharacter::HitStun, SlideHitStunDuration, false);
		}
		KickOnce = false;
		FTimerHandle CanKickAgainHandle{};
		GetWorldTimerManager().SetTimer(CanKickAgainHandle, this, &AAdrenCharacter::KickAgain, KickDuration, false);
		
	}
}

void AAdrenCharacter::KickAgain() {
	KickOnce = true;
}

void AAdrenCharacter::UpdateSensitivity(float Value){
	Sensitivity = Value;
}

void AAdrenCharacter::FinishShootingFullAuto(const FInputActionInstance& Instance){
	CamManager->StopAllCameraShakes(false);
}

void AAdrenCharacter::ResetTrigger(){
	
	bCanFire = true;
	if (Ammo == 0) {
		bCanFire = false;
		CamManager->StopAllCameraShakes(false);
	}
}

void AAdrenCharacter::DrainLife(bool ShouldDrainLife, float DeltaTime){
	if (ShouldDrainLife) {
		Health -= DeltaTime;
		HUDWidget->SetAdrenalineBarPercent(ConvertHealthToPercent(Health));
		if (Health <= 0.f) {
			PlayerDie();
		}
	}
}

void AAdrenCharacter::GainLife(float HealthRecovery){
	Health = FMath::Clamp(Health + HealthRecovery, 0, MaxHealth);
	if (bCanGenerateSloMo) {
		SloMo = FMath::Clamp(SloMo + 0.1f, 0, MaxSloMo);
		HUDWidget->SetSloMoBarPercent(ConvertSloMoToPercent(SloMo));
	}
	
}

void AAdrenCharacter::WantsToCrouch(const FInputActionInstance& Instance)
{
	if (MovementState == EPlayerMovementState::WallRunning || MovementState == EPlayerMovementState::Dashing) return;
	if (PlayerCapsule->GetScaledCapsuleHalfHeight() == CapsuleHalfHeight) {
		BeginCrouch();
		
	}
	if (MovementState == EPlayerMovementState::Crouching && GetVelocity().SquaredLength() > CrouchSpeedSquared && PlayerMovementComp->IsMovingOnGround()) {
		StartSlide();
	}
}

void AAdrenCharacter::BeginCrouch()
{
	MovementState = EPlayerMovementState::Crouching;
	MovementStateDelegate.ExecuteIfBound();
	PlayerCapsule->SetCapsuleHalfHeight(CrouchedCapsuleHalfHeight);
}

void AAdrenCharacter::StopCrouching(){
	TArray<AActor*> IgnoreSelf{};
	IgnoreSelf.AddUnique(this);
	if (EquippedWeapon != nullptr) {
		IgnoreSelf.AddUnique(EquippedWeapon);
	}
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation() + FVector::UpVector * CrouchedCapsuleHalfHeight, GetActorLocation() + FVector::UpVector * (CapsuleHalfHeight + 20.f), 10.f, ETraceTypeQuery::TraceTypeQuery1, false, IgnoreSelf, EDrawDebugTrace::ForDuration, CheckAboveHead, true);
	if (!CheckAboveHead.bBlockingHit) {
		MovementState = EPlayerMovementState::Running;
		MovementStateDelegate.ExecuteIfBound();
		PlayerCapsule->SetCapsuleHalfHeight(CapsuleHalfHeight);
	}
	if (CheckAboveHead.bBlockingHit) {
		GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Magenta, CheckAboveHead.GetActor()->GetName());
		MovementState = EPlayerMovementState::Sliding;
		MovementStateDelegate.ExecuteIfBound();
		PlayerMovementComp->AddImpulse(PlayerCam->GetForwardVector() * 2000.f, true);
	}
	
}

void AAdrenCharacter::Move(const FInputActionInstance& Instance) {
	if (MovementState == EPlayerMovementState::WallRunning) return;
	FVector2D AxisValue2D = Instance.GetValue().Get<FVector2D>();
	AddMovementInput(GetActorRightVector(), AxisValue2D.X);
	//Dont take any forward or backward input when sliding.
	if (MovementState == EPlayerMovementState::Sliding) return;
	AddMovementInput(GetActorForwardVector(), AxisValue2D.Y);
	
}

