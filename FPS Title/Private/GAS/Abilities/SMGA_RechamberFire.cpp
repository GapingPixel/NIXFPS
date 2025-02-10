// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_RechamberFire.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Possessables/SMPlayerCharacter.h"

bool USMGA_RechamberFire::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                             const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// Don't set the actor info, CanActivate is called on the CDO

	// A valid AvatarActor is required. Simulated proxy check means only authority or autonomous proxies should be executing abilities.
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
	return !GlobalFunctions::GetCurrentEquippableIfRechamberGun(ActorInfo->AvatarActor.Get(), GetWorld())->NeedsRechambering() || Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get())->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.IsReloading")));
}

void USMGA_RechamberFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.IsReloading"))))
	{
		const FGameplayEventData Payload;
		SendGameplayEvent(FGameplayTag::RequestGameplayTag(FName("GameplayEvent.CancelReload")), Payload);
		K2_EndAbility();
		//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		//GetEquippable()->AddOrRemoveFromInteractionCount(false); todo Not sure to add this
	} else
	{
		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}
}

void USMGA_RechamberFire::OnRangedWeaponTargetDataReadyCPP(const FGameplayAbilityTargetDataHandle& TargetData)
{
	Super::OnRangedWeaponTargetDataReadyCPP(TargetData);
	ASMManualRechamberGunBase* GunLocal = Cast<ASMManualRechamberGunBase>(Cast<ASMPlayerCharacter>(GetAvatarActorFromActorInfo())->GetInventoryComponent()->CurrentEquippable); 
	GunLocal->SetNeedsRechambering(true);
}
