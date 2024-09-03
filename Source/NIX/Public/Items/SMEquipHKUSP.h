// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/SMGunBase.h"
#include "SMEquipHKUSP.generated.h"

class USMAnim_EquippableHKUSP;
/**
 * 
 */
UCLASS()
class NIX_API ASMEquipHKUSP : public ASMGunBase
{
	GENERATED_BODY()

	
	TObjectPtr<USMAnim_EquippableHKUSP> AnimInstanceFP;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	void CurrentAmmoChanged(float OldAmmo, float NewAmmo);
};
