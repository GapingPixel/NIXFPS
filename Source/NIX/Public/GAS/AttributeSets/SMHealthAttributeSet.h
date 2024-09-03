// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSets/SMBaseAttributeSet.h"
#include "SMHealthAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API USMHealthAttributeSet : public USMBaseAttributeSet
{
	GENERATED_BODY()

public:

	USMHealthAttributeSet();
	
	ATTRIBUTE_ACCESSORS(USMHealthAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(USMHealthAttributeSet, MaxHealth)

	// Delegate to broadcast when the health attribute reaches zero.
	mutable FSMAttributeEvent OnOutOfHealth;

protected:
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

private:
	
	// The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "SpawnMaster|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	// The current max health attribute.  Max health is an attribute since gameplay effects can modify it.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "SpawnMaster|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

	// Used to track when the health reaches 0.
	bool bOutOfHealth = false;
};
