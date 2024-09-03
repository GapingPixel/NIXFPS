// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMEquippableAbilityBase.h"
#include "Items/SMGunBase.h"

void USMEquippableAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	bAddedToInteractCount = false;
	
	if (FGameplayAbilitySpec* spec = GetCurrentAbilitySpec())
	{
		OwnerEquippable = Cast<ASMEquippableBase>(spec->SourceObject);
	}

	if (bAddToEquippableInteractionCount)
	{
		ASMEquippableBase* equippable = GetEquippable();
		if (ensure(equippable))
		{
			equippable->AddOrRemoveFromInteractionCount(true);
			bAddedToInteractCount = true;
		}
	}

	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void USMEquippableAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (bAddToEquippableInteractionCount && bAddedToInteractCount)
	{
		ASMEquippableBase* equippable = GetEquippable();
		if (ensure(equippable))
		{
			equippable->AddOrRemoveFromInteractionCount(false);
		}
	}
}

ASMEquippableBase* USMEquippableAbilityBase::GetEquippable() const
{
	if (OwnerEquippable.IsValid())
	{
		return OwnerEquippable.Get();
	}
	else
	{
		if (FGameplayAbilitySpec* spec = GetCurrentAbilitySpec())
		{
			return Cast<ASMEquippableBase>(spec->SourceObject);
		}
	}

	return nullptr;
}

ASMGunBase* USMEquippableAbilityBase::GetGun() const
{
	if (FGameplayAbilitySpec* spec = GetCurrentAbilitySpec())
	{
		return Cast<ASMGunBase>(spec->SourceObject);
	}

	return nullptr;
}
