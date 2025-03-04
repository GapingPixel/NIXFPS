// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/SMGameplayAbility.h"
#include "SMGA_Sprint.generated.h"

class UAbilityTask_WaitInputRelease;
class USMCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_Sprint : public USMGameplayAbility
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USMCharacterMovementComponent> MoveComp;

	UPROPERTY()
	UAbilityTask_WaitInputRelease* AsyncTask;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	void OnInputRelease(float TimeHeld);
};

UCLASS()
class NIX_API USMGA_Jump : public USMGameplayAbility
{
	GENERATED_BODY()

	UPROPERTY()
	UAbilityTask_WaitInputRelease* AsyncTask;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GEStaminaCost;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Costs")
	float StaminaCost = -30.0f;  
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags)  const override;
	
	UFUNCTION()
	void OnInputRelease(float TimeHeld);
};
