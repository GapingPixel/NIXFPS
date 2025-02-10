// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/AttributeSets/SMBaseAttributeSet.h"
#include "SMCombatAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API USMCombatAttributeSet : public USMBaseAttributeSet
{
	GENERATED_BODY()

public:
	
	ATTRIBUTE_ACCESSORS(USMCombatAttributeSet, BaseDamage);

	void AdjustAttributesForDifficulty();
protected:

	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);
	

	
private:

	// The base amount of damage to apply in the damage execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "Lyra|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseDamage;
	
};
