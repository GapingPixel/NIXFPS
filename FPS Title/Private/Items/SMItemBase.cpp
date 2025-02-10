// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SMItemBase.h"
#include "Components/SMEquippableInventoryComponent.h"
#include "NIX/NIX.h"

DECLARE_CYCLE_STAT(TEXT("ItemTick"), STAT_ItemTick, STATGROUP_NIX);

ASMItemBase::ASMItemBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickInterval = 0.0;

	WorldMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WorldMeshComp"));
	WorldMeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	WorldMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	WorldMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	WorldMeshComponent->SetLinearDamping(0.5f);
	RootComponent = WorldMeshComponent;
	
	SphereCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollisionComponent->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	SphereCollisionComponent->SetCollisionResponseToChannel(COLLISION_SMCHARACTERBASE, ECollisionResponse::ECR_Overlap);
	SphereCollisionComponent->SetupAttachment(RootComponent);
	SetCanBeDamaged(false);
	bReplicates = true;
	SetReplicateMovement(true);
}

/* Item Functions
***********************************************************************************/

void ASMItemBase::PickUpTick()
{
	SCOPE_CYCLE_COUNTER(STAT_ItemTick)
	
	if (!HasAuthority())
	{
		return;
	}

	if (GetWorld()->GetTimeSeconds() < DropTime)
	{
		return;
	}

	// Get all actors that are overlapping
	TArray<AActor*> OutOverlappingActors;
	GetOverlappingActors(OutOverlappingActors);

	// We do this check so when there are no actors overlapping, we can turn off tick.
	if (OutOverlappingActors.Num() > 0)
	{
		for (const AActor* Actor : OutOverlappingActors)
		{
			check(Actor) // Not sure if we need this, but from experience i've has such a rare case where the element in a foreach loop can be nullptr.

			// @TODO: change this to just check if the actor has an equippable inventory component, then just pass the actor to OnPickUp. 
			// We check if it has the component first, so if it doesn't have it we can skip doing the cast.
			if (UActorComponent* ActorComponent = Actor->GetComponentByClass(USMEquippableInventoryComponent::StaticClass()))
			{
				if (USMEquippableInventoryComponent* InventoryComponent = Cast<USMEquippableInventoryComponent>(ActorComponent))
				{
					OnPickUp(InventoryComponent);
				}
			}
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(PickupTickTimerHandle);
	}
}

void ASMItemBase::OnPickUp(USMEquippableInventoryComponent* Inventory)
{
	if (Inventory)
	{
		BP_OnPickUp(Inventory);
	}
}

bool ASMItemBase::CanPickUp()
{
	return GetOwner() == nullptr;
}

void ASMItemBase::SetDropTime(const float dropTime)
{
	DropTime = GetWorld()->GetTimeSeconds() + dropTime;
}

void ASMItemBase::NetMulticastReceiveDropInformation_Implementation(FVector_NetQuantize DropLocation, FRotator DropRotation, FVector_NetQuantize Impulse)
{
	SetActorLocation(DropLocation);
	SetActorRotation(DropRotation);

	// @TODO: This gets called before simulating physics is true again.
	//WorldMeshComponent->AddImpulse(Impulse, NAME_None, true);
}

/* Blueprint Exposed
***********************************************************************************/



/* Other (uncategorized)
***********************************************************************************/

void ASMItemBase::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// enable tick if the overlapping actor has an inventory component (server only).
	if (HasAuthority() && !GetOwner() && OtherActor->GetComponentByClass(USMEquippableInventoryComponent::StaticClass()))
	{
		GetWorld()->GetTimerManager().SetTimer(PickupTickTimerHandle, this, &ASMItemBase::PickUpTick, 0.25f, true, 0.0f);
	}
}

