// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSets/SMBaseAttributeSet.h"
#include "SMCharacterAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API USMCharacterAttributeSet : public USMBaseAttributeSet
{
	GENERATED_BODY()

public:
	USMCharacterAttributeSet();
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_MovementSpeed)
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(USMCharacterAttributeSet, MovementSpeed)
	
protected:
	UFUNCTION()
	virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);
};
