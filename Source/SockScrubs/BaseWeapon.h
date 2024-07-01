// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

UENUM(BlueprintType)
enum class WeaponType : uint8 {
	Rifle UMETA(DisplayName = "Rifle"),
};

//Good learning experience, but not the right move for this, as I would need to enter in all the struct data in each child class anyway. 
//This would be good to use in each child class, which is why I tried to use it here, but each class still needs to make the struct by passing in their own variables anyway.
//SOOO instead of making two sets of variables, one for the struct, and one for the class that is needed to fill the ones in the struct. I will just make them in the class.

//USTRUCT(BlueprintType)
//struct FWeaponProperties {
//	GENERATED_BODY()
//
//	UPROPERTY(BlueprintReadWrite, EditAnywhere)
//	uint8 ClipSize{};
//	UPROPERTY(BlueprintReadWrite, EditAnywhere)
//	uint8 Ammo{};
//	UPROPERTY(BlueprintReadWrite, EditAnywhere)
//	class ABaseProjectile* ProjectileToSpawn{};
//	
//};


UCLASS()
class SOCKSCRUBS_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void FireAsLineTrace(FVector Start, FVector End);

	FTimerHandle LifeTimeHandle{};

	void CleanUp();

	void SetLifeTimer();

	void PlayPickupSound();


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	uint8 ClipSize{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	float FireRate{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	TSubclassOf<class ABaseProjectile> ProjectileToSpawn{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	USoundBase* GunSound{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	USoundBase* PickupSound{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	TSubclassOf<UCameraShakeBase> GunCameraShake{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	AActor* OwningActor{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	float FirePower{ 20.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	float ThrownDamage{ 60.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	float BulletRadius{ 5.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GunAttributes)
	class UMaterialInterface* BulletImpact{};

	bool DoOnce{ true };

	void ResetDoOnce();
	
	UFUNCTION()
	void OnStunColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	FORCEINLINE class USkeletalMeshComponent* GetGunMesh() { return TempGunMesh; }
	FORCEINLINE class USphereComponent* GetStunCollider() { return StunCollider; }
	FORCEINLINE class USphereComponent* GetPickupCollider() { return PickupCollider; }
	FORCEINLINE uint8 GetClipSize() { return ClipSize; }
	FORCEINLINE float GetFireRate() { return FireRate; }
	FORCEINLINE TSubclassOf<class UCameraShakeBase> GetCameraShakeBase() { return GunCameraShake; }
	FORCEINLINE void SetOwningActor(AActor* Actor) { OwningActor = Actor; }
	FORCEINLINE void ResetLifeTime() { GetWorldTimerManager().ClearTimer(LifeTimeHandle); }
};


