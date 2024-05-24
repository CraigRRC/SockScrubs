// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UPickupsInterface.h"
#include "AdrenCharacter.generated.h"


DECLARE_DELEGATE(MovementDelegate);

enum EPlayerMovementState : uint8 {
	Running,
	Crouching,
	Sliding,
	WallRunning,
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
class SOCKSCRUBS_API AAdrenCharacter : public ACharacter, public IUPickupsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAdrenCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void PickupWeapon(AActor* Weapon, WeaponType WeaponType) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
	//Overrides
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	//Movement Related
	MovementDelegate MovementStateDelegate{};

	enum EPlayerMovementState MovementState {};

	UPROPERTY(BlueprintReadOnly, Category = Anims)
	EPlayerWeaponState PlayerWeaponStatus{ EPlayerWeaponState::Unarmed };

	float MaxPlayerSpeed{};

	void UpdateMovementState();

	UPROPERTY()
	class UCharacterMovementComponent* PlayerMovementComp{};

	//Input
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Crouch{};

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
	class AAdrenPlayerController* AdrenPlayerController{};

	UFUNCTION()
	void Look(const struct FInputActionInstance& Instance);

	UFUNCTION()
	void Throw(const struct FInputActionInstance& Instance);

	void UnequipWeapon();


	UFUNCTION()
	void WantsToCrouch(const struct FInputActionInstance& Instance);

	void BeginCrouch();

	void StopCrouching();

	void StartSlide();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slide)
	class UCameraShakeSourceComponent* SlideCameraShake{};

	void CalcFloorInfluence();

	void ClampSlideVelocity();

	float DownhillForce{ 400000.f };

	float MaxSlideSpeed{ 10000000.f };

	float SlideImpulseForce{ 250.f };

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

	UFUNCTION()
	void ShootFullAuto(const struct FInputActionInstance& Instance);

	UFUNCTION()
	void FinishShootingFullAuto(const struct FInputActionInstance& Instance);

	TObjectPtr<APlayerCameraManager> CamManager{};

	void ResetTrigger();

	float FullAutoTriggerCooldown{};

	bool bCanFire{ true };

	//Character Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerAttributes")
	class UCapsuleComponent* PlayerCapsule{};
	float CapsuleHalfHeight{};
	float CrouchedCapsuleHalfHeight{};

	class ABaseWeapon* EquippedWeapon {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerAttributes")
	class UCameraComponent* PlayerCam{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerAttributes")
	class USkeletalMeshComponent* PlayerMesh{};

public:
	FORCEINLINE float GetStandingCapsuleHalfHeight() { return CapsuleHalfHeight; }
	FORCEINLINE float GetCrouchingCapsuleHalfHeight() { return CrouchedCapsuleHalfHeight; }
	FORCEINLINE EPlayerMovementState GetPlayerMovementState() { return MovementState; }
};




