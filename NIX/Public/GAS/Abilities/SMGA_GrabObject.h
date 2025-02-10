// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/SMGameplayAbility.h"
#include "SMGA_GrabObject.generated.h"

class ASMPlayerCharacter;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_GrabObject : public USMGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GEStaminaCost;

	UPROPERTY(EditDefaultsOnly)
	float StaminaCostPerSecond = -2;
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	
	
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	UFUNCTION(Server, Reliable)
	void EndGrab();
	
	UPROPERTY()
	TObjectPtr<ASMPlayerCharacter> Player;

	FTimerHandle StaminaHandle;
	FTimerHandle LiftableHandle;

	UPROPERTY()
	TObjectPtr<AActor> GrabbedObject;
};
