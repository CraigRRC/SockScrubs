// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

UCLASS()
class SOCKSCRUBS_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProjectileAttributes)
	float Damage{30.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProjectileAttributes)
	class USphereComponent* CollisionSphere{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProjectileAttributes)
	class UStaticMeshComponent* BulletMesh{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProjectileAttributes)
	class UProjectileMovementComponent* ProjectileMovementComponent{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProjectileAttributes)
	float InitialSpeed{ 3000.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProjectileAttributes)
	float MaxSpeed{ 3000.f };

	/*UFUNCTION()
	void OnCollisionSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);*/

	UFUNCTION()
	void OnCollisionSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
