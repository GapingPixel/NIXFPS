// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "SMItemBase.h"
#include "GameFramework/Actor.h"
#include "SMEquippableBase.generated.h"

class USMGameplayAbility;
class ASMPlayerController;
class ISMFirstPersonInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentAmmoChanged, float, OldAmmo, float, NewAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquippableDelegate);

USTRUCT(BlueprintType)
struct FRecoilSettings
{
	GENERATED_BODY()

	//The time it takes to aim down sights, in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float ADSTime = 0.25;

	/** The amount of recoil to apply. We choose a random point from 0-1 on the curve and use it to drive recoil.
	This means designers get lots of control over the recoil pattern */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Recoil)
	class UCurveVector* RecoilCurve = nullptr;

	//The speed at which the recoil bumps up per second
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Recoil)
	float RecoilSpeed = 20;

	//The speed at which the recoil resets per second
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Recoil)
	float RecoilResetSpeed = 35;
};

// Cluster of animations that fit all ranges (First Person Arms and Equippable, Third Person Arms and Equippable)
USTRUCT(BlueprintType)
struct FEquippableAnimCluster
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First Person")
	UAnimMontage* ArmsMontage1P = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third Person")
	UAnimMontage* FullBodyMontage3P = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First Person")
	UAnimMontage* EquippableMontage1P = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third Person")
	UAnimMontage* EquippableMontage3P = nullptr;
};

UCLASS(Blueprintable, abstract)
class ASMEquippableBase : public ASMItemBase
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
public:	
	ASMEquippableBase();

	// ~AActor interface start
	virtual void SetOwner(AActor* NewOwner) override;
	virtual void OnRep_Owner() override;
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	// ~AActor interface end
	
	// ~ASMItemBase interface start
	virtual void OnPickUp(USMEquippableInventoryComponent* Inventory) override;
	// ~ASMItemBase interface end

	// Called when the player tries to equip this equippable.
	virtual bool CanEquip();

	/* This should only be called when the equippable is essentially being "interacted" with, usually only from abilities.
	 * Equippable Abilities have this called in C++ already, so don't worry about using this function in Gameplay Abilitiy Blueprints.
	 *
	 * True adds to the interaction count, false takes away from the interaction count.
	 */
	UFUNCTION(BlueprintCallable, Category = Equippable)
	virtual void AddOrRemoveFromInteractionCount(bool bAddToCount);

	// Is this equippable currently being interacted with?
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Equippable)
	bool IsBeingInteracted() const { return bIsBeingInteracted; };

	// @TODO: i don't really want to return a copy of the animations, please use references or something
	UFUNCTION(BlueprintNativeEvent, Category = Equippable)
	FEquippableAnimCluster DetermineEquipAnimation() const;
	virtual FEquippableAnimCluster DetermineEquipAnimation_Implementation() const;

	/* Animation
	***********************************************************************************/

public:

	// The main Animation Blueprint that will take control of the arms. This should contain all of the overlay animations etc.
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Animation")
	TObjectPtr<UAnimBlueprint> ArmsAnimMainBP;

	// This is the Animation Blueprint that extends the functionality of the ArmsAnimMainBP variable. For example, entering/exiting shotgun reload stance. 
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimLayerABP;
	
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimLayerABP;

	// Animation to play when picking up this equippable for the first time.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Animation")
	FEquippableAnimCluster FirstTimeEquipAnimations;
	
	// Animations that relate to equipping.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Animation")
	FEquippableAnimCluster EquipAnimations;

	// Animations that relate to unequipping
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Animation")
	FEquippableAnimCluster UnEquipAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Animation")
	FEquippableAnimCluster FireAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Animation")
	FEquippableAnimCluster ReloadAnimations;
	
	/* Components
	***********************************************************************************/

protected:
	
	// The mesh that will be used for first person view.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Equippable)
	TObjectPtr<USkeletalMeshComponent> EquippableMesh1P;
	
	// The mesh that will be used for third person view.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Equippable)
	TObjectPtr<USkeletalMeshComponent> EquippableMesh3P;
	
	/* Equippable Essentials (fire, alt fire, etc).
	***********************************************************************************/

public:

	// The current ammo of this equippable.
	UPROPERTY(Replicated, VisibleAnywhere, Category = Equippable)
	float CurrentAmmo;

	// The maximum amount that CurrentAmmo variable can be.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Equippable, meta=(ClampMin="0"))
	int32 MaxCurrentAmmo = 15;

	// Default starting ammo amount. -1 means use MaxCurrentAmmo.
	UPROPERTY(BlueprintReadOnly, Category = Equippable, meta=(ClampMin="-1", ExposeOnSpawn=true))
	int32 StartingAmmo = -1;

	

	// To be overidden by SMGunBase.
	virtual float GetRecoilHeatMultiplier() { return 1.0f; };

public:
	UFUNCTION(BlueprintCallable, Category = Equippable)
	void ApplyRecoilToController();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Equippable)
	FORCEINLINE float GetCurrentAmmo() const { return CurrentAmmo; };
	
	UFUNCTION(BlueprintCallable, Category = Equippable)
	void SetAmmo(float AmountToSet);

	// Adds ammo to Equippable. Clamp will clamp between 0 and MaxAmmo.
	UFUNCTION(BlueprintCallable, Category = Equippable)
	void AddAmmo(float AmountToAdd, bool bClamp);

	/* Internal Functions
	***********************************************************************************/

public:

	// Attaches meshes to their correct places. (3p mesh on 3p fullbody character, 1p mesh on 1p arms)
	void AttachToPawn(bool bFirstPerson) const;

	// There is a slight issue where upon clearing the arms AnimBP there is a single frame where the new equippable is shown,
	// bClearFirstPersonArmsAnimBP gives us the choice to not clear it so the next equippable can just "override it" instead.
	// Called when we want to hide the equippable. This does not mean that we have been dropped, but rather the player has unequipped this equippable.
	void DetachFromPawn(bool bFirstPerson, bool bInstant = false);

	void AddAbilitiesToOwner();
	void RemoveAbilitiesFromOwner();
	void CancelEquippableAbilities();

	// Called when explicitly spawned in from Spawn Actor From Class
	virtual void OnExplicitlySpawnedIn();
	
	ISMFirstPersonInterface* GetOwnerFirstPersonInterface() const { return OwnerFirstPersonInterface; }

private:

	void OnUnEquipAnimationFinished(bool bFirstPerson);
	void OnEquippableReadyToFire() const;
	
	UPROPERTY(BlueprintReadOnly, Category = "Equippable", meta=(AllowPrivateAccess=true))
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

protected:
	virtual bool ShouldPlayFirstTimeAnimation() const;
	
	/* Blueprint Exposed
	***********************************************************************************/

protected:

	// Gameplay Abilities to grant the owner upon equip. This will be things such as Weapon Fire.
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	TArray<TSubclassOf<USMGameplayAbility>> Abilities;

	// Recoil settings.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Equippable)
	FRecoilSettings RecoilSettings;

	// Time it takes for the equippable to be ready for use when the equippable starts being equipped.
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	float EquippableReadyTime = 0.0f;

	// Amount of bullets per shot. Increasing this would be useful for things like a shot gun.
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	int32 BulletsPerCartridge = 1;

	// Maximum amount of recoil.
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Recoil")
	float MaxRecoilHeat = 10.f;
	
	// Multiplies recoil by heat.
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Recoil")
	bool bMultiplyRecoilToHeat = true;

	// Multiplies recoil by heat vertically.
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Recoil", meta=(EditCondition="bMultiplyRecoilToHeat"))
	bool bMultiplyRecoilToHeatY = true;
	
	// Multiplies recoil by heat horizontally.
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Recoil", meta=(EditCondition="bMultiplyRecoilToHeat"))
	bool bMultiplyRecoilToHeatX = true;

	// Recoil multiplier when aiming. Lower values means less recoil. 0 means no recoil when aiming.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Recoil", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float AimRecoilMultiplier = 0.25f;
public:
	// FOV that the player has when ADS is active.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable", meta=(ClampMin=10.f, ClampMax=120.f))
	float EquippableAimFOV = 75.f;
protected:
	/* When the equippable is picked up for the first time, play the first time equip animation.
	 * Note: when another player then picks it up, it WON'T play the animation unless bAllowFirstTimeAnimationPerPlayer is enabled. */
	UPROPERTY(EditDefaultsOnly, Category = Equippable)
	bool bAllowFirstTimeEquipAnimations = false;

	UPROPERTY(EditDefaultsOnly, Category = Equippable, meta=(EditCondition="bAllowFirstTimeEquipAnimations"))
	bool bAllowFirstTimeEquipAnimationsPerPlayer = false;

	
public:

	// Gets the slot tag that this equippable occupies.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equippable")
	FGameplayTag GetSlotTag() const { return SlotTag; };

	// Called when the player tries to equip this equippable.
	UFUNCTION(BlueprintNativeEvent, Category = "Equippable", meta=(DisplayName="CanEquip"))
	bool BP_CanEquip();

	USkeletalMeshComponent* GetEquippableMesh1P() const { return EquippableMesh1P; };
	USkeletalMeshComponent* GetEquippableMesh3P() const { return EquippableMesh3P; };

private:
	
	// The slot tag that this equippable occupies.
	UPROPERTY(EditDefaultsOnly, Category = Equippable, meta=(AllowPrivateAccess=true))
	FGameplayTag SlotTag;

	/* Networking
	***********************************************************************************/
	
public:

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	/* Other (uncategorized)
	***********************************************************************************/

public:
	
	UPROPERTY(BlueprintAssignable, Category = Equippable)
	FOnCurrentAmmoChanged OnCurrentAmmoChanged;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = Equippable)
	FEquippableDelegate OnEquippableIsIdle;

	UPROPERTY(Replicated)
	bool bHasBeenPickedUpBefore = false;

protected:
	
	void OnOwnerUpdated(AActor* NewOwner);

private:

	UPROPERTY(BlueprintReadOnly, Category = Equippable, meta=(AllowPrivateAccess=true, ExposeOnSpawn))
	TWeakObjectPtr<AActor> ActorToReceivePostSpawn;
	
	bool bIsBeingInteracted = false;

	int32 EquippableInteractionCount;
	
	ISMFirstPersonInterface* OwnerFirstPersonInterface = nullptr;

	mutable FTimerHandle UnEquipTimerTimerHandle;
	mutable FTimerHandle EquipTimerTimerHandle;
};
