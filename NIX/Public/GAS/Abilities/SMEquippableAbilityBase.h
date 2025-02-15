// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/SMGameplayAbility.h"
#include "SMEquippableAbilityBase.generated.h"

class ASMGunBase;
/**
 * 
 */
UCLASS()
class NIX_API USMEquippableAbilityBase : public USMGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Get the Equippable from SourceObject
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NIX|Ability")
	ASMEquippableBase* GetEquippable() const;

	// Get the gun from SourceObject (if the Equippable is a gun)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NIX|Ability")
	ASMGunBase* GetGun() const;

protected:
	// Set to true if you want this ability to count towards the current equippable interaction count.
	UPROPERTY(EditDefaultsOnly, Category = "NIX|Ability")
	bool bAddToEquippableInteractionCount = true;

private:
	bool bAddedToInteractCount = false;
	
	UPROPERTY()
	TWeakObjectPtr<ASMEquippableBase> OwnerEquippable;
};
