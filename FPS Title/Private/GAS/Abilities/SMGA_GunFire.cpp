// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_GunFire.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"


USMGA_GunFire::USMGA_GunFire()
{
	bAllowLastFire = true;
}

UAnimMontage* USMGA_GunFire::DetermineFireAnimation(EEMeshType MeshType) const
{
	if (bAllowLastFire && GetEquippable()->GetCurrentAmmo() <= 0)
	{
		switch (MeshType) {
		case EEMeshType::Arms1P:
			return GetGun()->LastFire.ArmsMontage1P;
			break;
		case EEMeshType::Equippable1P:
			return GetGun()->LastFire.EquippableMontage1P;
			break;
		case EEMeshType::Arms3P:
			return GetGun()->LastFire.FullBodyMontage3P;
			break;
		case EEMeshType::Equippable3P:
			return GetGun()->LastFire.EquippableMontage3P;
			break;
		}
	} 
	return Super::DetermineFireAnimation(MeshType);
}



void USMGA_GunFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	StartSprintingTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Character.StartSprinting")));
	StartSprintingTask->EventReceived.AddDynamic(this, &USMGA_GunFire::OnSprintReceived);
	StartSprintingTask->ReadyForActivation();
}

void USMGA_GunFire::OnSprintReceived(FGameplayEventData Payload)
{
	bShouldStopFire = true;
}

void USMGA_GunFire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (IsValid(StartSprintingTask))
	{
		StartSprintingTask->EndTask();
	}
}
