// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/SMGameplayAbility.h"
#include "SMGA_Suicide.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API USMGA_Suicide : public USMGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> HealthGE;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	float TimeToActivate = 5;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
};
