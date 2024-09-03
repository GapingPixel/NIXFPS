// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "SMPawnComponent.generated.h"

class USMAbilitySystemComponent;
/**
 * This component should be able to attach to any pawn that can be possessed by a player or AI.
 * It's designed so that the pawn in question should be possessed by something that also has a player state
 * regardless of it being AI or player.
 */
UCLASS()
class NIX_API USMPawnComponent : public UPawnComponent
{
	GENERATED_BODY()

public:

	USMPawnComponent(const FObjectInitializer& ObjectInitializer);

#pragma region GAS

	/* GAS
	***********************************************************************************/
	
protected:
	
	virtual void OnRegister() override;

	// Ability System Component pointer cached for convenience. The real ASC should be living on the PlayerState.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = GAS)
	USMAbilitySystemComponent* AbilitySystemComponent;

public:

	// Should be called by this components owning pawn to setup the ASC. The pawn must have a PlayerState (regardless if AI or player). 
	void InitializeAbilitySystemComponent(USMAbilitySystemComponent* InASC, AActor* OwnerActor);

	// In the case that for example we need to unpossess a character and possess another, we need to start thinking about uninitializing the ability system.
	void UninitializeAbilitySystemComponent();

	// Gets the SM Ability System Component.
	UFUNCTION(BlueprintPure, Category = GAS)
	USMAbilitySystemComponent* GetSMAbilitySystemComponent() const { return AbilitySystemComponent; };

	// Register with the OnAbilitySystemInitialized delegate and broadcast if condition is already met.
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);
	
	// Register with the OnAbilitySystemUninitialized delegate.
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);
	
	// Delegate fired when our pawn becomes the ability system's avatar actor
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	// Delegate fired when our pawn is removed as the ability system's avatar actor
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

#pragma endregion GAS

public:
	// Returns the pawn extension component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Spawn Master|Pawn")
	static USMPawnComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<USMPawnComponent>() : nullptr); }
	
};
