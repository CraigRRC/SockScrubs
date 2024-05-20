// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

UCLASS()
class SOCKSCRUBS_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	class USkeletalMeshComponent* TempGunMesh{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	class UStaticMeshComponent* GunMesh{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	class USphereComponent* StunCollider{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	class USphereComponent* PickupCollider{};

	


public:	
	FORCEINLINE class USkeletalMeshComponent* GetGunMesh() { return TempGunMesh; }
	FORCEINLINE class USphereComponent* GetStunCollider() { return StunCollider; }
	FORCEINLINE class USphereComponent* GetPickupCollider() { return PickupCollider; }

	

};

UENUM(BlueprintType)
enum class WeaponType : uint8 {
	Rifle UMETA(DisplayName = "Rifle"),
};
