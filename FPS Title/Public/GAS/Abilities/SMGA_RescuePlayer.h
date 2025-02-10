// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/SMGameplayAbility.h"
#include "SMGA_RescuePlayer.generated.h"

class ASMPlayerCharacter;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_RescuePlayer : public USMGameplayAbility
{
	GENERATED_BODY()

	USMGA_RescuePlayer();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	UPROPERTY()
	TObjectPtr<ASMPlayerCharacter> Player;
public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category ="Stats" )
	float TimeToRescuePlayer;
};
