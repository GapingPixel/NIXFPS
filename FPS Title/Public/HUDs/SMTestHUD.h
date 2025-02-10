// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SMEquippableInventoryComponent.h"
#include "GameFramework/HUD.h"
#include "SMTestHUD.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API ASMTestHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class ASMPlayerCharacter* OwningCharacter;

	UPROPERTY()
	class USMHealthComponent* HealthComp;
	
public:
	friend USMEquippableInventoryComponent;
};
