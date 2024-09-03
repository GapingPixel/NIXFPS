// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "SMDamageExecution.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API USMDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	USMDamageExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
