// Fill out your copyright notice in the Description page of Project Settings.


#include "AdrenCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UAdrenCharacterMovementComponent::UAdrenCharacterMovementComponent() {

}



//If the new move is similar enough to the current move, we can combine the moves to save bandwidth.
bool UAdrenCharacterMovementComponent::FSavedMove_Adren::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_Adren* NewAdrenMove = static_cast<FSavedMove_Adren*>(NewMove.Get());

	if (Saved_bWantsToCrouch != NewAdrenMove->Saved_bWantsToCrouch) {
		return false;
	}

	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

//Clears the movement state.
void UAdrenCharacterMovementComponent::FSavedMove_Adren::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToCrouch = 0;
}

//Returns our custom compressed flags.
uint8 UAdrenCharacterMovementComponent::FSavedMove_Adren::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToCrouch) Result = !FLAG_Custom_0;

	return Result;
}

//Look through all safe movement variables and set their respective safe variables.
void UAdrenCharacterMovementComponent::FSavedMove_Adren::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UAdrenCharacterMovementComponent* CharacterMovement = Cast<UAdrenCharacterMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToCrouch = CharacterMovement->Safe_bWantsToCrouch;
}

//Takes data in the saved moved and apply it to the saved move. (reverse of SetMoveFor)
void UAdrenCharacterMovementComponent::FSavedMove_Adren::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UAdrenCharacterMovementComponent* CharacterMovement = Cast<UAdrenCharacterMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToCrouch = Saved_bWantsToCrouch;

}

UAdrenCharacterMovementComponent::FNetworkPredictionData_Client_Adren::FNetworkPredictionData_Client_Adren(const UCharacterMovementComponent& ClientMovement) 
: Super(ClientMovement)
{
}

FSavedMovePtr UAdrenCharacterMovementComponent::FNetworkPredictionData_Client_Adren::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Adren());
}

//Tell the engine we want to use our prediction data client over theirs.
FNetworkPredictionData_Client* UAdrenCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

		//workaround to make a const variable from the character movement componet that is non-const.
		if (ClientPredictionData == nullptr) {
			UAdrenCharacterMovementComponent* MutableThis = const_cast<UAdrenCharacterMovementComponent*>(this);

			MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Adren(*this);
			MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
			MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
		}

	return ClientPredictionData;
}

void UAdrenCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	
	Safe_bWantsToCrouch = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UAdrenCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
}
