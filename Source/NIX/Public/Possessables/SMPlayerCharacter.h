// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Possessables/SMBaseCharacter.h"
#include "SMPlayerCharacter.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
/**
Comment Last Updated 26/06/22 4:28 PM

The design for this class is for the player class to be able to be a survivor OR a zombie.
We plan to have 2 gamemodes:
 - 1 more PvE related aka zombie master gameplay loop.
 - 1 more PvP related aka zombie panic gameplay loop.

For a player survivor to get infected and turn from survivor into zombie on the spot,
it might be pretty difficult to swap pawns at runtime, and that might be a lot of work.
Building the player character class from the ground up to support changing teams at any point will help support this.
 */

USTRUCT(BlueprintType)
struct FMappingContextWithPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<const UInputMappingContext> MappingContext;

	UPROPERTY(EditAnywhere, Category = Input)
	int32 ContextPriority;
	
};

UCLASS()
class ASMPlayerCharacter : public ASMBaseCharacter
{
	GENERATED_BODY()

public:
	ASMPlayerCharacter(const class FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

	virtual void PerformDeath(AActor*) override;
	
	// Sets whether this character is in first person view or in third person view.
	UFUNCTION(BlueprintCallable, Category = Character)
	virtual void SetThirdPerson(bool bThirdPerson);

	// Toggle third person view.
	UFUNCTION(BlueprintCallable, Category = Character)
	void ToggleThirdPerson();

	

	UFUNCTION(BlueprintCallable, Category = Character)
	void StartCrouch();
	UFUNCTION(BlueprintCallable, Category = Character)
	void StopCrouch();
	bool bIsCrouching;
	float TargetCapsuleHalfHeight;
	float TargetCameraZOffset;
	float CrouchSpeed;
	float DefaultCapsuleHalfHeight;
	float DefaultCameraZOffset;

	FTimerHandle CrouchTimerHandle;
	float CrouchDuration;
	void UpdateCrouch();
	
	UPROPERTY(EditDefaultsOnly, Category="Inputs")
	TObjectPtr<UInputAction> MoveInputAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Inputs")
	TObjectPtr<UInputAction> CrouchInputAction;

	UPROPERTY(EditDefaultsOnly, Category="Inputs")
	TObjectPtr<UInputAction> JumpInputAction;

	UPROPERTY(EditDefaultsOnly, Category="Inputs")
	TObjectPtr<UInputAction> WheelUpInputAction;

	UPROPERTY(EditDefaultsOnly, Category="Inputs")
	TObjectPtr<UInputAction> WheelDownInputAction;

	void ToggleCrouch();
	void OnMoveAction(const FInputActionValue& Value);

	void OnWheelUpAction();
	void OnWheelDownAction();
	UPROPERTY(BlueprintReadOnly)
	float DefaultFOV;
	UPROPERTY(BlueprintReadOnly)
	float FOVLastFrame;

	void SetEquippableFOV();

	UFUNCTION()
	void OnDeath(AActor* OwningActor);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

#pragma region Exposables

protected:

	// Height of the camera in first person from relative ground level.
	UPROPERTY(EditDefaultsOnly, Category = Character)
	float FirstPersonCameraHeight = 80.0f;

	// Default mapping contexts that this character will grant to the owning clients Player Controller
	UPROPERTY(EditDefaultsOnly, Category = Character)
	TArray<TObjectPtr<const UInputMappingContext>> DefaultInputMappingContexts;

#pragma endregion Exposables

#pragma region Components

protected:
	// The view that the player sees.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character)
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character)
	TObjectPtr<USpringArmComponent> CameraController;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character)
	TObjectPtr<USpringArmComponent> ThirdPersonSpringArm;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character)
	TObjectPtr<UCameraComponent> ThirdPersonCamera;

#pragma endregion Components

#pragma region Meshes
	
public:
	
	/* Meshes
	***********************************************************************************/

	// The first person hands mesh that can be viewed when possessing the character in first person.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Meshes)
	TObjectPtr<USkeletalMeshComponent> FirstPersonHandsMesh;

	// The first person legs mesh that can be viewed when possessing the character in first person.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Meshes)
	TObjectPtr<USkeletalMeshComponent> FirstPersonLegsMesh;

#pragma endregion Meshes

#pragma region Getters
	
public:

	/* Get a specific mesh. This is done in one function to avoid creating a bunch of getters. This also makes it cleaner and
	 * easier to understand what meshes there are to get too. */
	UFUNCTION(BlueprintPure, Category = Meshes)
	virtual USkeletalMeshComponent* GetMeshOfType(EMeshType MeshType) override;

	

#pragma endregion Getters

	/* Other
	***********************************************************************************/

private:

	UPROPERTY()
	bool bIsFirstPerson = true;
};
