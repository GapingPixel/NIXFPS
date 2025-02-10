// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/SMGameplayAbility.h"
#include "GA_Interact.generated.h"

class ASMPlayerCharacter;
/**
 * 
 */
UCLASS()
class NIX_API UGA_Interact : public USMGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	UPROPERTY()
	TObjectPtr<ASMPlayerCharacter> Player;

	UPROPERTY()
	TObjectPtr<AActor> ActorToInteract;
};
