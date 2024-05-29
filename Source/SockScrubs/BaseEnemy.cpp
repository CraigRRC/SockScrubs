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
	switch (EnemyState)
	{
	case EnemyState::Ready:
		GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Black, "Ready");
		break;
	case EnemyState::Activated:
		GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Black, "Activated");
		break;
	case EnemyState::Combat:
		GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Black, "Combat");
		break;
	case EnemyState::Stunned:
		break;
	default:
		break;
	}
}

void ABaseEnemy::LookAtPlayer(){
	if (Player == nullptr) return;
	float SightRadius{ 1.f };
	float SightDistance{ 10000.f };
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
	GEngine->AddOnScreenDebugMessage(5, 5.f, FColor::Black, FString::Printf(TEXT("Distance to Target: %.2f"), DistanceToTarget));
	DrawDebugLine(GetWorld(), GetActorLocation(), PlayerLocation, FColor::Red);

	if (DistanceToTarget < ActivationRadius) {
		EnemyState = EnemyState::Activated;
		EnemyStateDelegate.ExecuteIfBound();
	}
	else {
		EnemyState = EnemyState::Ready;
		EnemyStateDelegate.ExecuteIfBound();
	}
}

// Called every frame
void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (SeenPlayer != nullptr) {
		EnemyState = EnemyState::Combat;
		EnemyStateDelegate.ExecuteIfBound();
	}
	else {
		EnemyState = EnemyState::Activated;
		EnemyStateDelegate.ExecuteIfBound();
	}
	switch (EnemyState)
	{
	case EnemyState::Ready:
		if (!GetWorldTimerManager().IsTimerActive(DistHandle)) {
			GetWorldTimerManager().SetTimer(DistHandle, this, &ABaseEnemy::CalcDistBtwnPlayer, 1.f, false);
		}
		break;
	case EnemyState::Activated:
		if (!GetWorldTimerManager().IsTimerActive(DistHandle)) {
			GetWorldTimerManager().SetTimer(DistHandle, this, &ABaseEnemy::CalcDistBtwnPlayer, 1.f, false);
		}
		RotateTowardPlayer();
		LookAtPlayer();
		break;
	case EnemyState::Combat:
		RotateTowardPlayer();
		LookAtPlayer();
		break;
	case EnemyState::Stunned:
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

