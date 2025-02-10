// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_RechamberGun.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GAS/Abilities/AbilityTasks/SMPlayMontageForMesh.h"
#include "Possessables/SMPlayerCharacter.h"
#include "NIX/GlobalFunctions.h"

void USMGA_RechamberGun::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Sprinting"))))
	{
		WaitForSprintTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Character.StopSprinting")), nullptr, true);
		WaitForSprintTask->EventReceived.AddDynamic(this, &ThisClass::OnSprintReceived);
		WaitForSprintTask->ReadyForActivation();
	} else
	{
		DoMontages();
	}
}

void USMGA_RechamberGun::OnSprintReceived(FGameplayEventData Payload)
{
	if (CanContinueWithAbility(GetAvatarActorFromActorInfo()))
	{
		DoMontages();
	} else
	{
		K2_EndAbility();
	}
}

FORCEINLINE void USMGA_RechamberGun::DoMontages()
{
	Gun = GlobalFunctions::GetCurrentEquippableIfRechamberGun(GetAvatarActorFromActorInfo(),GetWorld());
	Character = Cast<ASMPlayerCharacter>(GetAvatarActorFromActorInfo());
	const FGameplayTagContainer EventTags;
	MontageTask = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),Gun->ReChamberAnimations.ArmsMontage1P,EventTags,
		Character->FirstPersonHandsMesh,1.0,FName(NAME_None), false,
		1.0,0.0,false);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnCompleted);

	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnInterrupted);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitForRechamberTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("GameplayEvent.ReChamber")), nullptr, true);
	WaitForRechamberTask->EventReceived.AddDynamic(this, &ThisClass::OnRechamberReceived);
	WaitForRechamberTask->ReadyForActivation();
}

void USMGA_RechamberGun::OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	K2_EndAbilityLocally();
}

void USMGA_RechamberGun::OnInterrupted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	K2_EndAbility();
}

void USMGA_RechamberGun::OnRechamberReceived(FGameplayEventData Payload)
{
	Gun->SetNeedsRechambering(false);
	FTimerHandle ToEnd;
	GetWorld()->GetTimerManager().SetTimer(ToEnd, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		K2_EndAbility();
	}), TimeAfterRechamberToEndAbility, false);
}

void USMGA_RechamberGun::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (bWasCancelled)
	{
		if (IsValid(Gun))
		{
			Gun->SetNeedsRechambering(true);
		}
	}
	if (IsValid(MontageTask))
	{
		MontageTask->EndTask();
	}
}

bool USMGA_RechamberGun::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                            const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const AActor* const AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (AvatarActor == nullptr || !ShouldActivateAbility(AvatarActor->GetLocalRole()))
	{
		return false;
	}
	//make into a reference for simplicity
	static FGameplayTagContainer DummyContainer;
	DummyContainer.Reset();
	// make sure the ability system component is valid, if not bail out.
	const UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!AbilitySystemComponent)
	{
		return false;
	}
	const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!Spec)
	{
		return false;
	}
	if (AbilitySystemComponent->GetUserAbilityActivationInhibited())
	{
		
		return false;
	}
	const UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	if (!AbilitySystemGlobals.ShouldIgnoreCooldowns() && !CheckCooldown(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	if (!AbilitySystemGlobals.ShouldIgnoreCosts() && !CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}
	if (!DoesAbilitySatisfyTagRequirements(*AbilitySystemComponent, SourceTags, TargetTags, OptionalRelevantTags))
	{	// If the ability's tags are blocked, or if it has a "Blocking" tag or is missing a "Required" tag, then it can't activate.
		return false;
	}
	// Check if this ability's input binding is currently blocked
	if (AbilitySystemComponent->IsAbilityInputBlocked(Spec->InputID))
	{
		return false;
	}
	return CanContinueWithAbility(ActorInfo->AvatarActor.Get());
}

bool USMGA_RechamberGun::CanContinueWithAbility(const AActor* AvatarActor) const
{
	const ASMManualRechamberGunBase* RechamberGun = GlobalFunctions::GetCurrentEquippableIfRechamberGun(AvatarActor, GetWorld());
	if (IsValid(RechamberGun))
	{
		return RechamberGun->GetCurrentAmmo() > 0.0 && RechamberGun->NeedsRechambering();
	} 
	return false; 
}
