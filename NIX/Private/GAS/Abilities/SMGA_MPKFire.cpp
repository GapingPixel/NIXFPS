// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_MPKFire.h"
#include "AbilitySystemComponent.h"

UAnimMontage* USMGA_MPKFire::DetermineFireAnimation(const EEMeshType MeshType) const
{
	if (MeshType == EEMeshType::Arms1P && GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Aiming"))))
	{
		return AMOverlayADS;
	}
	return Super::DetermineFireAnimation(MeshType);
}
