// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/SMGA_GunFire.h"
#include "SMGA_HKUSPFire.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API USMGA_HKUSPFire : public USMGA_GunFire
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TObjectPtr<UAnimMontage> AMOverlayADS;
	
	virtual UAnimMontage* DetermineFireAnimation(const EEMeshType MeshType) const override;
};
