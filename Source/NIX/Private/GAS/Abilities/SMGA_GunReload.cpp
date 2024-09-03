// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_GunReload.h"

#include "Items/SMGunBase.h"

USMGA_GunReload::USMGA_GunReload()
{
	bAllowLastFireReloadAnimation = true;
	EmptyMagReloadTime = 1.0;
}

UAnimMontage* USMGA_GunReload::DetermineReloadMontageArms1P() const
{
	const auto GunLocal = GetGunEquippable();
	if (bAllowLastFireReloadAnimation && GunLocal->GetCurrentAmmo() == 0)
	{
		return GunLocal->EmptyReloads.ArmsMontage1P;
	} 
	return USMGA_Reload::DetermineReloadMontageArms1P();
}

UAnimMontage* USMGA_GunReload::DetermineReloadMontageArms3P() const
{
	const auto GunLocal = GetGunEquippable();
	if (bAllowLastFireReloadAnimation && GunLocal->GetCurrentAmmo() == 0)
	{
		return GunLocal->EmptyReloads.FullBodyMontage3P;
	} 
	return Super::DetermineReloadMontageArms3P();
}

UAnimMontage* USMGA_GunReload::DetermineReloadMontageEquippable1P() const
{
	const auto GunLocal = GetGunEquippable();
	if (bAllowLastFireReloadAnimation && GunLocal->GetCurrentAmmo() == 0)
	{
		return GunLocal->EmptyReloads.EquippableMontage1P;
	} 
	return Super::DetermineReloadMontageEquippable1P();
}

UAnimMontage* USMGA_GunReload::DetermineReloadMontageEquippable3P() const
{
	const auto GunLocal = GetGunEquippable();
	if (bAllowLastFireReloadAnimation && GunLocal->GetCurrentAmmo() == 0)
	{
		return GunLocal->EmptyReloads.EquippableMontage3P;
	} 
	return Super::DetermineReloadMontageEquippable3P();
}

float USMGA_GunReload::DetermineReloadTime() const
{
	if (GetGunEquippable()->GetCurrentAmmo() == 0)
	{
		return EmptyMagReloadTime;
	}
	return Super::DetermineReloadTime();
}
