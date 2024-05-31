// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "BaseProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Damage.h"


// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	TempGunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TempMesh"));
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProbablyRealMesh"));
	StunCollider = CreateDefaultSubobject<USphereComponent>(TEXT("StunCollision"));
	PickupCollider = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	SetRootComponent(TempGunMesh);
	GunMesh->SetupAttachment(RootComponent);
	StunCollider->SetupAttachment(RootComponent);
	PickupCollider->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeapon::FireAsLineTrace(FVector Start, FVector End){
	//auto test = GetWorld()->SpawnActor<AActor>(ProjectileToSpawn.Get(), Direction, Rotation);
	FHitResult Hit{};
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility);
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), GunSound, GetActorLocation());
	DrawDebugLine(GetWorld(), Start, End, FColor::Blue, true);
	if (Hit.bBlockingHit) {
		GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Red, Hit.GetActor()->GetName());
		IDamage* HitActorHasInterface = Cast<IDamage>(Hit.GetActor());
		if (HitActorHasInterface) {
			HitActorHasInterface->DamageTaken(false, 0.f, this);
		}
	}
	
	
	
}

