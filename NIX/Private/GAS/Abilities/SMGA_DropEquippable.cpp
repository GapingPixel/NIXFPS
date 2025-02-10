// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_DropEquippable.h"

#include "Possessables/SMBaseCharacter.h"

void USMGA_DropEquippable::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	Cast<ASMBaseCharacter>(GetActorInfo().AvatarActor)->InventoryComponent->DropCurrentEquippable(true);
	K2_EndAbility();
}
