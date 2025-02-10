// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SMCharacterMovementComponent.generated.h"

class ASMBaseCharacter;
class USMAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class NIX_API USMCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	friend class FSavedMove_My;

public:
	// Sets sprinting to either enabled or disabled
	void SetSprinting(bool bSprinting);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;
	
private:
	// The ground speed when sprinting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Walking", Meta = (AllowPrivateAccess = "true"))
	float SprintSpeedMultiplier = 1.4f;

	// The acceleration when sprinting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Walking", Meta = (AllowPrivateAccess = "true"))
	float SprintAcceleration = 2400.f;

	// True if the sprint key is down
	bool bSprintKeyDown = false;

	void SetWantsToSprint(bool bNewWantsToSprint);
	uint8 bWantsToSprint : 1;
	UPROPERTY()
	TObjectPtr<ASMBaseCharacter> Char;

protected:

	
	TObjectPtr<USMAbilitySystemComponent> OwningASC;

	USMAbilitySystemComponent* GetASC();

	virtual void BeginPlay() override;
	
};

class FSavedMove_My : public FSavedMove_Character
{
public:

	typedef FSavedMove_Character Super;

	// Resets all saved variables.
	virtual void Clear() override;
	// Store input commands in the compressed flags.
	virtual uint8 GetCompressedFlags() const override;
	// This is used to check whether or not two moves can be combined into one.
	// Basically you just check to make sure that the saved variables are the same.
	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const override;
	// Sets up the move before sending it to the server. 
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	// Sets variables on character movement component before making a predictive correction.
	virtual void PrepMoveFor(class ACharacter* Character) override;

private:
	uint8 SavedWantsToSprint : 1;
};

class FNetworkPredictionData_Client_My : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	// Constructor
	FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement);

	//brief Allocates a new copy of our custom saved move
	virtual FSavedMovePtr AllocateNewMove() override;
};
