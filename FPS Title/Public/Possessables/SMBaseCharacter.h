// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "Interfaces/AbilityBindingInterface.h"
#include "Interfaces/SMFirstPersonInterface.h"
#include "SMBaseCharacter.generated.h"

class ASMPlayerState;
class USMPlayerAttributesComponent;
class USMCharacterMovementComponent;
class USMCharacterAttributeSet;
class USMEquippableInventoryComponent;
class USMPawnComponent;
class USMHealthComponent;
class USMAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FAbilityClassWithLevel
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> Ability = nullptr;

	UPROPERTY(EditDefaultsOnly)
	int32 Level = 0;
	
};

UCLASS()
class ASMBaseCharacter : public ACharacter, public ISMFirstPersonInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASMBaseCharacter(const class FObjectInitializer& ObjectInitializer);

	// ~Start of AActor Interface 
	virtual void PostInitializeComponents() override;
	virtual void OnRep_PlayerState() override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~End of AActor Interface

	// ~Start of APawn Interface 
	virtual void PossessedBy(AController* NewController) override;
	// ~End of APawn Interface'

	// Used by custom movement component. Do not use elsewhere.
	void SetSprintFromMovementComponent(bool bNewSprint) { bIsSprinting = bNewSprint; };

	UFUNCTION()
	virtual void PerformDeath(AActor* OwningActor);

	UFUNCTION()
	virtual void OnCurrentEquippableChanged(ASMEquippableBase* OldEquippable);

	// Gets the character's MyCustomMovementComponent
	UFUNCTION(BlueprintCallable, Category = "Movement")
	USMCharacterMovementComponent* GetMyMovementComponent() const;

	void ApplyPersonalSpacePush();

	FTimerHandle PushTimerHandle;
	
#pragma region BlueprintExposed
	
	/* Blueprint Exposed
	***********************************************************************************/

protected:
	
	// The amount of push force on tick to apply when a character collides with this one.
	UPROPERTY(EditDefaultsOnly, Category = "Character|Personal Space")
	float PushForce = 10000.0f;

	// How much more to apply force based on the distance between the characters. Calculation: (capsuleDiameter - distance) * DistanceMultiplier
	UPROPERTY(EditDefaultsOnly, Category = "Character|Personal Space")
	float DistanceMultiplier = 5.0f;

	// How often we should apply the force.
	UPROPERTY(EditDefaultsOnly, Category = "Character|Personal Space")
	float ForceApplicationTime = 0.75f;

	// When the character doesn't have a weapon, this will be the anim layer that links to the arms mesh anim instance.
	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TSubclassOf<UAnimInstance> UnarmedAnimationBlueprint;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	bool GetIsSprinting() const { return bIsSprinting; }

#pragma endregion BlueprintExposed
	
	
#pragma region BoneNames

private:

	// The bone for equippables to attach to in third person.
	UPROPERTY(EditDefaultsOnly, Category = BoneNames)
	FName ThirdPersonEquippableBone = FName("hand_r");

	// The bone for equippables to attach to in first person.
	UPROPERTY(EditDefaultsOnly, Category = BoneNames)
	FName FirstPersonEquippableBone = FName("hand_r");

#pragma endregion BoneNames
	
#pragma region GAS

	/* GAS
	***********************************************************************************/
	
public:
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	static void AddDefaultAbilities(USMAbilitySystemComponent* ASC, TArray<FAbilityClassWithLevel>& AbilitiesToAdd);
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> StartingAttributes;
protected:

	// Default Abilities (for testing) int32 for being level of ability.
	UPROPERTY(EditDefaultsOnly, Category = GAS)
	TArray<FAbilityClassWithLevel> DefaultAbilities;

	// Called when the Ability System Component is all setup on this character.
	void OnAbilitySystemComponentInitialized();

	void OnAbilitySystemComponentUnInitialized() const;

	// Called when the Ability System Component is initialized and ready to be used.
	UFUNCTION(BlueprintImplementableEvent, Category = GAS, meta=(DisplayName="On Ability System Component Initialized"))
	void BP_OnAbilitySystemComponentInitialized();
	
	void SetupGas();
	void SetupAttributes() const;

	/** The GameplayEffect that will be applied to the ASC that will define the starting attributes of this character. */
	
	
	UPROPERTY()
	USMCharacterAttributeSet* CharacterAttributeSet;

	//@TODO: Add survivorzombie attribute set here?

	/*
	 *		Classes:
	 * To help with how swapping to other types of characters (turning into a zombie for example), we should
	 * have a conceptual system put in place called something like "classes" where you can change "class" and it will
	 * do a bunch of checks and run code that will sort out for example: if turning into zombie from survivor, then
	 * drop all weapons before doing so.
	 *
	 *		Health Component and Teams:
	 * A "health component" should basically be the manager for these classes/teams so that AI and team damage can
	 * be handled properly. The health component will communicate with GAS to help with these instances so that
	 * we don't have raw float values on a component that we have to manage ourselves, we may as well just get
	 * GAS to do that anyway.
	 *
	 *		Equippables deriving from Items
	 * Should equippables derive from "items"? If not, we will have to spawn in items and attach the equippable to
	 * that item so that information carries on over. If it were derived from item, we won't have to spawn in/delete
	 * item actors at runtime each time a survivor drops/picks up an equippapble.
	 */
	
	#pragma endregion GAS

#pragma region Components
	
	/* Components
	***********************************************************************************/

public:
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Components)
	TObjectPtr<USMPawnComponent> PawnComponent;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Replicated, Category = Components)
	TObjectPtr<USMPlayerAttributesComponent> HealthComp;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Components)
	TObjectPtr<USMEquippableInventoryComponent> InventoryComponent;

#pragma endregion Components

#pragma region Getters

	/* Getters
	***********************************************************************************/
	
public:
	
	/** Gets standard movement speed of this character. */
	UFUNCTION(BlueprintGetter, Category = "Character|Movement")
	float GetMovementSpeed() const;
	
	virtual USkeletalMeshComponent* GetMeshOfType(EMeshType MeshType) override;

	// Get a specific mesh from this character.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Character, meta=(DisplayName="Get Mesh Of Type"))
	virtual USkeletalMeshComponent* BP_GetMeshOfType(EMeshType MeshType) { return GetMeshOfType(MeshType); };

	virtual FName GetBoneAttachName(EMeshType MeshType) override;

	virtual USMEquippableInventoryComponent* GetInventoryComponent() override;

	FORCEINLINE virtual USMAbilitySystemComponent* GetSMAbilitySystemComponent() const override { return static_cast<USMAbilitySystemComponent*>(GetAbilitySystemComponent()); };
	
#pragma endregion Getters

	/* Other (uncategorized, at least for now)
	***********************************************************************************/
	
protected:
	// Cached custom PlayerState so we don't have to keep casting.
	UPROPERTY()
	TObjectPtr<ASMPlayerState> SMPlayerStateCache = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<ASMBaseCharacter>> OverlappingCharacters;

private:
	UPROPERTY(Replicated)
	bool bIsSprinting = false;
};
