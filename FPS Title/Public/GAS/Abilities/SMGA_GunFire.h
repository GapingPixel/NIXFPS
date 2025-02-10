// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/SMGA_Fire.h"
#include "SMGA_GunFire.generated.h"

class UAbilityTask_WaitGameplayEvent;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_GunFire : public USMGA_Fire
{
	GENERATED_BODY()

public:

	USMGA_GunFire();
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	bool bAllowLastFire;

	TObjectPtr<UAbilityTask_WaitGameplayEvent> StartSprintingTask;

	virtual UAnimMontage* DetermineFireAnimation(EEMeshType MeshType = EEMeshType::Arms1P) const override;
	UFUNCTION()
	void OnSprintReceived(FGameplayEventData Payload);

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	
};
