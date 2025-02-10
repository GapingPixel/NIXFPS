// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMLiftableObject.generated.h"

class ASMPlayerCharacter;

UCLASS()
class NIX_API ASMLiftableObject : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ASMLiftableObject();

	void AddPlayerInteraction(ASMPlayerCharacter* Player);
	void RemovePlayerInteraction(ASMPlayerCharacter* Player);
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Weight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float LiftHeight;

	UPROPERTY()
	TMap<TObjectPtr<ASMPlayerCharacter>, FVector2D> ContributingPlayersInputs;
	
	bool bIsLifted;
	
	virtual void Tick(float DeltaTime) override;
	FVector2D CalculateMovementDirection() const;

	void LiftObject();
	void DropObject();
	
};
