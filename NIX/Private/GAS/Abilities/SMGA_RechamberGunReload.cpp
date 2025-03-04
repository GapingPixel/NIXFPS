// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_RechamberGunReload.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/Abilities/AbilityTasks/SMPlayMontageForMesh.h"
#include "Items/SMManualRechamberGunBase.h"
#include "Possessables/SMBaseCharacter.h"
#include "NIX/GlobalFunctions.h"

USMGA_RechamberGunReload::USMGA_RechamberGunReload()
{
	ExitReloadTime = 0.5;
	ReloadAnimationPlayRate = 1.0;
	TimeToDisplay = 1;
}

void USMGA_RechamberGunReload::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
	USMEquippableAbilityBase::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	bShouldCancelReload = false;
	SetupCachables();
	CancelReloadTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("GameplayEvent.CancelReload")));
	CancelReloadTask->EventReceived.AddDynamic(this, &USMGA_RechamberGunReload::OnCancelReload);
	CancelReloadTask->ReadyForActivation();
	EnterReload();
}

void USMGA_RechamberGunReload::OnCancelReload(FGameplayEventData Payload)
{
	//GEngine->AddOnScreenDebugMessage(-1, timeToDisplay, FColor::Yellow, TEXT("OnCancelReload"));	
	bShouldCancelReload = true;
}


FORCEINLINE void USMGA_RechamberGunReload::EnterReload()
{
	const FGameplayTagContainer EventTags;
	AsyncTaskPlayMontageForMeshEnterReload = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),RechamberGun->ReloadEnterAnimations.ArmsMontage1P,EventTags,
		Character->GetMeshOfType(EMeshType::FirstPersonHands),1.0,FName(NAME_None),
		true,1.0,0.0, false);
	AsyncTaskPlayMontageForMeshEnterReload->OnBlendOut.AddDynamic(this, &ThisClass::OnBlendOut);
	AsyncTaskPlayMontageForMeshEnterReload->OnInterrupted.AddDynamic(this, &ThisClass::InterruptedReload);
	AsyncTaskPlayMontageForMeshEnterReload->OnCancelled.AddDynamic(this, &ThisClass::InterruptedReload);
	AsyncTaskPlayMontageForMeshEnterReload->ReadyForActivation();
}



void USMGA_RechamberGunReload::InterruptedReload(FGameplayTag EventTag, FGameplayEventData EventData)
{
	K2_EndAbility();
}

void USMGA_RechamberGunReload::OnBlendOut(FGameplayTag EventTag, FGameplayEventData EventData)
{
	//GEngine->AddOnScreenDebugMessage(-1, timeToDisplay, FColor::Yellow, TEXT("OnBlendOut"));	
	SetAmountToReload();
	WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("GameplayEvent.ReloadSingleBullet")));
	WaitEventTask->EventReceived.AddDynamic(this, &USMGA_RechamberGunReload::OnWaitNotify);
	WaitEventTask->ReadyForActivation();
	OnBulletReload(EventTag, EventData);
}

void USMGA_RechamberGunReload::OnWaitNotify(FGameplayEventData Payload)
{
	//GEngine->AddOnScreenDebugMessage(-1, timeToDisplay, FColor::Yellow, TEXT("OnWaitNotify"));	
	RechamberGun->AddAmmo(1.0, true);
}

void USMGA_RechamberGunReload::OnBulletReload(FGameplayTag EventTag, FGameplayEventData EventData)
{
	//GEngine->AddOnScreenDebugMessage(-1, timeToDisplay, FColor::Yellow, TEXT("OnBulletReload"));	
	if ( (RechamberGun->GetCurrentAmmo() >= RechamberGun->MaxCurrentAmmo) || bShouldCancelReload)
	{
		EndReload();
	} else
	{
		ReloadLoop();
	}
}

FORCEINLINE void USMGA_RechamberGunReload::ReloadLoop()
{
	//GEngine->AddOnScreenDebugMessage(-1, timeToDisplay, FColor::Yellow, TEXT("ReloadLoop"));	
	const FGameplayTagContainer EventTags;
	AsyncTaskPlayMontageForMeshReloadLoopHands = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),RechamberGun->ReloadAnimations.ArmsMontage1P,EventTags,
		Character->GetMeshOfType(EMeshType::FirstPersonHands),ReloadAnimationPlayRate,FName(NAME_None),
		true,1.0,0.0, false);
	AsyncTaskPlayMontageForMeshReloadLoopHands->ReadyForActivation();

	AsyncTaskPlayMontageForMeshReloadLoopWeapon = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),RechamberGun->ReloadAnimations.EquippableMontage1P,EventTags,
		RechamberGun->GetEquippableMesh1P(),ReloadAnimationPlayRate,FName(NAME_None),
		true,1.0,0.0, false);
	AsyncTaskPlayMontageForMeshReloadLoopWeapon->OnBlendOut.AddDynamic(this, &ThisClass::OnBulletReload);
	AsyncTaskPlayMontageForMeshReloadLoopWeapon->ReadyForActivation();
}

FORCEINLINE void USMGA_RechamberGunReload::EndReload()
{
	//GEngine->AddOnScreenDebugMessage(-1, timeToDisplay, FColor::Yellow, TEXT("EndReload"));	
	const FGameplayTagContainer EventTags;
	AsyncTaskPlayMontageForMeshEndReload = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),RechamberGun->ReloadExitAnimations.ArmsMontage1P,EventTags,Character->GetMeshOfType(EMeshType::FirstPersonHands),1,FName(NAME_None),
		false,1.0,0.0, false);
	AsyncTaskPlayMontageForMeshEndReload->ReadyForActivation();
	
	GetWorld()->GetTimerManager().SetTimer(ExitAnimTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		K2_EndAbility();
	}), ExitReloadTime, false);
}


FORCEINLINE void USMGA_RechamberGunReload::SetAmountToReload()
{
	//GEngine->AddOnScreenDebugMessage(-1, timeToDisplay, FColor::Yellow, TEXT("SetAmountToReload"));	
	AmountToReload = FMath::TruncToInt(RechamberGun->MaxCurrentAmmo - RechamberGun->GetCurrentAmmo());
}

float USMGA_RechamberGunReload::DetermineReloadTime() const
{
	//GEngine->AddOnScreenDebugMessage(-1, timeToDisplay, FColor::Yellow, TEXT("DetermineReloadTime"));
	return RechamberGun->ReloadAnimations.ArmsMontage1P->GetPlayLength();
}

void USMGA_RechamberGunReload::SetupCachables()
{
	Super::SetupCachables();
	RechamberGun = GlobalFunctions::GetCurrentEquippableIfRechamberGun(GetAvatarActorFromActorInfo(), GetWorld());
}

void USMGA_RechamberGunReload::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (ExitAnimTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ExitAnimTimer);
	}
	
	if (IsValid(WaitEventTask))
	{
		WaitEventTask->EndTask();
	}
	if (IsValid(CancelReloadTask))
	{
		CancelReloadTask->EndTask();
	}
}

