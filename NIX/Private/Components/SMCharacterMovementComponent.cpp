// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SMCharacterMovementComponent.h"

#include "GameFramework/Character.h"
#include "Possessables/SMBaseCharacter.h"

void USMCharacterMovementComponent::SetSprinting(bool bSprinting)
{
	bSprintKeyDown = bSprinting;
}

void USMCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// Peform local only checks
	if (GetPawnOwner()->IsLocallyControlled())
	{
		if (bSprintKeyDown == true)
		{
			// Only set WantsToSprint to true if the player is moving forward (so that he can't sprint backwards)
			FVector Velocity2D = GetPawnOwner()->GetVelocity();
			FVector Forward2D = GetPawnOwner()->GetActorForwardVector();
			Velocity2D.Z = 0.0f;
			Forward2D.Z = 0.0f;
			Velocity2D.Normalize();
			Forward2D.Normalize();

			const bool bIsGoingForward = FVector::DotProduct(Velocity2D, Forward2D) > 0.5f;
			SetWantsToSprint(bIsGoingForward && !IsFalling());
		}
		else
		{
			SetWantsToSprint(false);
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USMCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	/*  There are 4 custom move flags for us to use. Below is what each is currently being used for:
	FLAG_Custom_0		= 0x10, // Sprinting
	FLAG_Custom_1		= 0x20, // Unused
	FLAG_Custom_2		= 0x40, // Unused
	FLAG_Custom_3		= 0x80, // Unused
*/

	// Read the values from the compressed flags
	bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	if (bWantsToSprint)
	{
		bIsSprinting = true;
	}
	else
	{
		bIsSprinting = false;
	}
	Char->SetSprintFromMovementComponent(bIsSprinting);
}

float USMCharacterMovementComponent::GetMaxSpeed() const
{
	ASMBaseCharacter* Owner = Cast<ASMBaseCharacter>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() No Owner"), *FString(__FUNCTION__));
		return Super::GetMaxSpeed();
	}

	// @TODO: if is not alive then return 0.0f (right here)
	
	if (IsCrouching())
	{
		return MaxWalkSpeedCrouched;
	}
	else
	{
		if (bWantsToSprint)
		{
			return Owner->GetMovementSpeed() * SprintSpeedMultiplier;
		}
		else
		{
			return Owner->GetMovementSpeed();
		}
	}
	
//	return Owner->GetMovementSpeed();
}

float USMCharacterMovementComponent::GetMaxAcceleration() const
{
	if (IsMovingOnGround() && bWantsToSprint)
	{
		return SprintAcceleration;
	}

	return Super::GetMaxAcceleration();
}

FNetworkPredictionData_Client* USMCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		// Return our custom client prediction class instead
		USMCharacterMovementComponent* MutableThis = const_cast<USMCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_My(*this);
	}

	return ClientPredictionData;
}

void USMCharacterMovementComponent::SetWantsToSprint(bool bNewWantsToSprint)
{
	if (bWantsToSprint != bNewWantsToSprint)
	{
		bWantsToSprint = bNewWantsToSprint;
		bIsSprinting = bNewWantsToSprint;
	}
}

USMAbilitySystemComponent* USMCharacterMovementComponent::GetASC()
{
	if (!OwningASC)
	{
		OwningASC = Char->GetSMAbilitySystemComponent();
	}

	return OwningASC;
}

void USMCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	Char = Cast<ASMBaseCharacter>(GetOwner());
}

void FSavedMove_My::Clear()
{
	Super::Clear();

	// Clear all values
	SavedWantsToSprint = 0;
}

uint8 FSavedMove_My::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	/* There are 4 custom move flags for us to use. Below is what each is currently being used for:
	FLAG_Custom_0		= 0x10, // Sprinting
	FLAG_Custom_1		= 0x20, // Unused
	FLAG_Custom_2		= 0x40, // Unused
	FLAG_Custom_3		= 0x80, // Unused
	*/

	// Write to the compressed flags 
	if (SavedWantsToSprint)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

bool FSavedMove_My::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const
{
	const FSavedMove_My* NewMove = static_cast<const FSavedMove_My*>(NewMovePtr.Get());

	// As an optimization, check if the engine can combine saved moves.
	if (SavedWantsToSprint != NewMove->SavedWantsToSprint)
	{
		return false;
	}

	return Super::CanCombineWith(NewMovePtr, Character, MaxDelta);
}

void FSavedMove_My::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	const USMCharacterMovementComponent* charMov = static_cast<USMCharacterMovementComponent*>(Character->GetCharacterMovement());
	if (charMov)
	{
		// Copy values into the saved move
		SavedWantsToSprint = charMov->bWantsToSprint;
	}
}

void FSavedMove_My::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	USMCharacterMovementComponent* CharMov = Cast<USMCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{
		// Copy values out of the saved move
		CharMov->SetWantsToSprint(SavedWantsToSprint);
	}
}

FNetworkPredictionData_Client_My::FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_My::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_My());
}
