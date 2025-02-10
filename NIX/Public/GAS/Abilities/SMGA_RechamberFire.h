// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/SMGA_GunFire.h"
#include "SMGA_RechamberFire.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API USMGA_RechamberFire : public USMGA_GunFire
{
	GENERATED_BODY()

public:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void OnRangedWeaponTargetDataReadyCPP(const FGameplayAbilityTargetDataHandle& TargetData) override;
	
};
