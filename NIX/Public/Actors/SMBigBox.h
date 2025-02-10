// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMBigBox.generated.h"

class UBoxComponent;

UCLASS()
class NIX_API ASMBigBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASMBigBox();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> BoxComponent;

	virtual void PostInitializeComponents() override;

	UFUNCTION()
	void CheckAmmountOfLittleBoxes(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	int8 LittleBoxCount;
	int8  LittleBoxTarget;
};
