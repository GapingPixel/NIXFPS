// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AttributeSets/SMCombatAttributeSet.h"

#include "Net/UnrealNetwork.h"

void USMCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USMCombatAttributeSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
}

void USMCombatAttributeSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMCombatAttributeSet, BaseDamage, OldValue);
}
