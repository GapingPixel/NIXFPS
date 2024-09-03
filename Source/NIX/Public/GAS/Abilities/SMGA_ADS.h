// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/SMEquippableAbilityBase.h"
#include "SMGA_ADS.generated.h"

class UAbilityTask_WaitInputRelease;
class USMAnimHandsFP;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_ADS : public USMEquippableAbilityBase
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TObjectPtr<USMAnimHandsFP> ArmsAnimInstance;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputRelease> AsyncTask;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnInputRelease(float TimeHeld);
};
