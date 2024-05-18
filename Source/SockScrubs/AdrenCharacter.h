// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AdrenCharacter.generated.h"

UCLASS()
class SOCKSCRUBS_API AAdrenCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAdrenCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Movement)
	class UAdrenCharacterMovementComponent* AdrenCharacterMovementComponent;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	class UInputAction* IA_Crouch;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Kick;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_ActivateSloMo;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Shoot;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_Throw;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* IA_StartRun;

	void Look(const struct FInputActionInstance& Instance);
	void Move(const struct FInputActionInstance& Instance);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
