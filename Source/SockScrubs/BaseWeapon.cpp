// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "BaseProjectile.h"
#include "Kismet/GameplayStatics.h"


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

void ABaseWeapon::Fire(FVector Direction, FRotator Rotation){
	auto test = GetWorld()->SpawnActor<AActor>(ProjectileToSpawn.Get(), Direction, Rotation);
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), GunSound, GetActorLocation());
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Blue, test->GetName());
	
	
	
}

