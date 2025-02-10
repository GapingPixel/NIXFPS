// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "SMSpawnMaster.generated.h"

class UInputMappingContext;
class USpringArmComponent;
class USMAbilitySystemComponent;
class USMNIXAttributeSet;

UCLASS()
class ASMSpawnMaster : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASMSpawnMaster();

	virtual void PostInitializeComponents() override;
	virtual void PawnClientRestart() override;

	/* Blueprint Exposed
	***********************************************************************************/

protected:

	// Use traditional spectator-like free roam movement instead of locking the movement to a flat plane.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=SpawnMasterPawn)
	bool bFreeLookMovement = false;

	// Default mapping contexts that this pawn will grant to the owning clients Player Controller
	UPROPERTY(EditDefaultsOnly, Category = Character)
	TArray<UInputMappingContext*> DefaultInputMappingContexts;
	
	/* Components
	***********************************************************************************/
	
protected:

	// First Person Camera for the Spawn Master.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=SpawnMasterPawn)
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=SpawnMasterPawn)
	USpringArmComponent* CameraController;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=SpawnMasterPawn)
	USphereComponent* SphereComponent;

	/* GAS
	***********************************************************************************/

public:
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=SpawnMasterPawn)
	USMAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleDefaultsOnly, Category=SpawnMasterPawn)
	USMNIXAttributeSet* SpawnMasterAttributeSet;
	
	/* Meshes
	***********************************************************************************/

	// The first person hands for the Spawn Master.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=SpawnMasterPawn)
	USkeletalMeshComponent* ArmsMesh1P;
};
