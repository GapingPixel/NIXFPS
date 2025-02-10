// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Camera/CameraComponent.h"
#include "Possessables/SMBaseCharacter.h"
#include "SMPlayerCharacter.generated.h"

class ASMBarricadeBase;
class UAISenseConfig_Hearing;
class UAIPerceptionComponent;
class UPhysicsHandleComponent;
class ASMInteractableActor;
class UAIPerceptionStimuliSourceComponent;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDownedHealth_Changed, float, OldValue, float, NewValue);

USTRUCT(BlueprintType)
struct FMappingContextWithPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<const UInputMappingContext> MappingContext;

	UPROPERTY(EditAnywhere, Category = Input)
	int32 ContextPriority = 0;
	
};

UCLASS()
class ASMPlayerCharacter : public ASMBaseCharacter , public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ASMPlayerCharacter(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY()
	TObjectPtr<ASMPlayerController> PC;
	
	UPROPERTY()
	TObjectPtr<ASMPlayerState> SMPlayerState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;
	
	void NoiseEvent();
	
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

	virtual void PerformDeath(AActor*) override;
	
	// Sets whether this character is in first person view or in third person view.
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = Character)
	virtual void SetThirdPerson(bool bThirdPerson);

	// Toggle third person view.
	UFUNCTION(BlueprintCallable, Category = Character)
	void ToggleThirdPerson();

	/*Rescue*/
	void RescueAlly();
	
	UPROPERTY(Replicated)
	bool bIsDowned;

	FTimerHandle RescuePlayerHandle;
	UFUNCTION(Client, Reliable)
	void ClientRescuePlayerHandleTimerClear();
	UFUNCTION(Server, Reliable)
	void Rescued();

	UPROPERTY(Replicated)
	FTimerHandle DownedPlayerHPTimer;
	UFUNCTION(Client, Reliable)
	void ClientDownedPlayerHPTimerClear();
	
	void DieForGood();
	
	UPROPERTY()
	TObjectPtr<ASMPlayerCharacter> PlayerToRescue;

	UFUNCTION(BlueprintCallable)
	void TestResetHealth();
	
	UPROPERTY(Replicated)
	int8 PlayerWounds;
	UPROPERTY(Replicated)
	int8 MaxPlayerWounds;
	
	FDownedHealth_Changed OnDownedHealthChanged;
	
	float DownedPlayerMaxHP = 15;
	UPROPERTY(Replicated)
	float DownedPlayerHP = DownedPlayerMaxHP;
	UFUNCTION()
	void RescueSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void RescueSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/*Grab Object*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;
	
	AActor* CanGrabObject();
	
	void DropObject();
	UPROPERTY(Replicated)
	TObjectPtr<AActor> GrabbedObject;
	UPROPERTY(Replicated)
	TObjectPtr<UPrimitiveComponent> GrabbedObjectComponent;
	
	const float GrabbedObjectDistance = 150;

	//Lift Object
	FVector2d CurrentMovementInput;
	
	/*Inputs*/
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

	UPROPERTY(EditDefaultsOnly, Category="Inputs")
	TObjectPtr<UInputAction> RescuePlayerInputAction;
	virtual void Jump() override;
	void ToggleCrouch();
	void OnMoveAction(const FInputActionValue& Value);
	void OnStopMoveAction(const FInputActionValue& Value);
	void OnWheelUpAction();
	void OnWheelDownAction();
	UPROPERTY(BlueprintReadOnly)
	float DefaultFOV;
	UPROPERTY(BlueprintReadOnly)
	float FOVLastFrame;

	void SetEquippableFOV();

	UFUNCTION()
	void OnDowned(AActor* OwningActor);

	FVector OriginalSpawnLocation;
	FRotator OriginalSpawnRotation;

	/*Barricade Logic*/
	bool CanInteractWithBarricadeObject();
	UPROPERTY(replicated)
	TObjectPtr<ASMBarricadeBase> CurrentBarricadeObject;

	UPROPERTY()
	TArray<TObjectPtr<ASMBarricadeBase>> BarricadeObjectsToEvaluate;

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

public:
	// The view that the player sees.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character)
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditDefaultsOnly, Category="CameraShakes")
	TSubclassOf<UCameraShakeBase> ConstantCameraShake;
	UPROPERTY(Replicated)
	bool bLowSanity;
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character)
	TObjectPtr<USpringArmComponent> CameraController;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character)
	TObjectPtr<USpringArmComponent> ThirdPersonSpringArm;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Character)
	TObjectPtr<UCameraComponent> ThirdPersonCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	TObjectPtr<USphereComponent> ReviveArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	UFUNCTION(NetMulticast, Reliable)
	void NotifyClientsOfSound(AActor* SoundSource);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> SanityCostGE;
	float HeardHorrorSanityCost = 5;

	

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

	bool bResting = false;

#pragma region Hallucination Events

	bool bGunJam = false;
	FTimerHandle TimerGunJan;
	
#pragma endregion Hallucination Events
private:

	UPROPERTY()
	bool bIsFirstPerson = true;

	
};
