// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_Sprint.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Components/SMCharacterMovementComponent.h"
#include "GameFramework/Character.h"

void USMGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	MoveComp = Cast<USMCharacterMovementComponent>(Cast<ACharacter>(GetAvatarActorFromActorInfo())->GetMovementComponent());
	MoveComp->SetSprinting(true);
	const FGameplayEventData Payload;
	SendGameplayEvent(FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Character.StartSprinting")), Payload);
	AsyncTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	AsyncTask->OnRelease.AddDynamic(this, &USMGA_Sprint::OnInputRelease);
	AsyncTask->ReadyForActivation();
}

void USMGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	AsyncTask->EndTask();
	MoveComp->SetSprinting(false);
	const FGameplayEventData Payload;
	SendGameplayEvent(FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Character.StopSprinting")), Payload);
}

void USMGA_Sprint::OnInputRelease(float TimeHeld)
{
	K2_EndAbility();
}
