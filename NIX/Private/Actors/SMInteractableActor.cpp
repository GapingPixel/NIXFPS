// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/SMInteractableActor.h"

#include "Net/UnrealNetwork.h"
#include "NIX/NIX.h"

// Sets default values
ASMInteractableActor::ASMInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->SetComponentTickEnabled(false);
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetComponentTickEnabled(false);
	StaticMeshComponent->SetIsReplicated(true);
	SetRootComponent(StaticMeshComponent);

	UpdateCollisionForWorld();
	/**/
	
}
/*
void ASMInteractableActor::UpdateCollisionForPlayerHandling()
{
	
}*/

void ASMInteractableActor::UpdateCollisionForPlayerHandling_Implementation()
{
	StaticMeshComponent->SetCollisionResponseToChannel(COLLISION_SMCHARACTERBASE, ECR_Ignore);
}

void ASMInteractableActor::UpdateCollisionForWorld()
{
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	StaticMeshComponent->SetCollisionResponseToChannel(COLLISION_SMCHARACTERBASE, ECR_Block);
}

void ASMInteractableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASMInteractableActor, StaticMeshComponent);
}





