// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Items/SMEquippableBase.h"
#include "SMEquippableInventoryComponent.generated.h"

struct FGameplayTag;
class ASMEquippableBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentEquippableChanged, ASMEquippableBase*, OldEquippable);

UENUM()
enum class EEquippableChangeStatus : uint8
{
	None,
	Equipping,
	UnEquipping
};

// An inventory of equippables of a specific slot.
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

public:
	
	FInventorySlot() : SlotTag(FGameplayTag::EmptyTag), MaxCount(0)
	{}
	FInventorySlot(FGameplayTag InSlotTag, int32 InMaxCount) : SlotTag(InSlotTag), MaxCount(InMaxCount)
	{}
	
	/* Main Variables
	***********************************************************************************/

public:
	
	// The slot tag that this slot inventory represents
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	FGameplayTag SlotTag;

	// The max amount of elements that can be in the slot inventory at once. Set to 0 to disable this slot inventory.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory, meta=(ClampMin="0", ClampMax="15"))
	int32 MaxCount;

	// The slot inventory of equippables that have the slot tag above.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Inventory, meta=(AllowPrivateAccess=true))
	TArray<ASMEquippableBase*> SlotInventory;

	/* Helper Functions
	***********************************************************************************/
	
public:

	// Adds an equippable to the slot inventory.
	bool AddToSlotInventory(ASMEquippableBase* equippableToAdd)
	{
		if (equippableToAdd && !SlotInventory.Contains(equippableToAdd))
		{
			SlotInventory.Emplace(equippableToAdd);
			return true;
		}

		return false;
	}

	// Removes an equippable from the slot inventory.
	bool RemoveFromSlotInventory(ASMEquippableBase* equippableToRemove)
	{
		{
			if (equippableToRemove && SlotInventory.Contains(equippableToRemove))
			{
				SlotInventory.RemoveSingle(equippableToRemove);
				return true;
			}

			return false;
		}
	}

	// Returns if the slot inventory is full.
	bool IsFull() const
	{
		{
			// if MaxCount isn't 0 then calculate how many elements we have compared to how many we're allowed to have, otherwise return true.
			return MaxCount != 0 ? SlotInventory.Num() >= MaxCount : true;
		}
	}

	/* Operator Overloads
	***********************************************************************************/
	
	FORCEINLINE bool operator==(const FInventorySlot& Other) const
	{
		return SlotTag == Other.SlotTag;
	}

	FORCEINLINE bool operator==(const FGameplayTag& Other) const
	{
		return SlotTag == Other;
	}
};

/**
 * Design Goals for this component:
 * 
 * We want to be able to attach this component to any character and it just work.
 * 
 * I want the possibility of having this potentially being on other kinds of pawns and it just work.
 * This is mostly for future proofing cases and ensuring that we have more flexibility without having to rewrite
 * the inventory component.
 *
 * - Have this component have the ability to attach to any pawn and it just work.
 * - Fully replicated all done automatically with hopefully no setup required.
 * - Be able to have a "temporary equippable" for one off interactions etc.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USMEquippableInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	// Sets default values for this component's properties
	USMEquippableInventoryComponent();

	// ~UActorComponent interface begin
	virtual void OnRegister() override;
	// ~UActorComponent interface end
	
	/* Equippable Management (next equippable, etc)
	***********************************************************************************/

	// Attempts to equip the next equippable in-line.
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = Inventory)
	void NextEquippable();

	// Attempts to equip the next equippable in-line.
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = Inventory)
	void PreviousEquippable();

	/* Blueprint Exposed
	***********************************************************************************/

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Inventory)
	static USMEquippableInventoryComponent* GetInventoryComponent(AActor* Actor);
	
	// Gets the entire equippable inventory with all slots.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Inventory)
	TArray<FInventorySlot>& GetEntireInventory() { return EquippableInventory; };
	
	// Gets a copy of a slot inventory.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Inventory)
	TArray<ASMEquippableBase*>& GetSlotInventory(FGameplayTag SlotTag, bool& bSuccess);

	// Gets currently held equippable.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Inventory)
	ASMEquippableBase* GetCurrentEquippable() { return CurrentEquippable; };

	// Returns if the owning pawn currently has a CurrentEquipapble.
	UFUNCTION(BlueprintCallable, BlueprintPure,  meta = (BlueprintThreadSafe, ThreadSafe),Category = Inventory)
	bool HasEquippableEquipped() const { return CurrentEquippable != nullptr; }
	
	// Attempts to equip an equippable. Equippable must be in inventory to work.
	UFUNCTION(BlueprintCallable, Category = Inventory, meta=(Keywords="SetCurrentEquippable SetEquippable SetWeapon SetCurrentWeapon"))
	void SetDesiredEquippable(ASMEquippableBase* desiredEquippable);

	// Gives an equippable that already exists in the world.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	bool GiveExistingEquippable(ASMEquippableBase* equippableToGive);

	// @TODO: Make it so this can be called from the server. At the moment only the client can call this and it work properly.
	// Drops the specified equippable out of the equippable inventory
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void DropEquippable(ASMEquippableBase* EquippableToDrop, bool bFromReplication, bool bDontFindNextEquippable = false, bool bInstantIfCurrent = false);

	// Drops current equippable.
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void DropCurrentEquippable(bool bInstant);

	// Called when the CurrentEquippable gets changed.
	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnCurrentEquippableChanged OnCurrentEquippableChanged;

	UFUNCTION(BlueprintCallable, BlueprintPure,  meta = (BlueprintThreadSafe, ThreadSafe), Category = Inventory)
	bool IsUnEquippingCurrentEquippable() const;

	UFUNCTION(BlueprintCallable, Category = Inventory, meta=(HidePin="bOwnerBeingDestroyed"))
	void DropAllEquippables(bool bOwnerBeingDestroyed = false);
	
protected:
	// If enabled, equippables that do not have an assigned slot cannot be picked up.
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	bool bDisallowSlotlessEquippables = true;

	// The slots in this array will be the slots that this character can use when playing.
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<FInventorySlot> StartingSlots;

	// The amount of time until a dropped item can be picked up again.
	UPROPERTY(EditDefaultsOnly, Category = Item)
	float RePickUpTime = 2.0f;

	// The amount of velocity to apply to an item when being dropped from the owning actor.
	UPROPERTY(EditDefaultsOnly, Category = Item)
	float DropVelocity = 100.f;

	// The amount of up/down velocity to apply to an item when being dropped from the owning actor.
	UPROPERTY(EditDefaultsOnly, Category = Item)
	float UpVelocity = 200.f;

	// Where to drop a dropped item relative to the inventory owner's view location.
	UPROPERTY(EditDefaultsOnly, Category = Item)
	FVector RelativeDropLocation = FVector(0.f, 0.f, 0.f);

	/* Delegates
	***********************************************************************************/

protected:

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnEquippableAddedToInventorySignature, ASMEquippableBase* Equippable, FGameplayTag SlotTag, FInventorySlot* Slot)
	FOnEquippableAddedToInventorySignature OnEquippableAddedToInventory;

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnEquippableRemovedFromInventorySignature, ASMEquippableBase* Equippable, FGameplayTag SlotTag, FInventorySlot* Slot)
	FOnEquippableRemovedFromInventorySignature OnEquippableRemovedFromInventory;
	
	/* Internal Functions
	***********************************************************************************/

private:

	void AttemptEquip(ASMEquippableBase* desiredEquippable, bool bFromReplication);
	
	void BeginUnEquippingCurrentEquippable();

	void ApplyEquippableChangingGameplayTag();

	float Play1PCurrentEquippableUnEquipAnimations() const;

	void OnUnEquipFinish();
	
	// Sets current equippable and handles equipping.	
	void SetCurrentEquippable(ASMEquippableBase* equippableToSet, bool bFromReplication);
	
	// Adds equippable to inventory and equips it if need be.
	bool AddEquippableToInventory(ASMEquippableBase* equippableToAdd);

	// Checks if we need to drop an equippable. Should be called by server.
	void CheckPerformEquippableDrop(bool bForceDrop = false);

	// Changes equippable change status.
	void SetEquippableChangeStatus(EEquippableChangeStatus NewStatus);

public:
	
	// Force drops current equippable just before being destroyed.
	void ForceDropCurrentEquippableBeforeDestroy();

	/* Helper Functions
	***********************************************************************************/

protected:
	
	// Checks to see if this component already has an equippable in it's inventory.
	bool AlreadyHasEquippable(TSubclassOf<ASMEquippableBase> equippable);

	// Finds a specified type of equippable in the inventory.
	ASMEquippableBase* FindEquippableByClass(TSubclassOf<ASMEquippableBase> equippableToFind);

	// Returns whether we have the specified equippable in the inventory array.
	bool FindEquippableInInventory(ASMEquippableBase* equippableToFind);

	// Returns if the tag is valid and the inventory has free space.
	bool CheckEquippableSlot(FGameplayTag slotGameplayTag);

	// Checks to see if an equippable can be equipped in an inventory.
	bool CanAddEquippableToInventory(const ASMEquippableBase* equippableToCheck);

	// Checks to see whether any more equippables of a certain slot can be added to the inventory.
	bool IsSlotFull(FGameplayTag slotToCheck);

	// Returns if we have a slot inventory in the EquippableInventory array.
	FInventorySlot* FindInventorySlotByTag(FGameplayTag slotTag);

	void GetAllEquippablesInInventory(TArray<ASMEquippableBase*>& OutEquippables);
	
	/* Equippable Variables
	***********************************************************************************/
	
public:

	// Instead of having ASMEquippableBase* as the array type, why do we not have a struct that contains:
		// FGameplayTag SlotType
		// TArray<ASMEquippableBase*> SlotInventory
	// we can then have operator overloads based on SlotType
	
	// Equippables that this Inventory Component holds.
	UPROPERTY(VisibleInstanceOnly, Replicated)
	TArray<FInventorySlot> EquippableInventory;

	/* CurrentEquippable is only replicated down automatically to simulated proxies. We manually replicate CurrentEquippable
	 * between owning client and the server. This is to allow simulated proxies to get only important updates from this
	 * variable, and keep control of exactly what happens to it on the owning client. OnRep is used to trigger animations
	 * etc for simulated proxies. */
	// DESCRIPTION: The current equippable that the owning pawn is holding. // @TODO: make this only replicate to simulated proxies in GetLifetimeReplicatedProperties
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing=OnRep_CurrentEquippable)
	ASMEquippableBase* CurrentEquippable;

	bool bIsUnEquippingEquippable = false;

	UFUNCTION(Server, Reliable)
	void UnEquipCurrent();
	void UnEquipCurrentClient();

	void UnEquipAndDestroy();
	/* Networking
	***********************************************************************************/
	
	
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastVisuallyUnEquip(ASMEquippableBase* equippableToUnEquip);
	
	UFUNCTION(Server, Reliable)
	void ServerAttemptEquip(ASMEquippableBase* desiredEquippable);

	UFUNCTION(Client, Reliable)
	void ClientAttemptEquip(ASMEquippableBase* desiredEquippable);
	
	UFUNCTION(Client, Reliable)
	void ClientSetCurrentEquippable(ASMEquippableBase* equippableToSet);

	UFUNCTION(Server, Reliable)
	void ServerSetCurrentEquippable(ASMEquippableBase* equippableToSet);

	UFUNCTION(Server, Reliable)
	void ServerDropEquippable(ASMEquippableBase* equippableToDrop, bool bInstant, bool bDontFindNextEquippable);

	UFUNCTION(Client, Reliable)
	void ClientDropEquippable(ASMEquippableBase* equippableToDrop, bool bInstant, bool bDontFindNextEquippable);

	UFUNCTION()
	void OnRep_CurrentEquippable(ASMEquippableBase* OldEquippable);
	
	UFUNCTION()
	void OnRep_EquippableChangeStatus(EEquippableChangeStatus OldEquippableChangeStatus);

	/* Other (uncategorized, at least for now)
	***********************************************************************************/

public:
	
	bool GetCachedHasAuthority() const { return bCachedHasAuthority; }
	
private:

	// This is used for functions that need to return a reference to an FInventorySlot.
	FInventorySlot DummySlot = FInventorySlot();
	
	// Should be length of unequip animation.
	FTimerHandle UnEquipTimerHandle;

	// This changes rapidly, and through the function of SetDesiredEquippable, it can trigger equippable changing behaviour. Do not touch this directly.
	UPROPERTY()
	TWeakObjectPtr<ASMEquippableBase> DesiredEquippable;

	// Cached equippable that the player wants to drop.
	UPROPERTY()
	TWeakObjectPtr<ASMEquippableBase> DesiredEquippableToDrop;

		// NOTE: This comment was made before DesiredEquippable existed:
	// This would be a variable that can change rapidly on clientside (spamming scroll wheel to change weapons)
	// and so as the previous weapon unequips, when it finishes whatever the DesiredEquippable is at the time
	// would be the one that it would start switching to. 
	
	// Cached variable of the owning pawns network role so we don't have to keep calling the same HasAuthority function or whatever.
	bool bCachedHasAuthority;

	UPROPERTY(ReplicatedUsing=OnRep_EquippableChangeStatus)
	EEquippableChangeStatus EquippableChangeStatus = EEquippableChangeStatus::None;

	FORCEINLINE bool IsListenServerOrStandalone() const { return (CachedOwnerNetMode == NM_ListenServer) || CachedOwnerNetMode == NM_Standalone; }
	// Net Mode of the owner. Cached to save calling GetNetMode and doing comparison check each time we want to know the Net Mode.
	ENetMode CachedOwnerNetMode;

	// Returns if the components owner is the local controller.
	FORCEINLINE bool OwnerIsLocalController() const { return GetOwner() && GetOwner()->GetInstigatorController() && GetOwner()->GetInstigatorController()->IsLocalController(); }

	// Gets if the owning pawn
	FORCEINLINE bool GetIsListenServerOrStandaloneLocalController() const { return (IsListenServerOrStandalone() == true && OwnerIsLocalController() == true); };
};