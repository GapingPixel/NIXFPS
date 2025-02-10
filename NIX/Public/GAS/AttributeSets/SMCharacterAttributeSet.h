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

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_SanityLVL)
	FGameplayAttributeData SanityLVL;
	ATTRIBUTE_ACCESSORS(USMCharacterAttributeSet, SanityLVL)
	
protected:
	UFUNCTION()
	virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);

	UFUNCTION()
	virtual void OnRep_SanityLVL(const FGameplayAttributeData& OldSanityLVL);
};

/*
UCLASS()
class NIX_API  USMPlayerAttributeSet : public USMCharacterAttributeSet
{
	GENERATED_BODY()

public:
	USMPlayerAttributeSet();
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_KillCount)
	FGameplayAttributeData KillCount;
	ATTRIBUTE_ACCESSORS(USMPlayerAttributeSet, KillCount)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_DeathCount)
	FGameplayAttributeData DeathCount;
	ATTRIBUTE_ACCESSORS(USMPlayerAttributeSet, DeathCount)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_RescueCount)
	FGameplayAttributeData RescueCount;
	ATTRIBUTE_ACCESSORS(USMPlayerAttributeSet, RescueCount)

	FireAssist

	FriendlyFireCount

	DamageDealt
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_RescueCount)
	FGameplayAttributeData RescueCount;
	ATTRIBUTE_ACCESSORS(USMPlayerAttributeSet, RescueCount)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_RescueCount)
	FGameplayAttributeData RescueCount;
	ATTRIBUTE_ACCESSORS(USMPlayerAttributeSet, RescueCount)
	
protected:
	UFUNCTION()
	virtual void OnRep_KillCount(const FGameplayAttributeData& OldKillCount);

	UFUNCTION()
	virtual void OnRep_DeathCount(const FGameplayAttributeData& OldDeathCount);
};*/