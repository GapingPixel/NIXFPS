// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_Sprint.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Components/SMCharacterMovementComponent.h"
#include "Components/SMHealthComponent.h"
#include "Components/SMPlayerAttributesComponent.h"
#include "GameFramework/Character.h"
#include "GAS/AttributeSets/SMHealthAttributeSet.h"
#include "Possessables/SMPlayerCharacter.h"

class USMHealthAttributeSet;

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

void USMGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	const USMCharacterMovementComponent* MoveComp = Cast<USMCharacterMovementComponent>(Cast<ACharacter>(GetAvatarActorFromActorInfo())->GetMovementComponent());
	if (!MoveComp->IsJumpAllowed() || !MoveComp->IsMovingOnGround() || MoveComp->IsFalling())
	{
		K2_EndAbility();
		return;
	}
	UAbilitySystemComponent* AbilitySystem = ActorInfo->AbilitySystemComponent.Get();
	FGameplayEffectContextHandle EffectContext = AbilitySystem->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	const FGameplayEffectSpecHandle StaminaCostSpec = AbilitySystem->MakeOutgoingSpec(GEStaminaCost, 1, EffectContext);
	if (StaminaCostSpec.IsValid())
	{
		USMHealthComponent* HealthC = Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get())->HealthComp;
		const float OldStamina = HealthC->GetStamina();
		StaminaCostSpec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.StaminaCost")), StaminaCost);
		AbilitySystem->ApplyGameplayEffectSpecToSelf(*StaminaCostSpec.Data.Get());
		const float NewStamina = HealthC->GetStamina();
		HealthC->OnStaminaChanged.Broadcast(HealthC, OldStamina, NewStamina, nullptr);
	}
	Cast<ASMPlayerCharacter>(GetAvatarActorFromActorInfo())->Jump();
	const FGameplayEventData Payload;
	SendGameplayEvent(FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Character.StartJumping")), Payload);
	AsyncTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	AsyncTask->OnRelease.AddDynamic(this, &USMGA_Jump::OnInputRelease);
	AsyncTask->ReadyForActivation();
}

void USMGA_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (IsValid(AsyncTask))
	{
		AsyncTask->EndTask();
		const FGameplayEventData Payload;
		SendGameplayEvent(FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Character.StopJumping")), Payload);
	}
}

bool USMGA_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const UAbilitySystemComponent* AbilitySystem = ActorInfo->AbilitySystemComponent.Get();
	const USMStaminaAttributeSet* StaminaSet = AbilitySystem->GetSet<USMStaminaAttributeSet>();
	const float CurrentStamina = StaminaSet->GetStamina();
	if (CurrentStamina < FMath::Abs(StaminaCost)) {
		return false;
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void USMGA_Jump::OnInputRelease(float TimeHeld)
{
	Cast<ASMPlayerCharacter>(GetAvatarActorFromActorInfo())->StopJumping();
	K2_EndAbility();
}
