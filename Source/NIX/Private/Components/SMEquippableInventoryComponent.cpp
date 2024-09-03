// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SMEquippableInventoryComponent.h"

#include "AbilitySystemGlobals.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "Interfaces/SMFirstPersonInterface.h"
#include "Items/SMEquippableBase.h"
#include "Net/UnrealNetwork.h"
#include "NIX/NIX.h"

DECLARE_CYCLE_STAT(TEXT("GiveNewEquippable"), STAT_GiveNewEquippable, STATGROUP_NIX);

static TAutoConsoleVariable<int32> CVarPrintInventory(
	TEXT("sm.PrintInventory"),
	0,
	TEXT("Prints the local player's inventory to screen.\n")
	TEXT("0 = Off.\n")
	TEXT("1 = Print inventory to main screen.\n"),
	ECVF_Default | ECVF_RenderThreadSafe);

// Sets default values for this component's properties
USMEquippableInventoryComponent::USMEquippableInventoryComponent()
{
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USMEquippableInventoryComponent::OnRegister()
{
	Super::OnRegister();

	bCachedHasAuthority = (GetOwnerRole() == ROLE_Authority);
	if (GetOwnerRole() == ENetRole::ROLE_None)
	{
		UE_LOG(LogNIX, Fatal, TEXT("bCachedHasAuthority in InitializeInventoryComponent in USMEquippableInventoryComponent is ROLE_None"))
	}

	CachedOwnerNetMode = GetNetMode();
	
	EquippableInventory = StartingSlots;
}

void USMEquippableInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	int32 MyVar = CVarPrintInventory.GetValueOnGameThread();
	if (MyVar > 0 && GEngine)
	{
		int32 equippableNum = 0;
		for (FInventorySlot& slot : EquippableInventory)
		{
			for (ASMEquippableBase* equippable : slot.SlotInventory)
			{
				if (equippable)
				{
					GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Emerald, FString::FromInt(equippableNum) + FString(": ") + AActor::GetDebugName(equippable));
					equippableNum++;
				}
			}
		}


		FString ConnectionType;

		switch(GetOwner()->GetNetMode())
		{
		case NM_Client:
			ConnectionType = FString("Client");
			break;

		case NM_Standalone:
			ConnectionType = FString("Standalone");
			break;
			
		case NM_ListenServer:
			ConnectionType = FString("Listen Server");
			break;
			
		case NM_DedicatedServer:
			ConnectionType = FString("Dedicated Server");
			break;
		
		default:
			ConnectionType = FString("None");
			break;
		}
		
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Emerald, FString("Inventory:"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, FString("CurrentEquippable: ") + AActor::GetDebugName(CurrentEquippable));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString("Owner Name: ") + AActor::GetDebugName(GetOwner()));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, FString("Connection type: ") + ConnectionType);
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString("==============================================="));
	}
}

/* Equippable Management (next equippable, etc)
***********************************************************************************/

void USMEquippableInventoryComponent::NextEquippable()
{
	DesiredEquippable.Reset();
	
	int32 nextSlotIdxToSearch = 0;
	
	ASMEquippableBase* DesiredOrCurrentEquippable = DesiredEquippable.IsValid() ? DesiredEquippable.Get() : CurrentEquippable;
	
	if (DesiredOrCurrentEquippable)
	{
		FInventorySlot* equippableSlot = nullptr;
		
		equippableSlot = FindInventorySlotByTag(DesiredOrCurrentEquippable->GetSlotTag());
		check(equippableSlot)
		
		const int32 CurrentEquippableIdx = equippableSlot->SlotInventory.Find(DesiredOrCurrentEquippable);
		const int32 NextElement = CurrentEquippableIdx + 1;
		if (equippableSlot->SlotInventory.IsValidIndex(NextElement))
		{
			// Equip the next equippable in CurrentEquippables current slot.
			SetDesiredEquippable(equippableSlot->SlotInventory[NextElement]);
			return;
		}
		
		nextSlotIdxToSearch = (EquippableInventory.Find(*equippableSlot) + 1) % EquippableInventory.Num();
		check(nextSlotIdxToSearch != INDEX_NONE)
	}
	
	int32 slotsSearched = 0;
	const int32 slotCount = EquippableInventory.Num();
	for (int32 Idx = nextSlotIdxToSearch; slotsSearched < slotCount; slotsSearched++)
	{
		if (EquippableInventory.IsValidIndex(Idx))
		{
			FInventorySlot& slotToSearch = EquippableInventory[Idx];
		
			for (ASMEquippableBase* Equippable : slotToSearch.SlotInventory)
			{
				check(Equippable)
				
				const bool bNotMarkedForDrop = DesiredEquippable.IsValid() == true ? Equippable != DesiredEquippableToDrop.Get() : true;
				if (Equippable->CanEquip() && bNotMarkedForDrop && Equippable != CurrentEquippable)
				{
					SetDesiredEquippable(Equippable);
					return;
				}
			}
		}

		Idx = (Idx + 1) % EquippableInventory.Num();
	}
	
	if (DesiredEquippable.IsValid())
	{
		SetDesiredEquippable(nullptr);
	}
}

void USMEquippableInventoryComponent::PreviousEquippable()
{
	DesiredEquippable.Reset();
	
	int32 NextSlotIdxToSearch = 0;

	
	ASMEquippableBase* DesiredOrCurrentEquippable = DesiredEquippable.IsValid() ? DesiredEquippable.Get() : CurrentEquippable;
	
	if (DesiredOrCurrentEquippable)
	{
		FInventorySlot* EquippableSlot = nullptr;
		
		EquippableSlot = FindInventorySlotByTag(DesiredOrCurrentEquippable->GetSlotTag());
		check(EquippableSlot)
		
		const int32 currentEquippableIdx = EquippableSlot->SlotInventory.Find(DesiredOrCurrentEquippable);
		const int32 nextElement = currentEquippableIdx + -1;
		if (EquippableSlot->SlotInventory.IsValidIndex(nextElement))
		{
			SetDesiredEquippable(EquippableSlot->SlotInventory[nextElement]);
			return;
		}

		NextSlotIdxToSearch =  FMath::Abs(EquippableInventory.Find(*EquippableSlot) - 1 + EquippableInventory.Num()) % EquippableInventory.Num();
		check(NextSlotIdxToSearch != INDEX_NONE)
	}

	int32 SlotsSearched = 0;
	const int32 SlotCount = EquippableInventory.Num();
	for (int32 SlotIdx = NextSlotIdxToSearch; SlotsSearched < SlotCount; SlotsSearched++)
	{
		if (EquippableInventory.IsValidIndex(SlotIdx))
		{
			FInventorySlot& SlotToSearch = EquippableInventory[SlotIdx];

			// Loop through the slots inventory backwards
			for (int32 SlotEquippableIdx = SlotToSearch.SlotInventory.Num() - 1; SlotEquippableIdx >= 0; SlotEquippableIdx--)
			{
				ASMEquippableBase* Equippable = SlotToSearch.SlotInventory[SlotEquippableIdx];
				check(Equippable)

				const bool bNotMarkedForDrop = DesiredEquippable.IsValid() == true ? Equippable != DesiredEquippableToDrop.Get() : true;
				if (Equippable->CanEquip() && bNotMarkedForDrop && Equippable != CurrentEquippable)
				{
					SetDesiredEquippable(Equippable);
					return;
				}
			}
		}
		SlotIdx = (SlotIdx - 1) % EquippableInventory.Num();
	}
	
	if (DesiredEquippable.IsValid())
	{
		SetDesiredEquippable(nullptr);
	}
}

USMEquippableInventoryComponent* USMEquippableInventoryComponent::GetInventoryComponent(AActor* Actor)
{
	if (Actor)
	{
		USMEquippableInventoryComponent* Inventory = Actor->FindComponentByClass<USMEquippableInventoryComponent>();
		if (Inventory)
		{
			return Inventory;
		}
	}
	return nullptr;
}

/* Blueprint Exposed Functions
***********************************************************************************/

TArray<ASMEquippableBase*>& USMEquippableInventoryComponent::GetSlotInventory(FGameplayTag SlotTag, bool& bSuccess)
{
	bSuccess = false;
	
	if (SlotTag.IsValid())
	{
		if (FInventorySlot* InvSlot = FindInventorySlotByTag(SlotTag))
		{
			bSuccess = true;
			return InvSlot->SlotInventory;
		}
	}

	return DummySlot.SlotInventory;
}

void USMEquippableInventoryComponent::SetDesiredEquippable(ASMEquippableBase* desiredEquippable)
{
	const ENetRole OwnerRole = GetOwnerRole(); 
	if (OwnerRole == ROLE_AutonomousProxy || GetIsListenServerOrStandaloneLocalController())
	{
		const bool bCurrentEquippableIsNotDesiredEquippable = CurrentEquippable != nullptr ? CurrentEquippable != desiredEquippable : true;
		if (bCurrentEquippableIsNotDesiredEquippable) 
		{
			if (!GetWorld()->GetTimerManager().IsTimerActive(UnEquipTimerHandle))
			{
				AttemptEquip(desiredEquippable, false);
			}
			
			if (CurrentEquippable != desiredEquippable)
			{
				DesiredEquippable = desiredEquippable;
			}
		}
	}
}

void USMEquippableInventoryComponent::AttemptEquip(ASMEquippableBase* desiredEquippable, bool bFromReplication)
{
	SM_LOG(Log, TEXT("%s called with equippable: %s, Authority: %i"), ANSI_TO_TCHAR(__FUNCTION__), *AActor::GetDebugName(desiredEquippable), bCachedHasAuthority)
	
	// no point going past this if statement if we have nothing to unequip in the first place.
	if (!CurrentEquippable)
	{
		SetCurrentEquippable(desiredEquippable, bFromReplication);
		return;
	}
	
	if (!bFromReplication)
	{
		if (bCachedHasAuthority && !GetIsListenServerOrStandaloneLocalController())
		{
			ClientAttemptEquip(desiredEquippable);
		}
		else
		{
			ServerAttemptEquip(desiredEquippable);
		}
	}
	
	BeginUnEquippingCurrentEquippable();
}

bool USMEquippableInventoryComponent::GiveExistingEquippable(ASMEquippableBase* equippableToGive)
{
	if (!bCachedHasAuthority)
	{
		return false;
	}
	
	if (!equippableToGive)
	{
		return false;
	}

	if (!CheckEquippableSlot(equippableToGive->GetSlotTag()))
	{
		return false;
	}

	const bool bSuccess = AddEquippableToInventory(equippableToGive);
	return bSuccess;
}

void USMEquippableInventoryComponent::DropEquippable(ASMEquippableBase* EquippableToDrop, bool bFromReplication, bool bDontFindNextEquippable, bool bInstantIfCurrent)
{
	const bool bAlreadyTryingToDrop = DesiredEquippableToDrop.IsValid() == true ? EquippableToDrop == DesiredEquippableToDrop : false; 
	if (EquippableToDrop && bAlreadyTryingToDrop == false)
	{
		const ASMEquippableBase* oldEquippable = CurrentEquippable;
		if (EquippableToDrop == CurrentEquippable)
		{
			const bool bIsListenServerOrStandaloneController = GetIsListenServerOrStandaloneLocalController();
			if (bCachedHasAuthority == false || bIsListenServerOrStandaloneController)
			{
				if (bDontFindNextEquippable == false)
				{
					NextEquippable();
				}

				if (bInstantIfCurrent)
				{
					// Instantly unequip
					GetWorld()->GetTimerManager().ClearTimer(UnEquipTimerHandle);
					OnUnEquipFinish();
				}
				
				if (DesiredEquippable.IsValid() == false)
				{
					SetDesiredEquippable(nullptr);
				}
			}
		}
		
		if (!bCachedHasAuthority)
		{
			// Notify the server that we want to drop equippable, as we can't trust clients to actually perform the drop.
			ServerDropEquippable(EquippableToDrop, bInstantIfCurrent, bDontFindNextEquippable);
		}
		else // if authority
		{
			if (!bFromReplication)
			{
				ClientDropEquippable(EquippableToDrop, bInstantIfCurrent, bDontFindNextEquippable);
			}
			
			DesiredEquippableToDrop = EquippableToDrop;

			// we want to perform the drop immediately if it's not our current equippable.
			if (EquippableToDrop != oldEquippable)
			{
				CheckPerformEquippableDrop(bInstantIfCurrent);
			}
		}
	}
}

void USMEquippableInventoryComponent::DropCurrentEquippable(bool bInstant)
{
	DropEquippable(CurrentEquippable, false, false, bInstant);
}

bool USMEquippableInventoryComponent::IsUnEquippingCurrentEquippable() const
{
	return EquippableChangeStatus == EEquippableChangeStatus::UnEquipping;
}

void USMEquippableInventoryComponent::DropAllEquippables(bool bOwnerBeingDestroyed)
{
	if (!bCachedHasAuthority)
	{
		return;
	}
	
	TArray<ASMEquippableBase*> AllEquippables;
	GetAllEquippablesInInventory(AllEquippables);
	
	for (ASMEquippableBase* equippable : AllEquippables)
	{
		if (CurrentEquippable != equippable)
		{
			DropEquippable(equippable, false);
		}
	}

	if (CurrentEquippable)
	{
		DropEquippable(CurrentEquippable, false, true,true);
	}
}

/* Internal Functions
***********************************************************************************/

void USMEquippableInventoryComponent::BeginUnEquippingCurrentEquippable()
{
	const bool bIsListenServerOrStandaloneController = GetIsListenServerOrStandaloneLocalController();
	if (!bCachedHasAuthority || bIsListenServerOrStandaloneController)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(UnEquipTimerHandle))
		{
			// We are already unequipping our current equippable.
			return;
		}
		
		ApplyEquippableChangingGameplayTag();
		
		const float montageLength = Play1PCurrentEquippableUnEquipAnimations();
		const FTimerDelegate functionDelegate = FTimerDelegate::CreateUObject(this, &USMEquippableInventoryComponent::OnUnEquipFinish);
		GetWorld()->GetTimerManager().SetTimer(UnEquipTimerHandle, functionDelegate, FMath::Max(0.0f, montageLength - 0.01), false);

		if (CurrentEquippable)
		{
			CurrentEquippable->RemoveAbilitiesFromOwner();
		}
		
		// If we're the local listen server on a simulated session, we want to tell all other clients to visually unequip still.
		if (bIsListenServerOrStandaloneController)
		{
			SetEquippableChangeStatus(EEquippableChangeStatus::UnEquipping);
		}
	}
	else
	{
		if (CurrentEquippable)
		{
			CurrentEquippable->RemoveAbilitiesFromOwner();
		}
		
		SetEquippableChangeStatus(EEquippableChangeStatus::UnEquipping);
	}
}

void USMEquippableInventoryComponent::ApplyEquippableChangingGameplayTag()
{
	const ISMFirstPersonInterface* Interface = CurrentEquippable->GetOwnerFirstPersonInterface();
	check(Interface)
		
	USMAbilitySystemComponent* ASC = Interface->GetSMAbilitySystemComponent();
	check(ASC)
		
	const FName TagName = FName("Character.IsChangingEquippable");
	if (!ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TagName)))
	{
		Interface->GetSMAbilitySystemComponent()->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TagName));
	}
}

float USMEquippableInventoryComponent::Play1PCurrentEquippableUnEquipAnimations() const
{
	check(CurrentEquippable)
	
	ISMFirstPersonInterface* interface = CurrentEquippable->GetOwnerFirstPersonInterface();
	check(interface)
	
	float montageLength = 0.25f;
	
	// Play montage on arms
	USkeletalMeshComponent* armsMesh1P = interface->GetMeshOfType(EMeshType::FirstPersonHands);
	if (ensure(armsMesh1P))
	{
		if (UAnimInstance* animInstanceFullBodyMesh1P = armsMesh1P->GetAnimInstance())
		{
			
			const float length = animInstanceFullBodyMesh1P->Montage_Play(CurrentEquippable->UnEquipAnimations.ArmsMontage1P);
			montageLength = length == 0.0f ? 0.25 : length; // We want 0.25 seconds minimum for unequipping regardless.
		}
	}

	// Play montage on equippable 1P
	if (UAnimInstance* animInstanceArms1P = armsMesh1P->GetAnimInstance())
	{
		animInstanceArms1P->Montage_Play(CurrentEquippable->UnEquipAnimations.EquippableMontage1P);
	}
	
	return montageLength;
}

void USMEquippableInventoryComponent::OnUnEquipFinish()
{
	const bool bIsListenServerOrStandaloneController = GetIsListenServerOrStandaloneLocalController();
	if (!bCachedHasAuthority || bIsListenServerOrStandaloneController)
	{
		if (DesiredEquippable.IsValid())
		{
			if (FindEquippableInInventory(DesiredEquippable.Get()))
			{
				// SetCurrentEquippable handles equipping
				SetCurrentEquippable(DesiredEquippable.Get(), false);
				DesiredEquippable.Reset();
			}
		}
		else
		{
			UE_LOG(LogNIX, Log, TEXT("OnUnEquipFinish has finished yet the DesiredEquippable is nullptr."))
			SetCurrentEquippable(nullptr, false);
			
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner())->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.IsChangingEquippable")));
		}
	}
}

void USMEquippableInventoryComponent::SetCurrentEquippable(ASMEquippableBase* equippableToSet, bool bFromReplication)
{
	if (!equippableToSet)
	{
		SM_LOG(Log, TEXT("equippableToSet in SetCurrentEquippable is nullptr. bFromReplication: %i, NetMode: %i"), bFromReplication, static_cast<int32>(CachedOwnerNetMode))
	}
	
	if (!bFromReplication)
	{
		const bool bAlreadyEquipped = CurrentEquippable != nullptr ? (CurrentEquippable == equippableToSet) : false; // Incase CurrentEquippable and equippableToSet are nullptr.
		if ((bAlreadyEquipped && CurrentEquippable != nullptr))
		{
			return;
		}

		const bool bIsListenServerOrStandaloneLocalController = GetIsListenServerOrStandaloneLocalController();
		if (bCachedHasAuthority && bIsListenServerOrStandaloneLocalController == false) // We want standalone/listen server to use client code.
		{
			ClientSetCurrentEquippable(equippableToSet);
			CurrentEquippable = equippableToSet;
		}
		else // if client
		{
			if (bIsListenServerOrStandaloneLocalController == false)
			{
				ServerSetCurrentEquippable(equippableToSet);
			}

			ASMEquippableBase* OldEquippable = CurrentEquippable;

			if (OldEquippable)
			{
				OldEquippable->DetachFromPawn(true, false);
				
				if (bCachedHasAuthority)
				{
					OldEquippable->RemoveAbilitiesFromOwner();
				}
			}
			
			CurrentEquippable = equippableToSet;
			
			if (CurrentEquippable)
			{
				CurrentEquippable->AttachToPawn(true);
			}

			OnCurrentEquippableChanged.Broadcast(OldEquippable);
			
			SetEquippableChangeStatus(EEquippableChangeStatus::Equipping);
		}
	}
	else
	{
		// This part of the code is reached if:
		// A. The server requested the equippable change, and this is the client responding to that change.
		// B. The client requested the equippable change, and this is the server responding to that change.
		// C. The listen server/standalone requested the equippable change, and this is him responding to his own request.
		
		// CurrentEquippable is replicated to simulated proxies, so in the OnRep function that's where animations etc are fired off from (for simulated proxies).
		// OnRep_CurrentEquippable function is called here for listen servers/standalone
		
		ASMEquippableBase* oldEquippable = CurrentEquippable;
		if (/*!bCachedHasAuthority*/true)// we want the client to just attach the pawn, but if we did this then montage events wouldn't fire in abilities.
		{
			if (oldEquippable)
			{
				oldEquippable->DetachFromPawn(true, false);
			}
		}

		CurrentEquippable = equippableToSet;
		
		if (/*!bCachedHasAuthority && */CurrentEquippable)
		{
			CurrentEquippable->AttachToPawn(true);
		}

		if (bCachedHasAuthority)
		{
			if (oldEquippable)
			{
				oldEquippable->RemoveAbilitiesFromOwner();
			}
		}

		// For cases where the listen server needs to visually equip equippables for third person.
		if (IsListenServerOrStandalone())
		{
			OnRep_CurrentEquippable(oldEquippable);
		}
		else
		{
			OnCurrentEquippableChanged.Broadcast(oldEquippable);
		}

		SetEquippableChangeStatus(EEquippableChangeStatus::Equipping);
	}

	if (CurrentEquippable)
	{
		CurrentEquippable->bHasBeenPickedUpBefore = true;
	}

	if (bCachedHasAuthority)
	{
		CheckPerformEquippableDrop();
		
		if (CurrentEquippable)
		{
			CurrentEquippable->AddAbilitiesToOwner();
		}
	}
}

bool USMEquippableInventoryComponent::AddEquippableToInventory(ASMEquippableBase* equippableToAdd)
{
	if (!equippableToAdd || !bCachedHasAuthority)
	{
		return false;
	}

	if (CanAddEquippableToInventory(equippableToAdd))
	{
		if (FInventorySlot* invSlot = FindInventorySlotByTag(equippableToAdd->GetSlotTag()))
		{
			const bool bSuccess = invSlot->AddToSlotInventory(equippableToAdd);
			if (!bSuccess)
			{
				SM_LOG(Warning, TEXT("Could not add equippable to slot inventory in function %s"), ANSI_TO_TCHAR(__FUNCTION__))
				return false;
			}
			else
			{
				equippableToAdd->SetOwner(this->GetOwner());
				OnEquippableAddedToInventory.Broadcast(equippableToAdd, equippableToAdd->GetSlotTag(), invSlot);
				return true;
			}
		}
	}
	else
	{
		return false;
	}
	
	return false;
}

void USMEquippableInventoryComponent::CheckPerformEquippableDrop(bool bForceDrop)
{
	const bool bDesiredEquippableToDropIsNotCurrentEquippable = DesiredEquippableToDrop.Get() != CurrentEquippable;
	if (bCachedHasAuthority && DesiredEquippableToDrop.IsValid() && (bDesiredEquippableToDropIsNotCurrentEquippable || bForceDrop))
	{
		ASMEquippableBase* equippableToDrop = DesiredEquippableToDrop.Get();
		
		// remove from inventory
		FInventorySlot* slot = FindInventorySlotByTag(equippableToDrop->GetSlotTag());
		if (slot)
		{
			if (slot->RemoveFromSlotInventory(equippableToDrop) == false)
			{
				SM_LOG(Warning, TEXT("CheckPerformEquippableDrop could not remove equippable from slot inventory."))
				return; // @TODO: should we return here?
			}
		}

		// Stops the equippable from being picked up instantly.
		equippableToDrop->SetDropTime(RePickUpTime);
		
		// set owner to nullptr (this will free it from being invisible and have physics once again etc)
		equippableToDrop->SetOwner(nullptr);

		const AActor* compOwner = GetOwner();
		check(compOwner)

		FVector cameraLoc;
		FRotator cameraRot;
		compOwner->GetActorEyesViewPoint(cameraLoc, cameraRot);
		
		const FVector dropLocation = cameraLoc + (RelativeDropLocation);
		const FRotator dropRotation = compOwner->GetActorRotation() - FRotator(90.f, 0.f, 0.f);

		equippableToDrop->SetActorLocationAndRotation(dropLocation, dropRotation, false, nullptr, ETeleportType::ResetPhysics);
		
		// Throw the equippable in the world;
		const float Velocity = DropVelocity;
		const FVector ImpulseToAdd = cameraRot.Vector() * Velocity + FVector(0.f, 0.f, UpVelocity);
		equippableToDrop->GetWorldMesh()->AddImpulse(ImpulseToAdd, NAME_None, true);

		equippableToDrop->ForceNetUpdate();
		
		// Tell all clients to perform the throw on their ends.
		equippableToDrop->NetMulticastReceiveDropInformation(dropLocation, dropRotation, ImpulseToAdd);

		DesiredEquippableToDrop = nullptr;
	}
}

void USMEquippableInventoryComponent::SetEquippableChangeStatus(EEquippableChangeStatus NewStatus)
{
	if (bCachedHasAuthority)
	{
		const EEquippableChangeStatus OldStatus = EquippableChangeStatus;
		EquippableChangeStatus = NewStatus;
		
		if (IsListenServerOrStandalone())
		{
			OnRep_EquippableChangeStatus(OldStatus);
		}
	}
}

void USMEquippableInventoryComponent::ForceDropCurrentEquippableBeforeDestroy()
{
	if (!IsValid(this))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("ForceDropCurrentEquippableBeforeDestroy called when object is not valid."));
		}
		return;
	}
	
	if (CurrentEquippable)
	{
		DesiredEquippableToDrop = CurrentEquippable;
		CheckPerformEquippableDrop(true);
	}
}

/* Helper Functions
***********************************************************************************/

bool USMEquippableInventoryComponent::AlreadyHasEquippable(TSubclassOf<ASMEquippableBase> equippable)
{
	return FindEquippableByClass(equippable) != nullptr;
}

ASMEquippableBase* USMEquippableInventoryComponent::FindEquippableByClass(TSubclassOf<ASMEquippableBase> equippableToFind)
{
	if (equippableToFind)
	{
		if (FInventorySlot* invSlot = FindInventorySlotByTag(equippableToFind->GetDefaultObject<ASMEquippableBase>()->GetSlotTag()))
		{
			for(ASMEquippableBase* equippable : invSlot->SlotInventory)
			{
				if (equippable->GetSlotTag() == equippableToFind->GetDefaultObject<ASMEquippableBase>()->GetSlotTag())
				{
					return equippable;
				}
			}
		}
	}

	return nullptr;
}

bool USMEquippableInventoryComponent::FindEquippableInInventory(ASMEquippableBase* equippableToFind)
{
	if (equippableToFind)
	{

		if (const FInventorySlot* invSlot = FindInventorySlotByTag(equippableToFind->GetSlotTag()))
		{
			if (invSlot->SlotInventory.Contains(equippableToFind))
			{
				return true;
			}
		}
	}

	return false;
}

bool USMEquippableInventoryComponent::CheckEquippableSlot(FGameplayTag slotGameplayTag)
{
	if (slotGameplayTag.IsValid())
	{
		if (slotGameplayTag == FGameplayTag::RequestGameplayTag("EquippableSlot.NoSlot"))
		{
			return bDisallowSlotlessEquippables;
		}
		return IsSlotFull(slotGameplayTag) == false;
	}

	UE_LOG(LogNIX, Error, TEXT("CheckEquippableSlot called with invalid GameplayTag in class %s"), *GetNameSafe(this))
	
	return true;
}

bool USMEquippableInventoryComponent::CanAddEquippableToInventory(const ASMEquippableBase* equippableToCheck)
{
	if (equippableToCheck == nullptr)
	{
		return false;
	}
	
	const bool bAlreadyHasOwner = equippableToCheck->GetOwner() != nullptr;
	const bool bSlotFull = IsSlotFull(equippableToCheck->GetSlotTag());
	const bool bAlreadyHasEquippable = AlreadyHasEquippable(equippableToCheck->StaticClass());
	if (bAlreadyHasOwner || bSlotFull || bAlreadyHasEquippable)
	{
		SM_LOG(Log, TEXT("CanAddEquippableToInventory returned false."))
		SM_LOG(Log, TEXT("bAlreadyHasOwner: %i, bSlotFull: %i, bAlreadyHasEquippable: %i"), bAlreadyHasOwner, bSlotFull, bAlreadyHasEquippable)
		return false;
	}
	
	return true;
}

bool USMEquippableInventoryComponent::IsSlotFull(FGameplayTag slotToCheck)
{
	if (FInventorySlot* inventorySlot = FindInventorySlotByTag(slotToCheck))
	{
		return inventorySlot->IsFull();
	}

	return false;
}

FInventorySlot* USMEquippableInventoryComponent::FindInventorySlotByTag(FGameplayTag slotTag)
{
	if (slotTag.IsValid())
	{
		for (FInventorySlot& invSlot : EquippableInventory)
		{
			if (invSlot.SlotTag == slotTag)
			{
				return &invSlot;
			}
		}
	}

	return nullptr;
}

void USMEquippableInventoryComponent::GetAllEquippablesInInventory(TArray<ASMEquippableBase*>& OutEquippables)
{
	for (FInventorySlot& slot : EquippableInventory)
	{
		for (ASMEquippableBase* equippable : slot.SlotInventory)
		{
			OutEquippables.Add(equippable);
		}
	}
}

void USMEquippableInventoryComponent::NetMulticastVisuallyUnEquip_Implementation(ASMEquippableBase* equippableToUnEquip)
{
	if (!IsValid(this))
	{
		return;
	}
	
	if (GetOwnerRole() == ROLE_AutonomousProxy && IsListenServerOrStandalone())
	{
		return;
	}
	
	if (equippableToUnEquip)
	{
		equippableToUnEquip->DetachFromPawn(false, false);
	}
}

/* Networking
***********************************************************************************/

void USMEquippableInventoryComponent::ClientSetCurrentEquippable_Implementation(ASMEquippableBase* equippableToSet)
{
	SetCurrentEquippable(equippableToSet, true);
}

void USMEquippableInventoryComponent::ServerSetCurrentEquippable_Implementation(ASMEquippableBase* equippableToSet)
{
	SetCurrentEquippable(equippableToSet, true);
}

void USMEquippableInventoryComponent::ServerAttemptEquip_Implementation(ASMEquippableBase* desiredEquippable)
{
	AttemptEquip(desiredEquippable, true);
}

void USMEquippableInventoryComponent::ClientAttemptEquip_Implementation(ASMEquippableBase* desiredEquippable)
{
	AttemptEquip(desiredEquippable, true);
}

void USMEquippableInventoryComponent::ServerDropEquippable_Implementation(ASMEquippableBase* equippableToDrop, bool bInstant, bool bDontFindNextEquippable)
{
	DropEquippable(equippableToDrop, true, bDontFindNextEquippable, bInstant);
}

void USMEquippableInventoryComponent::ClientDropEquippable_Implementation(ASMEquippableBase* equippableToDrop, bool bInstant, bool bDontFindNextEquippable)
{
	DropEquippable(equippableToDrop, true, bDontFindNextEquippable, bInstant);
}

void USMEquippableInventoryComponent::OnRep_CurrentEquippable(ASMEquippableBase* OldEquippable)
{
	if (CurrentEquippable)
	{
		CurrentEquippable->AttachToPawn(false);
	}
	
	OnCurrentEquippableChanged.Broadcast(OldEquippable);
}

void USMEquippableInventoryComponent::OnRep_EquippableChangeStatus(EEquippableChangeStatus OldEquippableChangeStatus)
{
	if (EquippableChangeStatus == EEquippableChangeStatus::UnEquipping)
	{
		if (CurrentEquippable)
		{
			CurrentEquippable->DetachFromPawn(false, false);
		}
	}
	else if (EquippableChangeStatus == EEquippableChangeStatus::Equipping)
	{
		if (CurrentEquippable)
		{
			CurrentEquippable->AttachToPawn(false);
		}
	}
}

void USMEquippableInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USMEquippableInventoryComponent, CurrentEquippable, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(USMEquippableInventoryComponent, EquippableInventory, COND_OwnerOnly);
	DOREPLIFETIME(USMEquippableInventoryComponent, EquippableChangeStatus);
}
