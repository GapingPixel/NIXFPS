// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "SMItemBase.generated.h"

class USMEquippableInventoryComponent;
UCLASS()
class ASMItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ASMItemBase();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	/* Components
	***********************************************************************************/

protected:
	
	// The mesh that this item uses when dropped in the world.
	UPROPERTY(VisibleDefaultsOnly, Category = Item)
	TObjectPtr<UStaticMeshComponent> WorldMeshComponent;

	// The collision sphere that detects actors that have an inventory component.
	UPROPERTY(VisibleDefaultsOnly, Category = Item)
	TObjectPtr<USphereComponent> SphereCollisionComponent;
	
	/* Item Functions
	***********************************************************************************/

protected:

	virtual void PickUpTick();
	
public:
	
	// Called when an actor with an inventory attempts to "pick up" this item.
	virtual void OnPickUp(USMEquippableInventoryComponent* Inventory);

	// Called when an actor with an inventory component tries to pick up this item.
	virtual bool CanPickUp();

	// Sets the last time in world seconds when this item was dropped.
	void SetDropTime(float dropTime);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastReceiveDropInformation(FVector_NetQuantize DropLocation, FRotator DropRotation, FVector_NetQuantize Impulse);
	
	/* Blueprint Exposed
	***********************************************************************************/

public:
	
	// Called when an actor with an inventory attempts to "pick up" this item. Inventory cannot be null. Only called on server.
	UFUNCTION(BlueprintImplementableEvent, Category = Item, meta=(DisplayName="OnPickUp"))
	void BP_OnPickUp(USMEquippableInventoryComponent* Inventory);

	// Called when an actor with an inventory component tries to pick up this item.
	UFUNCTION(BlueprintImplementableEvent, Category = Item, meta=(DisplayName="CanPickUp"))
	void BP_CanPickUp();

	/* Getters
	***********************************************************************************/

public:

	UStaticMeshComponent* GetWorldMesh() const { return WorldMeshComponent; };

	/* Other (uncategorized, at least for now)
***********************************************************************************/
	
private:

	float DropTime = 0.0f;

	FTimerHandle PickupTickTimerHandle;
	
};
