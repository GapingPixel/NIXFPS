// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AbilityBindingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UAbilityBindingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NIX_API IAbilityBindingInterface
{
	GENERATED_BODY()

	
public:

	/* Ability binding
	***********************************************************************************/
	
	virtual void BindAbility(struct FGameplayAbilitySpec& Spec) const = 0;

	virtual void UnbindAbility(struct FGameplayAbilitySpecHandle Handle) const = 0;
};
