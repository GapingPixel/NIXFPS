// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AttributeSets/SMCharacterAttributeSet.h"

#include "Net/UnrealNetwork.h"

USMCharacterAttributeSet::USMCharacterAttributeSet()
{
	
}

void USMCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void USMCharacterAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMCharacterAttributeSet, MovementSpeed, OldMovementSpeed)
}

void USMCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USMCharacterAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
}
