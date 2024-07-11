// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "BaseProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Damage.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"



// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	TempGunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TempMesh"));
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProbablyRealMesh"));
	StunCollider = CreateDefaultSubobject<USphereComponent>(TEXT("StunCollision"));
	MuzzleFlashLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleFlashLocation"));
	StunCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	PickupCollider = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	SetRootComponent(TempGunMesh);
	GunMesh->SetupAttachment(RootComponent);
	StunCollider->SetupAttachment(RootComponent);
	StunCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupCollider->SetupAttachment(RootComponent);
	MuzzleFlashLocation->SetupAttachment(RootComponent);
	StunCollider->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnStunColliderBeginOverlap);

}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	SetLifeTimer();
	StunCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

void ABaseWeapon::SetLifeTimer()
{
	GetWorldTimerManager().SetTimer(LifeTimeHandle, this, &ABaseWeapon::CleanUp, 10.f, false);
}

void ABaseWeapon::PlayPickupSound(){
	if (!PickupSound) return;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupSound, GetActorLocation());
}



void ABaseWeapon::OnStunColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult){
	IDamage* HitActor = Cast<IDamage>(OtherActor);
	if (HitActor && DoOnce) {
		USphereComponent* Head = Cast<USphereComponent>(OtherComp);
		if (Head) {
			HitActor->DamageTaken(true, 100.f, this, FVector::ZeroVector, NAME_None, true);
		}
		else {
			HitActor->DamageTaken(true, ThrownDamage, this);
		}
		DoOnce = false;
		FTimerHandle DoOnceHandle{};
		GetWorldTimerManager().SetTimer(DoOnceHandle, this, &ABaseWeapon::ResetDoOnce, 1.f, false);
	}
}

void ABaseWeapon::ResetDoOnce(){
	DoOnce = true;
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeapon::FireAsLineTrace(FVector Start, FVector End){
	//auto test = GetWorld()->SpawnActor<AActor>(ProjectileToSpawn.Get(), Direction, Rotation);
	FHitResult Hit{};
	//GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Camera);
	GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECollisionChannel::ECC_Camera, FCollisionShape::MakeSphere(BulletRadius));
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), GunSound, GetActorLocation());
	//DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 0.5f);
	if (Hit.bBlockingHit) {
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, BulletRadius, 10, FColor::Blue);
		IDamage* HitActorHasInterface = Cast<IDamage>(Hit.GetActor());
		if (HitActorHasInterface) {
			if (HitSound != nullptr && GetWorld() != nullptr) {
				UGameplayStatics::PlaySound2D(GetWorld(), HitSound, 2.f);
			}
			USphereComponent* Head = Cast<USphereComponent>(Hit.GetComponent());
			if (Head) {
				GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Red, "HeadShot", false);
				FirePower = 100.f;
				HitActorHasInterface->DamageTaken(false, FirePower, OwningActor, Hit.ImpactPoint, Hit.BoneName, true);
			}
			else{
				GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Red, "Bodyshot", false);
				FirePower = 35.f;
				HitActorHasInterface->DamageTaken(false, FirePower, OwningActor, Hit.ImpactPoint, Hit.BoneName);
			}
			//GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Red, Hit.GetComponent()->GetName(), false);
			//Spawn Blood VFX.
			if (BloodSplatter != nullptr && GetWorld() != nullptr) {
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BloodSplatter, Hit.ImpactPoint, Hit.ImpactNormal.ToOrientationRotator());
			}
			
		}
		else {
			UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BulletImpact, FVector(4, 16, 16), Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), 10.f);
			if (BulletSparks != nullptr && GetWorld() != nullptr) {
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BulletSparks, Hit.ImpactPoint, Hit.ImpactNormal.ToOrientationRotator());
			}
			
		}
	}

	if (MuzzleFlash != nullptr && GetWorld() != nullptr) {
		UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlash, MuzzleFlashLocation, NAME_None, MuzzleFlashLocation->GetComponentLocation(), MuzzleFlashLocation->GetComponentRotation(), EAttachLocation::KeepWorldPosition, true);
	}
}

void ABaseWeapon::CleanUp(){
	Destroy();
}

