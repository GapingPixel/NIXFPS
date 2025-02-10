// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/SMGA_Reload.h"
#include "SMGA_GunReload.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API USMGA_GunReload : public USMGA_Reload
{
	GENERATED_BODY()

public:

	USMGA_GunReload();
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Reload Settings", meta=(MultiLine="true"))
	bool bAllowLastFireReloadAnimation;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Reload Settings", meta=(MultiLine="true"))
	float EmptyMagReloadTime;

	virtual UAnimMontage* DetermineReloadMontageArms1P() const override;
	virtual UAnimMontage* DetermineReloadMontageArms3P() const override;
	virtual UAnimMontage* DetermineReloadMontageEquippable1P() const override;
	virtual UAnimMontage* DetermineReloadMontageEquippable3P() const override;
	virtual float DetermineReloadTime() const override;
};
