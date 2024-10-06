// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "AdrenCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "BaseWeapon.h"
#include "Components/WidgetComponent.h"
#include "EnemyHealthWidget.h"
#include "Components/PointLightComponent.h"
#include "AdrenGameMode.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	EnemyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EnemyMesh"));
	EnemyMesh->SetupAttachment(Root);
	EnemyMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	//BodyHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyHitbox"));
	//BodyHitbox->SetupAttachment(EnemyMesh);
	//BodyHitbox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	HeadHitbox = CreateDefaultSubobject<USphereComponent>(TEXT("HeadHitbox"));
	HeadHitbox->SetupAttachment(EnemyMesh);
	HeadHitbox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	TempGunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthWidgetComponent->SetupAttachment(EnemyMesh);
	HealthWidgetComponent->SetVisibility(false);
	HealthWidgetComponent->SetComponentTickEnabled(false);
	TempGunMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	IntentionHint = CreateDefaultSubobject<UPointLightComponent>(TEXT("EyeGlare"));
	IntentionHint->SetupAttachment(EnemyMesh);
	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawn"));
	ProjectileSpawnPoint->SetupAttachment(TempGunMesh);
	
}

// Called when the game starts or when spawned
void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	AAdrenGameMode* GameMode = Cast<AAdrenGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode) {
		GameMode->BindEnemyEliminated(this);
	}
	EnemyStateDelegate.BindUObject(this, &ABaseEnemy::SwitchState);
	//EnemyMesh->OnComponentHit.AddDynamic(this, &ABaseEnemy::OnBodyHit);
	EnemyWeaponStateDelegate.BindUObject(this, &ABaseEnemy::SwitchWeaponState);
	if (GetWorld() && UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) != nullptr) {
		Player = CastChecked<AAdrenCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}
	HealthWidget = Cast<UEnemyHealthWidget>(HealthWidgetComponent->GetWidget());
	if (HealthWidget) {
		HealthWidget->SetHealthPercent(ConvertHealthToPercent());
	}
	if (EnemyMesh != nullptr) {
		EnemyMesh->SetComponentTickEnabled(false);
	}
}

void ABaseEnemy::Destroyed(){
	Super::Destroyed();
	if (EnemyStateDelegate.IsBound()) {
		EnemyStateDelegate.Unbind();
	}
	if (EnemyWeaponStateDelegate.IsBound()) {
		EnemyWeaponStateDelegate.Unbind();
	}
}

float ABaseEnemy::ConvertHealthToPercent(){
	return Health / MaxHealth;
}

void ABaseEnemy::DamageTaken(bool Stun, float DamageDelta, AActor* DamageDealer, FVector ImpactPoint, FName BoneName, bool Headshot, bool Tripped, bool Kicked) {
	if (EnemyState == EEnemyState::Dead) return;
	
	if (Stun) {
		EnemyState = EEnemyState::Stunned;
		EnemyStateDelegate.ExecuteIfBound();
		EnemyMesh->SetSimulatePhysics(true);
	}
	Health -= DamageDelta;
	if (HealthWidget) {
		HealthWidget->SetHealthPercent(ConvertHealthToPercent());
	}

	if (Tripped) {
		EnemyMesh->SetSimulatePhysics(true);
		EnemyMesh->AddImpulse(DamageDealer->GetActorForwardVector() * 10000.f + DamageDealer->GetVelocity(), FName("RightLeg"), true);
		EnemyMesh->AddImpulse(DamageDealer->GetActorForwardVector() * 10000.f + DamageDealer->GetVelocity(), FName("LeftLeg"), true);
	}
	//Temp
	if (bDiedToKillZ) {
		EnemyEliminatedDelegate.Execute(this, 5.f);
	}
	
	float ClampedHealth = FMath::Clamp(Health, 0, MaxHealth);
	if (ClampedHealth <= 0.f) {
		EnemyState = EEnemyState::Dead;
		EnemyStateDelegate.ExecuteIfBound();
		if (BloodSplat != nullptr) {
			FHitResult BloodSplatterHit{};
			GetWorld()->LineTraceSingleByChannel(BloodSplatterHit, GetActorLocation() + FVector::UpVector * 180.f, (GetActorLocation() + FVector::UpVector * 180.f) + GetActorForwardVector() * -1000.f, ECollisionChannel::ECC_Visibility);
			if (BloodSplatterHit.bBlockingHit) {
				UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodSplat, BloodSplatSize, BloodSplatterHit.ImpactPoint, FRotator(BloodSplatterHit.ImpactNormal.Rotation().Pitch, BloodSplatterHit.ImpactNormal.Rotation().Yaw, FMath::RandRange(0, 360)), 10.f);
			}
		}
		
		if (!Headshot) {
			if (DeathSound) {
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation(), SFXVolume, 1.0f);
			}
			if (Kicked) {
				EnemyMesh->SetSimulatePhysics(true);
				EnemyMesh->AddImpulse(DamageDealer->GetActorForwardVector() * 40000.f + DamageDealer->GetVelocity(), FName("Spine"), true);
				EnemyEliminatedDelegate.Execute(this, 3.f);
			}
			else {
				//Temp
				if (!bDiedToKillZ) {
					EnemyMesh->SetSimulatePhysics(true);
					EnemyMesh->AddImpulse(DamageDealer->GetActorForwardVector() * 5000.f, BoneName, true);
					EnemyEliminatedDelegate.Execute(this, 1.f);
				}
			}
		}
	}
	if (Headshot) {
		EnemyEliminatedDelegate.Execute(this, 2.f);
		EnemyMesh->SetSimulatePhysics(true);
		EnemyMesh->AddImpulse(DamageDealer->GetActorForwardVector() * 10000.f, FName("Head"), true);
		if (HeadshotTing && HeadshotAttenuation) {
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), HeadshotTing, GetActorLocation(), SFXVolume, 1.0f);
		}
	}
}


void ABaseEnemy::SwitchState(){
	switch (EnemyState)
	{
	case EEnemyState::Ready:
		HealthWidgetComponent->SetVisibility(false);
		EnemyMesh->SetNotifyRigidBodyCollision(false);
		break;
	case EEnemyState::Activated:
		HealthWidgetComponent->SetVisibility(false);
		EnemyMesh->SetNotifyRigidBodyCollision(false);
		break;
	case EEnemyState::Combat:
		if (GetWorldTimerManager().IsTimerActive(DistHandle)) {
			GetWorldTimerManager().ClearTimer(DistHandle);
		}
		HealthWidgetComponent->SetComponentTickEnabled(true);
		HealthWidgetComponent->SetVisibility(true);
		EnemyMesh->SetNotifyRigidBodyCollision(true);
		EnemyMesh->SetComponentTickEnabled(true);
		break;
	case EEnemyState::Stunned:
		bIsStunned = true;
		if (GetWorldTimerManager().IsTimerActive(FireHandle)) {
			GetWorldTimerManager().ClearTimer(FireHandle);
		}
		if (GetWorldTimerManager().IsTimerActive(DistHandle)) {
			GetWorldTimerManager().ClearTimer(DistHandle);
		}
		HealthWidgetComponent->SetComponentTickEnabled(true);
		HealthWidgetComponent->SetVisibility(true);
		//Turn off the weapon visiblility
		TempGunMesh->SetVisibility(false);
		//Spawn a Rifle
		if (WeaponToSpawnWhenDropped && EnemyWeaponState == EEnemyWeaponState::Armed) {
			DropEquippedWeapon();
		}

		//Set Timer to have the enemy come back from stun.
		GetWorldTimerManager().SetTimer(StunDuration, this, &ABaseEnemy::ReturnFromStunState, 4.f, false);
		break;
	case EEnemyState::Dead:
		bIsDead = true;
		TempGunMesh->SetVisibility(false);
		HealthWidgetComponent->SetVisibility(false);
		EnemyMesh->SetNotifyRigidBodyCollision(false);
		EnemyMesh->PutAllRigidBodiesToSleep();
		EnemyMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		EnemyMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Ignore);
		EnemyMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		HeadHitbox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		HeadHitbox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Ignore);
		HeadHitbox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		//This is for the memes.
		//UGameplayStatics::PlaySound2D(GetWorld(), DeathSounds[FMath::RandRange(0, DeathSounds.Num() - 1)], 10.0f, FMath::FRandRange(0.8, 1));
		GetWorldTimerManager().ClearAllTimersForObject(this);
		if (EnemyWeaponState == EEnemyWeaponState::Armed) {
			DropEquippedWeapon();
		}
		//Create Timer
		//Play Sound
		if (!GetWorldTimerManager().IsTimerActive(DeathTimer)) {
			GetWorldTimerManager().SetTimer(DeathTimer, this, &ABaseEnemy::CleanUp, 15.f, false);
		}
		break;
	default:
		break;
	}
}

void ABaseEnemy::AllowEnemyToCollide(){
	CollideOnce = true;
}

void ABaseEnemy::CleanUp(){
	Destroy();
}

void ABaseEnemy::DropEquippedWeapon()
{
	if (!WeaponToSpawnWhenDropped) return;
	ABaseWeapon* SpawnedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(WeaponToSpawnWhenDropped, TempGunMesh->GetComponentTransform());
	//Simulate physics on the rifle
	SpawnedWeapon->GetGunMesh()->SetSimulatePhysics(true);
	//Set disarmed
	EnemyWeaponState = EEnemyWeaponState::Disarmed;
	EnemyWeaponStateDelegate.ExecuteIfBound();
}

void ABaseEnemy::SwitchWeaponState(){
	switch (EnemyWeaponState)
	{
	case EEnemyWeaponState::Armed:
		break;
	case EEnemyWeaponState::Disarmed:
		break;
	default:
		break;
	}
}

void ABaseEnemy::ReturnFromStunState(){
	EnemyState = EEnemyState::Activated;
	EnemyStateDelegate.ExecuteIfBound();
}

void ABaseEnemy::LookAtPlayer(){
	if (Player == nullptr) return;
	float SightRadius{ 1.f };
	float SightDistance{ 5000.f };
	const TArray<AActor*> Empty{};
	TArray<FHitResult> HitResults{};
	UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetActorLocation() + FVector::UpVector * 185.f, GetActorLocation() + FVector::UpVector * 185.f + GetActorForwardVector() * SightDistance, SightRadius, ETraceTypeQuery::TraceTypeQuery1, false, Empty, EDrawDebugTrace::None, HitResults, true);
	for (FHitResult const &hit : HitResults) {
		if (hit.bBlockingHit) {
			if (Cast<AAdrenCharacter>(hit.GetActor())) {
				SeenPlayer = Cast<AAdrenCharacter>(hit.GetActor());
				if (EnemyWeaponState != EEnemyWeaponState::Disarmed) {
					EnemyState = EEnemyState::Combat;
					EnemyStateDelegate.ExecuteIfBound();
				}
			}
			else {
				SeenPlayer = nullptr;
				EnemyState = EEnemyState::Activated;
				EnemyStateDelegate.ExecuteIfBound();
				if (GetWorldTimerManager().IsTimerActive(FireHandle)) {
					GetWorldTimerManager().ClearTimer(FireHandle);
				}
			}
		}
	}
}

void ABaseEnemy::RotateTowardPlayer(){
	if (Player == nullptr) return;
	SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation() + FVector::UpVector * 140.f, Player->GetActorLocation()));
}

void ABaseEnemy::CalcDistBtwnPlayer(){
	if (Player == nullptr) return;

	FVector PlayerLocation = Player->GetActorLocation();
	FVector Target = PlayerLocation - GetActorLocation();
	double DistanceToTarget = Target.SizeSquared2D();
	//GEngine->AddOnScreenDebugMessage(5, 5.f, FColor::Black, FString::Printf(TEXT("Distance to Target: %.2f"), DistanceToTarget), false);

	if (DistanceToTarget < ActivationRadius) {
		EnemyState = EEnemyState::Activated;
		EnemyStateDelegate.ExecuteIfBound();
	}
	else {
		EnemyState = EEnemyState::Ready;
		EnemyStateDelegate.ExecuteIfBound();
	}
}

void ABaseEnemy::Fire_Implementation(){

}

//Buggy 
//void ABaseEnemy::OnBodyHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit){
//	if (EnemyState == EEnemyState::Dead) return;
//	if (!CollideOnce) return;
//	if (EnemyMesh->IsAnySimulatingPhysics()) {
//		if (EnemyMesh->GetComponentVelocity().SquaredLength() >= 90000.f && EnemyMesh->GetComponentVelocity().SquaredLength() <= 120000.f) {
//			DamageTaken(false, 20.f, this, FVector::ZeroVector, NAME_None, false, false, false);
//			GetWorldTimerManager().SetTimer(EnemySlammedIntoWallHandle, this, &ABaseEnemy::AllowEnemyToCollide, 0.1f, false);
//			CollideOnce = false;
//		}
//	}
//}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (EnemyMesh->IsAnySimulatingPhysics()) {
		EnemyMesh->AddForce(FVector::DownVector * 5000.f, NAME_None, true);
	}

	switch (EnemyState)
	{
	case EEnemyState::Ready:
		if (!GetWorldTimerManager().IsTimerActive(DistHandle)) {
			GetWorldTimerManager().SetTimer(DistHandle, this, &ABaseEnemy::CalcDistBtwnPlayer, 0.01f, false);
		}
		break;
	case EEnemyState::Activated:
		if (!GetWorldTimerManager().IsTimerActive(DistHandle)) {
			GetWorldTimerManager().SetTimer(DistHandle, this, &ABaseEnemy::CalcDistBtwnPlayer, 5.f, false);
		}
		RotateTowardPlayer();
		LookAtPlayer();
		break;
	case EEnemyState::Combat:
		RotateTowardPlayer();
		LookAtPlayer();
		if (!GetWorldTimerManager().IsTimerActive(FireHandle)) {
			GetWorldTimerManager().SetTimer(FireHandle, this, &ABaseEnemy::Fire, FMath::FRandRange(RateOfFire, RateOfFire + 1.0f), false);
		}
		if (GetWorldTimerManager().IsTimerActive(FireHandle)) {
			if (GetWorldTimerManager().GetTimerRemaining(FireHandle) <= 0.6) {
				IntentionHint->SetIntensity(GetWorldTimerManager().GetTimerElapsed(FireHandle));
			}
			else {
				IntentionHint->SetIntensity(0);
			}
		}
		break;
	case EEnemyState::Stunned:
		break;
	case EEnemyState::Dead:
		return;
		break;
	default:
		break;
	}

	
}

// Called to bind functionality to input
void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

