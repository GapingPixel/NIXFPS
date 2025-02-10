// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MonsterHallucination.h"

// Sets default values
AMonsterHallucination::AMonsterHallucination()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMonsterHallucination::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(30);
}



