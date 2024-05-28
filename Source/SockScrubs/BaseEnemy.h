// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseEnemy.generated.h"

UENUM(Blueprintable)
enum class EnemyState : uint8 {
	Ready UMETA(DisplayName = "Ready"),
	Activated UMETA(DisplayName = "Activated"),
	Combat UMETA(DisplayName = "InCombat"),
	Stunned UMETA(DisplayName = "Stunned"),
};

UCLASS()
class SOCKSCRUBS_API ABaseEnemy : public APawn
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyAttributes")
	float ActivationRadius{ 16000000.f };

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

	

public:
	

};
