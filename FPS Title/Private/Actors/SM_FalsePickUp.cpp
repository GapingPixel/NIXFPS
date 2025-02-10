// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/SM_FalsePickUp.h"

// Sets default values
ASM_FalsePickUp::ASM_FalsePickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	WorldMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WorldMeshComp"));
	WorldMeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	WorldMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	WorldMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	WorldMeshComponent->SetLinearDamping(0.5f);
	RootComponent = WorldMeshComponent;

}

void ASM_FalsePickUp::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(40);
}





