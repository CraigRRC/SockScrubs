// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemy.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "AdrenCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	TempBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	TempBodyMesh->SetupAttachment(Root);
	BodyHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyHitbox"));
	BodyHitbox->SetupAttachment(Root);
	HeadHitbox = CreateDefaultSubobject<USphereComponent>(TEXT("HeadHitbox"));
	HeadHitbox->SetupAttachment(Root);
	TempHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	TempHeadMesh->SetupAttachment(Root);
	TempGunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	TempGunMesh->SetupAttachment(Root);
	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawn"));
	ProjectileSpawnPoint->SetupAttachment(TempGunMesh);
}

// Called when the game starts or when spawned
void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	EnemyStateDelegate.BindUObject(this, &ABaseEnemy::SwitchState);
	if (GetWorld() && UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) != nullptr) {
		Player = CastChecked<AAdrenCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		
	}
	
}

void ABaseEnemy::SwitchState(){
	GEngine->AddOnScreenDebugMessage(6, 5.f, FColor::Black, "Pew");
	switch (EnemyState)
	{
	case EnemyState::Ready:
		break;
	case EnemyState::Activated:
		break;
	case EnemyState::Combat:
		break;
	case EnemyState::Stunned:
		break;
	default:
		break;
	}
}

void ABaseEnemy::LookAtPlayer(){

}

void ABaseEnemy::CalcDistBtwnPlayer(){
	if (Player == nullptr) return;

	FVector PlayerLocation = Player->GetActorLocation();
	FVector Target = PlayerLocation - GetActorLocation();
	double DistanceToTarget = Target.SizeSquared2D();
	GEngine->AddOnScreenDebugMessage(5, 5.f, FColor::Black, FString::Printf(TEXT("Distance to Target: %.2f"), DistanceToTarget));
	DrawDebugLine(GetWorld(), GetActorLocation(), PlayerLocation, FColor::Red);

	if (DistanceToTarget < ActivationRadius) {
		EnemyState = EnemyState::Activated;
		EnemyStateDelegate.ExecuteIfBound();
	}
}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalcDistBtwnPlayer();
}

// Called to bind functionality to input
void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

