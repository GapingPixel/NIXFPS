// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMDeathAbility.h"

#include "AbilitySystemComponent.h"
#include "Components/SMHealthComponent.h"
#include "Possessables/SMBaseCharacter.h"

void USMDeathAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo)
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	FGameplayTagContainer AbilityTypesToIgnore;
	ASC->CancelAbilities(nullptr, nullptr, this);

	SetCanBeCanceled(false);

	USMHealthComponent* HealthComp = USMHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo());
	check(HealthComp)
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void USMDeathAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
