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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	FGameplayTag SlotTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory, meta=(ClampMin="0", ClampMax="15"))
	int32 MaxCount;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Inventory, meta=(AllowPrivateAccess=true))
	TArray<ASMEquippableBase*> SlotInventory;

	/* Helper Functions
	***********************************************************************************/
	
public:
	
	bool AddToSlotInventory(ASMEquippableBase* equippableToAdd)
	{
		if (equippableToAdd && !SlotInventory.Contains(equippableToAdd))
		{
			SlotInventory.Emplace(equippableToAdd);
			return true;
		}

		return false;
	}

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
	
	bool IsFull() const
	{
		{
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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USMEquippableInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	USMEquippableInventoryComponent();

	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = Inventory)
	void NextEquippable();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = Inventory)
	void PreviousEquippable();

	/* Blueprint Exposed
	***********************************************************************************/

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Inventory)
	static USMEquippableInventoryComponent* GetInventoryComponent(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Inventory)
	TArray<FInventorySlot>& GetEntireInventory() { return EquippableInventory; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Inventory)
	TArray<ASMEquippableBase*>& GetSlotInventory(FGameplayTag SlotTag, bool& bSuccess);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Inventory)
	ASMEquippableBase* GetCurrentEquippable() { return CurrentEquippable; };

	UFUNCTION(BlueprintCallable, BlueprintPure,  meta = (BlueprintThreadSafe, ThreadSafe),Category = Inventory)
	bool HasEquippableEquipped() const { return CurrentEquippable != nullptr; }
	
	UFUNCTION(BlueprintCallable, Category = Inventory, meta=(Keywords="SetCurrentEquippable SetEquippable SetWeapon SetCurrentWeapon"))
	void SetDesiredEquippable(ASMEquippableBase* desiredEquippable);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	bool GiveExistingEquippable(ASMEquippableBase* equippableToGive);
	
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

	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	bool bDisallowSlotlessEquippables = true;


	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<FInventorySlot> StartingSlots;


	UPROPERTY(EditDefaultsOnly, Category = Item)
	float RePickUpTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = Item)
	float DropVelocity = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = Item)
	float UpVelocity = 200.f;

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
	
	void SetCurrentEquippable(ASMEquippableBase* equippableToSet, bool bFromReplication);
	
	bool AddEquippableToInventory(ASMEquippableBase* equippableToAdd);
	
	void CheckPerformEquippableDrop(bool bForceDrop = false);
	
	void SetEquippableChangeStatus(EEquippableChangeStatus NewStatus);

public:
	
	void ForceDropCurrentEquippableBeforeDestroy();

	/* Helper Functions
	***********************************************************************************/

protected:
	
	bool AlreadyHasEquippable(TSubclassOf<ASMEquippableBase> equippable);
	
	ASMEquippableBase* FindEquippableByClass(TSubclassOf<ASMEquippableBase> equippableToFind);
	
	bool FindEquippableInInventory(ASMEquippableBase* equippableToFind);
	
	bool CheckEquippableSlot(FGameplayTag slotGameplayTag);
	
	bool CanAddEquippableToInventory(const ASMEquippableBase* equippableToCheck);
	
	bool IsSlotFull(FGameplayTag slotToCheck);
	
	FInventorySlot* FindInventorySlotByTag(FGameplayTag slotTag);

	void GetAllEquippablesInInventory(TArray<ASMEquippableBase*>& OutEquippables);
	
	/* Equippable Variables
	***********************************************************************************/
	
public:

	
	// Equippables that this Inventory Component holds.
	UPROPERTY(VisibleInstanceOnly, Replicated)
	TArray<FInventorySlot> EquippableInventory;
	
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing=OnRep_CurrentEquippable)
	ASMEquippableBase* CurrentEquippable;

	bool bIsUnEquippingEquippable = false;

	/* Networking
	***********************************************************************************/

protected:
	
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



public:
	
	bool GetCachedHasAuthority() const { return bCachedHasAuthority; }
	
private:
	
	FInventorySlot DummySlot = FInventorySlot();
	
	
	FTimerHandle UnEquipTimerHandle;


	UPROPERTY()
	TWeakObjectPtr<ASMEquippableBase> DesiredEquippable;

	// Cached equippable that the player wants to drop.
	UPROPERTY()
	TWeakObjectPtr<ASMEquippableBase> DesiredEquippableToDrop;
	
	
	bool bCachedHasAuthority;

	UPROPERTY(ReplicatedUsing=OnRep_EquippableChangeStatus)
	EEquippableChangeStatus EquippableChangeStatus = EEquippableChangeStatus::None;

	FORCEINLINE bool IsListenServerOrStandalone() const { return (CachedOwnerNetMode == NM_ListenServer) || CachedOwnerNetMode == NM_Standalone; }
	
	ENetMode CachedOwnerNetMode;
	
	FORCEINLINE bool OwnerIsLocalController() const { return GetOwner() && GetOwner()->GetInstigatorController() && GetOwner()->GetInstigatorController()->IsLocalController(); }


	FORCEINLINE bool GetIsListenServerOrStandaloneLocalController() const { return (IsListenServerOrStandalone() == true && OwnerIsLocalController() == true); };
};