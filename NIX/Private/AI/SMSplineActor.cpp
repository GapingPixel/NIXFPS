// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SMSplineActor.h"

#include "Components/SplineComponent.h"

// Sets default values
ASMSplineActor::ASMSplineActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	SetRootComponent(DefaultSceneRoot);
	SplineComponent = CreateDefaultSubobject<USplineComponent>("SplineComponent");
	SplineComponent->SetupAttachment(RootComponent);
}



