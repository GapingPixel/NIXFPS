// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/SMGameplayAbility.h"
#include "SMGA_Rest.generated.h"

class UAbilityTask_WaitInputRelease;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_Rest : public USMGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	float TimeToActivate = 5;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	float SanityToRestorePerSec = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> SanityGE;
protected:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	FTimerHandle Timer;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputRelease> AsyncTask_WaitInputRelease;
	UFUNCTION()
	void OnKeyReleased(float TimeHeld);
};
