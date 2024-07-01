// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UPickupsInterface.h"
#include "Damage.h"
#include "AdrenCharacter.generated.h"


DECLARE_DELEGATE(MovementDelegate);
DECLARE_DELEGATE(StartRunDelegate);
DECLARE_DELEGATE(PauseGameDelegate);

UENUM(Blueprintable)
enum EPlayerState : uint8 {
	Alive UMETA(DisplayName = "Alive"),
	Dead UMETA(DisplayName = "Dead")
};

UENUM(Blueprintable)
enum EPlayerMovementState : uint8 {
	Running UMETA(DisplayName = "Running"),
	Crouching UMETA(DisplayName = "Crouching"),
	Sliding UMETA(DisplayName = "Sliding"),
	Dashing UMETA(DisplayName = "Dashing"),
	WallRunning UMETA(DisplayName = "WallRunning"),
};

UENUM(Blueprintable)
enum class EPlayerWeaponState : uint8 {
	Unarmed UMETA(DisplayName = "Unarmed"),
	HasRifle UMETA(DisplayName = "HasRifle"),
	HasShotgun UMETA(DisplayName = "HasShotgun"),
	HasPistol UMETA(DisplayName = "HasPistol"),
	HasRockets UMETA(DisplayName = "HasRockets"),
};

UCLASS()
class SOCKSCRUBS_API AAdrenCharacter : public ACharacter, public IUPickupsInterface, public IDamage
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAdrenCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void StopSliding();

	bool RunStarted{ false };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI)
	class UPlayerHUDWidget* HUDWidget{};

	UFUNCTION()
	virtual void PickupWeapon(AActor* Weapon, WeaponType WeaponType) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void DamageTaken(bool Stun, float DamageDelta, AActor* DamageDealer, FVector ImpactPoint, FName BoneName, bool Headshot, bool Tripped, bool Kicked) override;

	void PlayerDie();

	void PlayerDead();

	StartRunDelegate StartRunDelegate{};

	PauseGameDelegate PauseGameDelegate{};

	FTimerHandle WallRunningHandle{};

	float WallRunningDuration{ 2.f };
	
	void FellOffWall();

	float KickDuration{ 0.45f };

	float WallJumpForce{ 600.f };


protected:
	//Overrides
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	virtual void Jump() override;

	bool KickOnce{ true };

	bool EndDashOnce{ true };
	
	void KickAgain();

	float Sensitivity{};

	UFUNCTION()
	void UpdateSensitivity(float Value);

	//Movement Related
	MovementDelegate MovementStateDelegate{};
	EPlayerMovementState MovementState {EPlayerMovementState::Running};

	EPlayerState PlayerState{ EPlayerState::Alive };

	FHitResult LeftOfPlayerHit{};
	FHitResult RightOfPlayerHit{};

	UPROPERTY(BlueprintReadOnly, Category = Anims)
	EPlayerWeaponState PlayerWeaponStatus{ EPlayerWeaponState::Unarmed };

	float MaxPlayerSpeed{};

	void UpdateMovementState();

	UPROPERTY(BlueprintReadOnly, Category = PlayerAttributes)
	class UCharacterMovementComponent* PlayerMovementComp{};

	//Input
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Pause{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Jump{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Kick{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Look{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Move{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_ActivateSloMo{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Shoot{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Throw{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_StartRun{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Restart{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Slide{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_AirDash{};

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class AAdrenPlayerController* AdrenPlayerController{};

	UFUNCTION()
	void Look(const struct FInputActionInstance& Instance);

	UFUNCTION()
	void AirDash(const struct FInputActionInstance& Instance);

	UFUNCTION(BlueprintCallable)
	void Throw(const struct FInputActionInstance& Instance);

	UFUNCTION()
	void ThrowWhenEmpty(const struct FInputActionInstance& Instance);

	void UnequipWeapon();

	void StartRun();

	UFUNCTION()
	void PauseGame(const struct FInputActionInstance& Instance);


	UFUNCTION()
	void WantsToCrouch(const struct FInputActionInstance& Instance);

	void BeginCrouch();

	void StopCrouching();

	void StartSlide();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slide)
	class UCameraShakeSourceComponent* SlideCameraShake{};

	FHitResult CheckAboveHead{};

	void CalcFloorInfluence();

	float DownhillForce{ 380000.f };

	float MaxSpeed{ 5290000.f };

	float SlideImpulseForce{ 350.f };

	float DashImpulseForce{ 250000.f };

	UFUNCTION()
	void Move(const struct FInputActionInstance& Instance);

	//GunProperties
	uint8 Ammo{};

	UPROPERTY(EditAnywhere, Category = Anims)
	class UAnimMontage* FireMontage{};

	float ProjectileForwardOffset{100.f};

	struct FTimerHandle WeaponHandle {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	class UCameraShakeSourceComponent* FireCameraShake{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerAttributes)
	class UCameraShakeSourceComponent* PlayerCameraShake{};

	UFUNCTION()
	void ShootFullAuto(const struct FInputActionInstance& Instance);

	UFUNCTION()
	void ActivateSloMo(const struct FInputActionInstance& Instance);
	
	bool bSloMo{false};

	bool bCanGenerateSloMo{true};

	UFUNCTION()
	void Kick(const struct FInputActionInstance& Instance);

	UFUNCTION()
	void HitStun();

	float DashHitStunDuration{ 0.05f };

	float SlideHitStunDuration{ 0.01f };

	float KickHitStunDuration{ 0.05f };

	void EnableKickHitbox();

	void StopKicking();

	void EndDash();

	bool bCanDash{ true };

	UFUNCTION()
	void OnKickHitboxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void FinishShootingFullAuto(const struct FInputActionInstance& Instance);

	TObjectPtr<APlayerCameraManager> CamManager{};

	void ResetTrigger();

	float FullAutoTriggerCooldown{};

	bool bCanFire{ true };

	//Character Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerAttributes)
	class UCapsuleComponent* PlayerCapsule{};
	float CapsuleHalfHeight{};
	float CrouchedCapsuleHalfHeight{};

	void DrainLife(bool ShouldDrainLife, float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerAttributes)
	bool ShouldDrainHealth{ false };

	void GainLife(float HealthRecovery);

	class AAdrenGameMode* GameMode{};

	FTimerHandle KickTimerHandle{};

	class ABaseWeapon* EquippedWeapon {};

	UPROPERTY(EditDefaultsOnly, Category = PlayerAttributes)
	float SlideKickDamage{ 60.f };

	UPROPERTY(EditDefaultsOnly, Category = PlayerAttributes)
	float KickDamage{ 100.f };

	UPROPERTY(EditDefaultsOnly, Category = PlayerAttributes)
	float CrouchSpeedSquared{ 15000.f };

	UPROPERTY(EditDefaultsOnly, Category = PlayerAttributes)
	float Health{ 10.f };

	UPROPERTY(EditDefaultsOnly, Category = PlayerAttributes)
	float MaxHealth{ Health };

	UPROPERTY(EditDefaultsOnly, Category = PlayerAttributes)
	float SloMo{ 0.f };

	UPROPERTY(EditDefaultsOnly, Category = PlayerAttributes)
	float MaxSloMo{ 2.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerAttributes)
	class UCameraComponent* PlayerCam{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerAttributes)
	class UBoxComponent* KickHitbox{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerAttributes)
	class USkeletalMeshComponent* PlayerMesh{};

public:
	FORCEINLINE float GetStandingCapsuleHalfHeight() { return CapsuleHalfHeight; }
	FORCEINLINE float GetCrouchingCapsuleHalfHeight() { return CrouchedCapsuleHalfHeight; }
	FORCEINLINE EPlayerMovementState GetPlayerMovementState() { return MovementState; }
	FORCEINLINE void SetRunStarted(bool Delta) { RunStarted = Delta; }
	FORCEINLINE float ConvertHealthToPercent(float CurrentHealth) { return CurrentHealth / MaxHealth; }
	FORCEINLINE float ConvertSloMoToPercent(float Delta) { return Delta / MaxSloMo; }
	FORCEINLINE void SetPlayerSensitivity(float Delta) { Sensitivity = Delta; }
	
};




