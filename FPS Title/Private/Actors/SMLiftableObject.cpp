// Fill out your copyright notice in the Description page of Project Settings.
#include "Actors/SMLiftableObject.h"
#include "Possessables/SMPlayerCharacter.h"

// Sets default values
ASMLiftableObject::ASMLiftableObject()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	SetRootComponent(RootComponent);
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Weight = 500.f;
	LiftHeight = 80;
}

// Called every frame
void ASMLiftableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ContributingPlayersInputs.Num() > 0)
	{
		float PlayerMovementSpd = 0;
		for (TPair<TObjectPtr<ASMPlayerCharacter>, FVector2D>& PlayerInput : ContributingPlayersInputs)
		{
			const FVector2D InputValue = PlayerInput.Key->CurrentMovementInput;
            
			const FVector WorldInput = 
			(PlayerInput.Key->GetActorForwardVector() * InputValue.X) +  // Forward/Backward mapped to X-axis input
			(PlayerInput.Key->GetActorRightVector() * InputValue.Y);     // Right/Left mapped to Y-axis input

			// Convert the world input (FVector) back into FVector2D for X and Y plane movement
			PlayerInput.Value = FVector2D(WorldInput.X, WorldInput.Y);

			// Optionally get the player's movement speed (assuming all players have the same speed)
			PlayerMovementSpd = PlayerInput.Key->GetMovementSpeed();
		}

		FVector2D MovementDirection = CalculateMovementDirection();
		if (!MovementDirection.IsNearlyZero())
		{
			MovementDirection.Normalize();
			FVector CurrentLocation = GetActorLocation();
			CurrentLocation.X += MovementDirection.X * DeltaTime * PlayerMovementSpd;  
			CurrentLocation.Y += MovementDirection.Y * DeltaTime * PlayerMovementSpd;  
			SetActorLocation(CurrentLocation, true); 
		}
	}
}
FORCEINLINE FVector2D ASMLiftableObject::CalculateMovementDirection() const
{
	FVector2D TotalForce = FVector2D::ZeroVector;
	// Sum the input directions from all contributing players
	for (const TPair<TObjectPtr<ASMPlayerCharacter>, FVector2D>& PlayerInput : ContributingPlayersInputs)
	{
		TotalForce += PlayerInput.Value;
	}
	// If players are pushing in opposite directions, this will cancel out the forces
	//Add MinTotal Force
	return TotalForce;

	//Code for 2 Players
	/*if (ContributingPlayersInputs.Num() < 2)
	{
		return TotalForce;  // Not enough players to determine direction coordination
	}

	// Retrieve the first two players' inputs to compare their directions
	auto PlayerIterator = ContributingPlayersInputs.CreateConstIterator();
	const FVector2D FirstPlayerInput = PlayerIterator->Value;
	++PlayerIterator;
	const FVector2D SecondPlayerInput = PlayerIterator->Value;

	// Normalize the input vectors for direction comparison
	FVector2D NormalizedFirstInput = FirstPlayerInput.GetSafeNormal();
	FVector2D NormalizedSecondInput = SecondPlayerInput.GetSafeNormal();

	// Calculate the dot product between the two input vectors
	float DotProduct = FVector2D::DotProduct(NormalizedFirstInput, NormalizedSecondInput);

	// Define a threshold to consider the movement direction similar
	const float SimilarDirectionThreshold = 0.8f;

	// If players are moving in roughly the same direction, sum their forces
	if (DotProduct > SimilarDirectionThreshold)
	{
		TotalForce = FirstPlayerInput + SecondPlayerInput;
	}

	// If the directions are not similar enough, the object won't move (TotalForce remains Zero)
	return TotalForce;*/
}

void ASMLiftableObject::LiftObject()
{
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Z += LiftHeight;
	SetActorLocation(CurrentLocation, true);
	//StaticMeshComponent->SetEnableGravity(false);
	bIsLifted = true;
}

void ASMLiftableObject::DropObject()
{
	FVector CurrentLocation = GetActorLocation();
	//StaticMeshComponent->SetEnableGravity(true);
	CurrentLocation.Z -= LiftHeight;  // Move the object back to the ground
	SetActorLocation(CurrentLocation);
	bIsLifted = false;
}



void ASMLiftableObject::AddPlayerInteraction(ASMPlayerCharacter* Player)
{
	if (!ContributingPlayersInputs.Contains(Player)) {
		ContributingPlayersInputs.Add(Player, FVector2D::ZeroVector);
		if (!bIsLifted)
		{
			LiftObject();
		}
	}
}

void ASMLiftableObject::RemovePlayerInteraction(ASMPlayerCharacter* Player)
{
	if (ContributingPlayersInputs.Contains(Player))
	{
		// Assuming players contribute a fixed amount of force
		ContributingPlayersInputs.Remove(Player);
		if (ContributingPlayersInputs.Num() == 0)
		{
			DropObject();
		}
	}
}


