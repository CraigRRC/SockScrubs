// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Damage.h"
#include "BaseEnemy.generated.h"

DECLARE_DELEGATE(EnemyStateDelegate);
DECLARE_DELEGATE(EnemyWeaponStateDelegate);

UENUM(Blueprintable)
enum class EEnemyState : uint8 {
	Ready UMETA(DisplayName = "Ready"),
	Activated UMETA(DisplayName = "Activated"),
	Combat UMETA(DisplayName = "InCombat"),
	Stunned UMETA(DisplayName = "Stunned"),
	Dead UMETA(DisplayName = "Dead"),
};

UENUM(Blueprintable)
enum class EEnemyWeaponState : uint8 {
	Armed UMETA(DisplayName = "Armed"),
	Disarmed UMETA(DisplayName = "Disarmed"),
};

UCLASS()
class SOCKSCRUBS_API ABaseEnemy : public APawn, public IDamage
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABaseEnemy();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	EnemyStateDelegate EnemyStateDelegate{};

	EnemyWeaponStateDelegate EnemyWeaponStateDelegate{};

	EEnemyState EnemyState{ EEnemyState::Ready };

	EEnemyWeaponState EnemyWeaponState{ EEnemyWeaponState::Armed };

	UFUNCTION()
	virtual void DamageTaken(bool Stun, float DamageDelta, AActor* DamageDealer);

	UFUNCTION()
	void SwitchState();

	void DropEquippedWeapon();

	UFUNCTION()
	void SwitchWeaponState();

	UFUNCTION()
	void ReturnFromStunState();

	FTimerHandle StunDuration{};

	FTimerHandle DistHandle{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	double ActivationRadius{ 30000000.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	TSubclassOf<class ABaseWeapon> WeaponToSpawnWhenDropped{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	float Health{ 100.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	float MaxHealth{ Health };

	//Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	class USphereComponent* HeadHitbox{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	class UBoxComponent* BodyHitbox{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	class UStaticMeshComponent* TempBodyMesh{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	class UStaticMeshComponent* TempHeadMesh{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	class USkeletalMeshComponent* TempGunMesh{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	class USceneComponent* ProjectileSpawnPoint{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	class USceneComponent* Root{};

	void LookAtPlayer();

	void RotateTowardPlayer();

	void CalcDistBtwnPlayer();

	UFUNCTION(BlueprintNativeEvent)
	void Fire();

	void Fire_Implementation();

	UPROPERTY(EditDefaultsOnly, Category = "EnemyAttributes")
	float RateOfFire{ 1.5f };

	FTimerHandle FireHandle{};

	UPROPERTY()
	class AAdrenCharacter* Player{};

	UPROPERTY()
	class AAdrenCharacter* SeenPlayer{};


public:
	

};
