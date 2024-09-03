// Fill out your copyright notice in the Description page of Project Settings.
#include "GAS/Abilities/SMGA_Reload.h"
#include "GameplayTask.h"
#include "Components/SMEquippableInventoryComponent.h"
#include "GAS/Abilities/AbilityTasks/SMPlayMontageForMesh.h"
//#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h" todo This Include is Evil
#include "Tasks/GameplayTask_WaitDelay.h"
#include "Possessables/SMBaseCharacter.h"
#include "NIX/GlobalFunctions.h"
#include "Animation/AnimMontage.h"


USMGA_Reload::USMGA_Reload()
{
	ReloadTime = 1.57;
}

bool USMGA_Reload::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                      const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const auto EquippableLocal = GlobalFunctions::GetCurrentEquippable(ActorInfo->AvatarActor.Get());
	if (IsValid(EquippableLocal))
	{
		return EquippableLocal->GetCurrentAmmo() < EquippableLocal->MaxCurrentAmmo;
	}
	return  false;
}

void USMGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SetupCachables();
	DoReload();
}

FORCEINLINE void USMGA_Reload::DoReload()
{
	ReloadTask = UGameplayTask_WaitDelay::TaskWaitDelay(*this, DetermineReloadTime());
	ReloadTask->OnFinish.AddDynamic(this, &USMGA_Reload::TaskWaitDelayFinish);
	ReloadTask->ReadyForActivation();
	const FGameplayTagContainer EventTag;
	//AsyncTaskPlayMontageFullBodyMontage3p = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,FName("None"),Equippable->ReloadAnimations.FullBodyMontage3P, 1);
	AsyncTaskPlayMontageFullBodyMontage3p = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),Equippable->ReloadAnimations.FullBodyMontage3P, EventTag);
	AsyncTaskPlayMontageFullBodyMontage3p->ReadyForActivation();
	if (Character->IsLocallyControlled())
	{
		AsyncTaskPlayMontageDetermineReloadMontangeArms1p = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),DetermineReloadMontageArms1P(), EventTag, Character->GetMeshOfType(EMeshType::FirstPersonHands), 1,
																														FName(NAME_None), false, 1, 0, false);
		AsyncTaskPlayMontageDetermineReloadMontangeArms1p->OnCompleted.AddDynamic(this, &USMGA_Reload::PlayMontageCompleted);
		AsyncTaskPlayMontageDetermineReloadMontangeArms1p->OnInterrupted.AddDynamic(this, &USMGA_Reload::PlayMontageInterrupted);
		AsyncTaskPlayMontageDetermineReloadMontangeArms1p->OnCancelled.AddDynamic(this, &USMGA_Reload::PlayMontageInterrupted);
		AsyncTaskPlayMontageDetermineReloadMontangeArms1p->ReadyForActivation();

		AsyncTaskPlayMontageDetermineReloadMontageEquippable1p  = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),DetermineReloadMontageEquippable1P(), EventTag, Character->InventoryComponent->GetCurrentEquippable()->GetEquippableMesh1P(), 1,
			FName(NAME_None), false, 1, 0, false);
		AsyncTaskPlayMontageDetermineReloadMontageEquippable1p->ReadyForActivation();
	}
}

void USMGA_Reload::TaskWaitDelayFinish()
{
	Equippable->SetAmmo(Equippable->MaxCurrentAmmo);
}

void USMGA_Reload::PlayMontageInterrupted(FGameplayTag Tag, FGameplayEventData Data)
{
	K2_CancelAbility();
}

void USMGA_Reload::PlayMontageCompleted(FGameplayTag Tag, FGameplayEventData Data)
{
	K2_EndAbility();
}

void USMGA_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                              const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsValid(ReloadTask))
	{
		ReloadTask->EndTask();
	}
	/*if (AsyncTaskPlayMontageFullBodyMontage3p->IsActive())
	{
		AsyncTaskPlayMontageFullBodyMontage3p->EndTask();
	}
	if (AsyncTaskPlayMontageDetermineReloadMontangeArms1p->IsActive())
	{
		AsyncTaskPlayMontageDetermineReloadMontangeArms1p->EndTask();
	}
	if (AsyncTaskPlayMontageDetermineReloadMontageEquippable1p->IsActive())
	{
		AsyncTaskPlayMontageDetermineReloadMontageEquippable1p->EndTask();
	}*/
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FORCEINLINE UAnimMontage* USMGA_Reload::DetermineReloadMontageArms1P() const
{
	return Equippable->ReloadAnimations.ArmsMontage1P;
}

FORCEINLINE UAnimMontage* USMGA_Reload::DetermineReloadMontageEquippable1P() const
{
	return Equippable->ReloadAnimations.EquippableMontage1P;
}

FORCEINLINE UAnimMontage* USMGA_Reload::DetermineReloadMontageArms3P() const
{
	return Equippable->ReloadAnimations.FullBodyMontage3P;
}

FORCEINLINE UAnimMontage* USMGA_Reload::DetermineReloadMontageEquippable3P() const
{
	return Equippable->ReloadAnimations.EquippableMontage3P;
}

FORCEINLINE float USMGA_Reload::DetermineReloadTime() const
{
	return ReloadTime;
}

FORCEINLINE ASMGunBase* USMGA_Reload::GetGunEquippable() const
{
	if (IsValid(Gun))
	{
		return Gun;
	}
	return Cast<ASMGunBase>(Cast<ASMBaseCharacter>(GetAvatarActorFromActorInfo())->InventoryComponent->GetCurrentEquippable()); 
}

FORCEINLINE void USMGA_Reload::SetupCachables()
{
	Character = Cast<ASMBaseCharacter>(GetAvatarActorFromActorInfo());
	Equippable = Character->InventoryComponent->GetCurrentEquippable();
}


