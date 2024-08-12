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
#include "VHS_Anim.h"
#include "AdrenGameMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/AudioComponent.h"


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
	KickCameraShake = CreateDefaultSubobject<UCameraShakeSourceComponent>(TEXT("KickCameraShakeSource"));
	KickHitbox->OnComponentBeginOverlap.AddDynamic(this, &AAdrenCharacter::OnKickHitboxBeginOverlap);
	PreviousFOV = PlayerCam->FieldOfView;
	HighSpeedFOV = PreviousFOV + FOVBuffer;
	

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
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void AAdrenCharacter::Destroyed()
{
	Super::Destroyed();
	GetWorldTimerManager().ClearAllTimersForObject(this);
	if (MovementStateDelegate.IsBound()) {
		MovementStateDelegate.Unbind();
	}
	if (GameMode != nullptr) {
		if (GameMode->GainAdrenalineDelegate.IsBound()) {
			GameMode->GainAdrenalineDelegate.Unbind();
		}
	}
}

void AAdrenCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult){
	Super::CalcCamera(DeltaTime, OutResult);

	OutResult.Rotation += CameraTiltOffset;
}

void AAdrenCharacter::IncreaseTilt(){
	if (!GetWorld()) return;
	GetWorldTimerManager().ClearTimer(TiltTimerHandle);
	GetWorldTimerManager().SetTimer(TiltTimerHandle, [this]() {
		InterpTilt(MaxHeadTilt, GetWorld()->GetDeltaSeconds());
		}, GetWorld()->GetDeltaSeconds(), true);
}

void AAdrenCharacter::DecreaseTilt(){
	if (!GetWorld()) return;
	GetWorldTimerManager().ClearTimer(TiltTimerHandle);
	GetWorldTimerManager().SetTimer(TiltTimerHandle, [this]() {
		InterpTilt(-MaxHeadTilt, GetWorld()->GetDeltaSeconds());
		}, GetWorld()->GetDeltaSeconds(), true);
}

void AAdrenCharacter::InterpTilt(float TiltAmount, float DeltaTime){
	if (!GetWorld()) return;
	float CurrentTilt = CameraTiltOffset.Roll;
	if (ReturnToZero) {
		TiltAmount = 0;
	}
	float InterpedTilt = FMath::FInterpTo(CurrentTilt, TiltAmount, DeltaTime, TiltInterpSpeed);
	CameraTiltOffset.Roll = InterpedTilt;
	GEngine->AddOnScreenDebugMessage(13, 1.f, FColor::Blue, FString::Printf(TEXT("%f"), InterpedTilt));
	if (FMath::IsNearlyEqual(InterpedTilt, TiltAmount, 0.01f && TiltAmount != 0)) {
		GetWorldTimerManager().ClearTimer(TiltTimerHandle);
		CameraTiltOffset.Roll = TiltAmount;
	}
}

void AAdrenCharacter::Jump(){
	if (MovementState == EPlayerMovementState::WallRunning) {
		Super::Jump();
		if (JumpSounds.Num() > 0 && bCanJumpGrunt) {
			UGameplayStatics::PlaySound2D(GetWorld(), JumpSounds[FMath::RandRange(0, JumpSounds.Num() - 1)], SFXVolume);
			bCanJumpGrunt = false;
		}
		GetWorld()->LineTraceSingleByChannel(LeftOfPlayerHit, GetActorLocation() + FVector::UpVector * CapsuleHalfHeight, GetActorLocation() + GetActorRightVector() * -WallRunBlockingHitLength, ECollisionChannel::ECC_Camera);
		GetWorld()->LineTraceSingleByChannel(RightOfPlayerHit, GetActorLocation() + FVector::UpVector * CapsuleHalfHeight, GetActorLocation() + GetActorRightVector() * WallRunBlockingHitLength, ECollisionChannel::ECC_Camera);
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
		if (JumpSounds.Num() > 0 && bCanJumpGrunt) {
			UGameplayStatics::PlaySound2D(GetWorld(), JumpSounds[FMath::RandRange(0, JumpSounds.Num() - 1)], SFXVolume);
			bCanJumpGrunt = false;
		}
		

		if (!PlayerMovementComp->IsMovingOnGround()) {
			GetWorld()->LineTraceSingleByChannel(LeftOfPlayerHit, GetActorLocation() + FVector::UpVector * CapsuleHalfHeight, GetActorLocation() + GetActorRightVector() * -WallRunBlockingHitLength, ECollisionChannel::ECC_Camera);
			GetWorld()->LineTraceSingleByChannel(RightOfPlayerHit, GetActorLocation() + FVector::UpVector * CapsuleHalfHeight, GetActorLocation() + GetActorRightVector() * WallRunBlockingHitLength, ECollisionChannel::ECC_Camera);
			
			if (LeftOfPlayerHit.bBlockingHit) {
				GEngine->AddOnScreenDebugMessage(12, 5.f, FColor::Red, "Here");
				MovementState = EPlayerMovementState::WallRunning;
				MovementStateDelegate.ExecuteIfBound();
				PlayerMovementComp->AddImpulse(GetActorRightVector() * -WallRunSuctionImpulse, true);
				//CameraTiltOffset.Roll += MaxHeadTilt;
				ReturnToZero = false;
				IncreaseTilt();
				HeadTiltedRight = false;
				if (PlayerMovementComp->Velocity.SizeSquared2D() < MaxSpeed / 2) {
					PlayerMovementComp->Velocity = FVector::ZeroVector;
					PlayerMovementComp->AddImpulse(GetActorForwardVector() * WallRunImpulse, true);
				}
				
				PlayerMovementComp->AddImpulse(FVector::UpVector * 250.f, true);
				GetWorldTimerManager().SetTimer(WallRunningHandle, this, &AAdrenCharacter::FellOffWall, WallRunningDuration, false);
				
			}
			if (RightOfPlayerHit.bBlockingHit) {
				GEngine->AddOnScreenDebugMessage(12, 5.f, FColor::Red, "Here");
				MovementState = EPlayerMovementState::WallRunning;
				MovementStateDelegate.ExecuteIfBound();
				PlayerMovementComp->AddImpulse(GetActorRightVector() * WallRunSuctionImpulse, true);
				//CameraTiltOffset.Roll -= MaxHeadTilt;
				ReturnToZero = false;
				DecreaseTilt();
				HeadTiltedRight = true;
				if (PlayerMovementComp->Velocity.SizeSquared2D() < MaxSpeed / 2) {
					PlayerMovementComp->AddImpulse(GetActorForwardVector() * WallRunImpulse, true);
				}
				PlayerMovementComp->AddImpulse(FVector::UpVector * 250.f, true);
				GetWorldTimerManager().SetTimer(WallRunningHandle, this, &AAdrenCharacter::FellOffWall, WallRunningDuration, false);
			}
			
		}
	}
}

void AAdrenCharacter::FellOffWall() {
	MovementState = EPlayerMovementState::Running;
	MovementStateDelegate.ExecuteIfBound();
	ReturnToZero = true;
	if (HeadTiltedRight) {
		//CameraTiltOffset.Roll += MaxHeadTilt;
		
		IncreaseTilt();
	}
	else {
		//CameraTiltOffset.Roll -= MaxHeadTilt;
		DecreaseTilt();
	}
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
		PlayerMovementComp->Velocity = GetVelocity().GetClampedToMaxSize2D(2300);
	}

	if (PlayerMovementComp->IsMovingOnGround() || MovementState == EPlayerMovementState::WallRunning) {
		bCanDash = true;
		bCanJumpGrunt = true;
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
			GetWorld()->LineTraceSingleByChannel(LeftOfPlayerHit, GetActorLocation() + FVector::UpVector * CapsuleHalfHeight, GetActorLocation() + GetActorRightVector() * -WallRunBlockingHitLength, ECollisionChannel::ECC_Camera);
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
			GetWorld()->LineTraceSingleByChannel(RightOfPlayerHit, GetActorLocation() + FVector::UpVector * CapsuleHalfHeight, GetActorLocation() + GetActorRightVector() * WallRunBlockingHitLength, ECollisionChannel::ECC_Camera);
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
			if (HeadRushDepleted != nullptr) {
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadRushDepleted, GetActorLocation(), SFXVolume);
			}
			bCanGenerateSloMo = true;
			bSloMo = false;
			FPostProcessSettings Temp{};
			Temp.bOverride_WhiteTemp = true;
			Temp.WhiteTemp = 6500.f;
			PlayerCam->PostProcessSettings = Temp;
		}
	}

	if (bCanGenerateSloMo) {
		//10 seconds
		SloMo = FMath::Clamp(SloMo + DeltaTime / 5, 0, MaxSloMo);
		HUDWidget->SetSloMoBarPercent(ConvertSloMoToPercent(SloMo));
		if (SloMo == MaxSloMo) {
			if (HeadRushAvalible != nullptr) {
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadRushAvalible, GetActorLocation(), SFXVolume);
			}
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

void AAdrenCharacter::PlayNearFinishedHeadRushSFX()
{
	if (HeadRushNearFinished != nullptr) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadRushNearFinished, GetActorLocation(), SFXVolume);
	}
}

void AAdrenCharacter::StopSliding()
{
	DecreaseFOV();
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
	IncreaseFOV();
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
	//GameMode->ResetComboCount();
	if (Grunts.Num() > 0) {
		UGameplayStatics::PlaySound2D(GetWorld(), Grunts[FMath::RandRange(0, Grunts.Num() - 1)], SFXVolume);
	}
	if (HitSound != nullptr) {
		UGameplayStatics::PlaySound2D(GetWorld(), HitSound, SFXVolume);
	}
	Health -= DamageDelta;
	
	float ClampedHealth = FMath::Clamp(Health, 0.f, MaxHealth);
	HUDWidget->SetAdrenalineBarPercent(ConvertHealthToPercent(ClampedHealth));
	if (ClampedHealth <= 0.f) {
		ShouldDrainHealth = false;
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathGrunt, GetActorLocation(), SFXVolume);
		FTimerHandle GruntSFXDelay{};
		GetWorldTimerManager().SetTimer(GruntSFXDelay, this, &AAdrenCharacter::PlayDeathSFX, 0.02f, false);
		PlayerDie();
	}
	else {
		if (PlayerCameraShake->CameraShake) {
			CamManager->StartCameraShake(PlayerCameraShake->CameraShake, 1.0f);
		}
	}
}

void AAdrenCharacter::PlayDeathSFX()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSFX, GetActorLocation(), SFXVolume);
}

void AAdrenCharacter::PlayerDie()
{
	PlayerState = EPlayerState::Dead;
	VHSWidget->SetOwningPlayer(AdrenPlayerController);
	VHSWidget->AddToPlayerScreen();
	CamManager->StopAllCameraShakes(true);
	DisableInput(AdrenPlayerController);
	PlayerCam->SetFieldOfView(PreviousFOV);
	GetWorldTimerManager().ClearTimer(FOVTimerHandle);
	GetWorldTimerManager().ClearTimer(CrouchStandTimer);
	GetWorldTimerManager().ClearTimer(TiltTimerHandle);
	FTimerHandle DeathTimer{};
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1f);
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AAdrenCharacter::PlayerDead, 0.1f, false);
}

void AAdrenCharacter::PlayerDead()
{
	//Trigger Event that tells the GameMode that the player died.
	FString LevelString = UGameplayStatics::GetCurrentLevelName(GetWorld());
	FName LevelName = FName(LevelString);
	UGameplayStatics::OpenLevel(GetWorld(), LevelName);
}

void AAdrenCharacter::Look(const FInputActionInstance& Instance) {
	FVector2D AxisValue2D = Instance.GetValue().Get<FVector2D>();
	AddControllerYawInput(AxisValue2D.X * Sensitivity);
	AddControllerPitchInput(AxisValue2D.Y * Sensitivity);
}

void AAdrenCharacter::AirDash(const FInputActionInstance& Instance){
	if (!PlayerMovementComp->IsMovingOnGround() && bCanDash) {
		IncreaseFOV();
		PlayerMovementComp->Velocity = FVector::ZeroVector;
		PlayerMovementComp->AddImpulse((PlayerCam->GetForwardVector() + FVector::UpVector * 0.2f) * DashImpulseForce);
		MovementState = EPlayerMovementState::Dashing;
		MovementStateDelegate.ExecuteIfBound();
		bCanDash = false;
		if (AirDashGrunt != nullptr) {
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), AirDashGrunt, GetActorLocation(), SFXVolume);
		}
		if (HeadTiltedRight) {
			ReturnToZero = true;
			//CameraTiltOffset.Roll += MaxHeadTilt;
			IncreaseTilt();
		}
		else {
			ReturnToZero = true;
			//CameraTiltOffset.Roll -= MaxHeadTilt;
			DecreaseTilt();
		}

		
	}
}

void AAdrenCharacter::IncreaseFOV(){
	if (!GetWorld()) return;
	GetWorldTimerManager().ClearTimer(FOVTimerHandle);

	GetWorldTimerManager().SetTimer(FOVTimerHandle, [this]() {
		InterpFOV(HighSpeedFOV, GetWorld()->GetDeltaSeconds()); 
		} ,GetWorld()->GetDeltaSeconds(), true);
}

void AAdrenCharacter::DecreaseFOV() {
	if (!GetWorld()) return;
	GetWorldTimerManager().ClearTimer(FOVTimerHandle);

	GetWorldTimerManager().SetTimer(FOVTimerHandle, [this]() {
		InterpFOV(PreviousFOV, GetWorld()->GetDeltaSeconds());
		}, GetWorld()->GetDeltaSeconds(), true);
}

void AAdrenCharacter::InterpFOV(float TargetFOV, float DeltaTime) {
	if (!GetWorld()) return; 
	float CurrentFOV = PlayerCam->FieldOfView;
	float InterpedFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVInterpSpeed);
	PlayerCam->SetFieldOfView(InterpedFOV);

	if (FMath::IsNearlyEqual(InterpedFOV, TargetFOV, 0.01f)) {
		PlayerCam->SetFieldOfView(TargetFOV);
		GetWorldTimerManager().ClearTimer(FOVTimerHandle);
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
	HUDWidget->SetOutOfAmmoVisiblilty(ESlateVisibility::Hidden);
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
		HUDWidget->SetOutOfAmmoVisiblilty(ESlateVisibility::Hidden);
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
		if (Ammo == 0) {
			HUDWidget->SetOutOfAmmoVisiblilty(ESlateVisibility::Visible);
		}
	}

	GetWorldTimerManager().SetTimer(WeaponHandle, this, &AAdrenCharacter::ResetTrigger, FullAutoTriggerCooldown, false);
	bCanFire = false;

}

void AAdrenCharacter::ActivateSloMo(const FInputActionInstance& Instance){
	if (SloMo != MaxSloMo) return;
	bSloMo = true;
	bCanGenerateSloMo = false;
	if (HeadRushUsage != nullptr) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadRushUsage, GetActorLocation(), SFXVolume);
	}
	FTimerHandle LaughSFXDelay{};
	GetWorldTimerManager().SetTimer(LaughSFXDelay, this, &AAdrenCharacter::PlayNearFinishedHeadRushSFX, 1.1f, false);
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.5f);
	CustomTimeDilation = 1.2f;
	FPostProcessSettings Temp{};
	Temp.bOverride_WhiteTemp = true;
	Temp.WhiteTemp = 4000.f;
	PlayerCam->PostProcessSettings = Temp;

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
	DecreaseFOV();
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
		if (KickCameraShake->CameraShake) {
			CamManager->StartCameraShake(KickCameraShake->CameraShake, 1.0f);
		}

		if (KickHitSound != nullptr) {
			UGameplayStatics::PlaySound2D(GetWorld(), KickHitSound, SFXVolume);
		}

		if (MovementState == EPlayerMovementState::Sliding || MovementState == EPlayerMovementState::Dashing) {
			HitActor->DamageTaken(true, SlideKickDamage, this, FVector::ZeroVector, NAME_None, false, true, true);
		}
		else {
			HitActor->DamageTaken(true, KickDamage, this, FVector::ZeroVector, NAME_None, false, false, true);
			if (!bSloMo) {
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
				FTimerHandle HitStun{};
				GetWorldTimerManager().SetTimer(HitStun, this, &AAdrenCharacter::HitStun, DashHitStunDuration, false);
			}
			
		}
		if (MovementState == EPlayerMovementState::Dashing) {
			EndDash();
			PlayerMovementComp->Velocity = GetVelocity() * 0.5f; 
			if (!bSloMo) {
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
				FTimerHandle HitStun{};
				GetWorldTimerManager().SetTimer(HitStun, this, &AAdrenCharacter::HitStun, DashHitStunDuration, false);
			}
		}
		if (MovementState == EPlayerMovementState::Sliding) {
			PlayerMovementComp->Velocity = GetVelocity() * 0.9f;
			if (!bSloMo) {
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
				FTimerHandle HitStun{};
				GetWorldTimerManager().SetTimer(HitStun, this, &AAdrenCharacter::HitStun, SlideHitStunDuration, false);
			}
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
	GEngine->AddOnScreenDebugMessage(12, 1.f, FColor::Black, FString::Printf(TEXT("%f"), HealthRecovery));
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
	InterpCameraStandToCrouch();
}

void AAdrenCharacter::InterpCameraStandToCrouch(){
	if (!GetWorld()) return;
	GetWorldTimerManager().ClearTimer(CrouchStandTimer);

	FVector CurrentCameraLocation = PlayerCam->GetRelativeLocation();
	InterpedLocation = FVector(0, 0, CurrentCameraLocation.Z - CrouchedCapsuleHalfHeight);

	GetWorldTimerManager().SetTimer(CrouchStandTimer, [this]() {
		SmoothCrouchStandInterp(InterpedLocation, GetWorld()->GetDeltaSeconds());
		}, GetWorld()->GetDeltaSeconds(), true);

	
}

void AAdrenCharacter::InterpCameraCrouchToStand(){
	if (!GetWorld()) return;
	GetWorldTimerManager().ClearTimer(CrouchStandTimer);

	FVector CurrentCameraLocation = PlayerCam->GetRelativeLocation();
	InterpedLocation = FVector(0, 0, CurrentCameraLocation.Z + CrouchedCapsuleHalfHeight);

	GetWorldTimerManager().SetTimer(CrouchStandTimer, [this]() {
		SmoothCrouchStandInterp(InterpedLocation, GetWorld()->GetDeltaSeconds());
		}, GetWorld()->GetDeltaSeconds(), true);

}

void AAdrenCharacter::SmoothCrouchStandInterp(FVector Target, float DeltaTime){
	if (!GetWorld()) return;
	FVector CurrentLocation = PlayerCam->GetRelativeLocation();
	//FVector TargetLocation = FMath::VInterpTo(CurrentLocation, Target, DeltaTime, 5.f);
	FVectorSpringState SpringState{};
	FVector TargetLocation = UKismetMathLibrary::VectorSpringInterp(CurrentLocation, Target, SpringState, 5000.f, 0.1f, DeltaTime);

	PlayerCam->SetRelativeLocation(TargetLocation.GetClampedToSize(CrouchedCapsuleHalfHeight, CapsuleHalfHeight));

	if (FVector::Dist(CurrentLocation, TargetLocation) <= KINDA_SMALL_NUMBER) {
		PlayerCam->SetRelativeLocation(TargetLocation);
		GetWorldTimerManager().ClearTimer(CrouchStandTimer);
	}
}

void AAdrenCharacter::StopCrouching(){

	InterpCameraCrouchToStand();

	TArray<AActor*> IgnoreSelf{};
	IgnoreSelf.AddUnique(this);
	if (EquippedWeapon != nullptr) {
		IgnoreSelf.AddUnique(EquippedWeapon);
	}
	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation() + FVector::UpVector * CrouchedCapsuleHalfHeight, GetActorLocation() + FVector::UpVector * (CapsuleHalfHeight + 50.f), 5.f, ETraceTypeQuery::TraceTypeQuery1, false, IgnoreSelf, EDrawDebugTrace::ForDuration, CheckAboveHead, true);
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

