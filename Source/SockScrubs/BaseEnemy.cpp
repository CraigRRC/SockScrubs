// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "AdrenCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "BaseWeapon.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	TempBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	TempBodyMesh->SetupAttachment(Root);
	TempBodyMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	BodyHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyHitbox"));
	BodyHitbox->SetupAttachment(Root);
	BodyHitbox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	HeadHitbox = CreateDefaultSubobject<USphereComponent>(TEXT("HeadHitbox"));
	HeadHitbox->SetupAttachment(Root);
	HeadHitbox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	TempHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	TempHeadMesh->SetupAttachment(Root);
	TempHeadMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	TempGunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	TempGunMesh->SetupAttachment(Root);
	TempGunMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawn"));
	ProjectileSpawnPoint->SetupAttachment(TempGunMesh);
	
}

// Called when the game starts or when spawned
void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	EnemyStateDelegate.BindUObject(this, &ABaseEnemy::SwitchState);
	EnemyWeaponStateDelegate.BindUObject(this, &ABaseEnemy::SwitchWeaponState);
	if (GetWorld() && UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) != nullptr) {
		Player = CastChecked<AAdrenCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}
}

void ABaseEnemy::Destroyed(){
	EnemyStateDelegate.Unbind(); 
	EnemyWeaponStateDelegate.Unbind();
}

void ABaseEnemy::DamageTaken(bool Stun, float DamageDelta, AActor* DamageDealer){
	GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Magenta, "EnemyDamaged", false);
	if (Stun) {
		EnemyState = EEnemyState::Stunned;
		EnemyStateDelegate.ExecuteIfBound();
	}
	Health -= DamageDelta;
	float ClampedHealth = FMath::Clamp(Health, 0, MaxHealth);
	GEngine->AddOnScreenDebugMessage(5, 5.f, FColor::Black, FString::Printf(TEXT("HP: %.2f"), ClampedHealth));
	if (ClampedHealth <= 0.f) {
		EnemyState = EEnemyState::Dead;
		EnemyStateDelegate.ExecuteIfBound();
	}
	

}

void ABaseEnemy::SwitchState(){
	switch (EnemyState)
	{
	case EEnemyState::Ready:
		GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Black, "Ready");
		break;
	case EEnemyState::Activated:
		GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Black, "Activated");
		break;
	case EEnemyState::Combat:
		if (GetWorldTimerManager().IsTimerActive(DistHandle)) {
			GetWorldTimerManager().ClearTimer(DistHandle);
		}
		GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Black, "Combat");
		break;
	case EEnemyState::Stunned:
		if (GetWorldTimerManager().IsTimerActive(FireHandle)) {
			GetWorldTimerManager().ClearTimer(FireHandle);
		}
		if (GetWorldTimerManager().IsTimerActive(DistHandle)) {
			GetWorldTimerManager().ClearTimer(DistHandle);
		}
		
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
		GetWorldTimerManager().ClearAllTimersForObject(this);
		DropEquippedWeapon();
		//Create Timer
		//Play Sound
		//Tell UI that i am no longer living
		Destroy();
		//Something
		break;
		
	default:
		break;
	}
}

void ABaseEnemy::DropEquippedWeapon()
{
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
	UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetActorLocation() + FVector::UpVector * 150.f, GetActorLocation() + GetActorForwardVector() * SightDistance, SightRadius, ETraceTypeQuery::TraceTypeQuery1, false, Empty, EDrawDebugTrace::ForOneFrame, HitResults, true);
	for (FHitResult const &hit : HitResults) {
		if (hit.bBlockingHit) {
			GEngine->AddOnScreenDebugMessage(4, 5.f, FColor::Yellow, hit.GetActor()->GetName());
			if (Cast<AAdrenCharacter>(hit.GetActor())) {
				SeenPlayer = Cast<AAdrenCharacter>(hit.GetActor());
			}
			else {
				SeenPlayer = nullptr;
			}
		}
	}
}

void ABaseEnemy::RotateTowardPlayer(){
	if (Player == nullptr) return;
	SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation() + FVector::UpVector * 100.f, Player->GetActorLocation()));
}

void ABaseEnemy::CalcDistBtwnPlayer(){
	if (Player == nullptr) return;

	FVector PlayerLocation = Player->GetActorLocation();
	FVector Target = PlayerLocation - GetActorLocation();
	double DistanceToTarget = Target.SizeSquared2D();
	GEngine->AddOnScreenDebugMessage(5, 5.f, FColor::Black, FString::Printf(TEXT("Distance to Target: %.2f"), DistanceToTarget), false);
	DrawDebugLine(GetWorld(), GetActorLocation(), PlayerLocation, FColor::Red);

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

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	

	switch (EnemyState)
	{
	case EEnemyState::Ready:
		if (!GetWorldTimerManager().IsTimerActive(DistHandle)) {
			GetWorldTimerManager().SetTimer(DistHandle, this, &ABaseEnemy::CalcDistBtwnPlayer, 1.f, false);
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
			GetWorldTimerManager().SetTimer(FireHandle, this, &ABaseEnemy::Fire, RateOfFire, false);
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

	if (SeenPlayer != nullptr && EnemyWeaponState != EEnemyWeaponState::Disarmed) {
		EnemyState = EEnemyState::Combat;
		EnemyStateDelegate.ExecuteIfBound();
	}

	//GEngine->AddOnScreenDebugMessage(5, 5.f, FColor::Black, FString::Printf(TEXT("Distance to Target: %.2f"), GetWorldTimerManager().GetTimerRemaining(StunDuration)));
}

// Called to bind functionality to input
void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

