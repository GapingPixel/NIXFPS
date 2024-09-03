// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_ADS.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Animation/SMAnimHandsFP.h"
#include "Possessables/SMBaseCharacter.h"

void USMGA_ADS::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	ArmsAnimInstance = Cast<USMAnimHandsFP>(Cast<ASMBaseCharacter>( GetAvatarActorFromActorInfo())->GetMeshOfType(EMeshType::FirstPersonHands)->GetAnimInstance());
	ArmsAnimInstance->bShouldADS = true;
	AsyncTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
	AsyncTask->OnRelease.AddDynamic(this, &USMGA_ADS::OnInputRelease);
	AsyncTask->ReadyForActivation();
}

void USMGA_ADS::OnInputRelease(float TimeHeld)
{
	K2_EndAbility();
}

void USMGA_ADS::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	AsyncTask->EndTask();
	ArmsAnimInstance->bShouldADS = false;
}

