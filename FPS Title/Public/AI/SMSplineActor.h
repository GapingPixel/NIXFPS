// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMSplineActor.generated.h"

class USplineComponent;

UCLASS()
class NIX_API ASMSplineActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASMSplineActor();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USceneComponent> DefaultSceneRoot;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USplineComponent> SplineComponent;


};
