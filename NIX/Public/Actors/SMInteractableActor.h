// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMInteractableActor.generated.h"

UCLASS()
class NIX_API ASMInteractableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASMInteractableActor();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UFUNCTION(NetMulticast, Reliable)
	void UpdateCollisionForPlayerHandling();
	void UpdateCollisionForWorld();
};
