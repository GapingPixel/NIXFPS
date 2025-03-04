// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/SMEquippableAbilityBase.h"
#include "SMGA_SanityPill.generated.h"

class USMPlayerAttributesComponent;
class UAbilityTask_WaitInputRelease;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_SanityPill : public USMEquippableAbilityBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	float TimeToActivate = 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	float SanityToRestore = 35;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> SanityGE;
protected:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	FTimerHandle Timer;

	UPROPERTY()
	UAbilityTask_WaitInputRelease* AsyncTask_WaitInputRelease;
	UFUNCTION()
	void OnKeyReleased(float TimeHeld);
	
};
