// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_SanityPill.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Components/SMPlayerAttributesComponent.h"
#include "Possessables/SMPlayerCharacter.h"

bool USMGA_SanityPill::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const USMPlayerAttributesComponent* HealthC = Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get())->HealthComp;
	if (HealthC->GetSanity() >= HealthC->GetMaxSanity())
	{
		GEngine->AddOnScreenDebugMessage(-1, TimeToActivate, FColor::White, FString("Canceled Pill"));
		return false;
	} 
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void USMGA_SanityPill::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	USMPlayerAttributesComponent* HealthC = Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get())->HealthComp;
	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();//Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get())->GetAbilitySystemComponent();
	AsyncTask_WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
	AsyncTask_WaitInputRelease->OnRelease.AddDynamic(this, &USMGA_SanityPill::OnKeyReleased);
	AsyncTask_WaitInputRelease->ReadyForActivation();
	GEngine->AddOnScreenDebugMessage(-1, TimeToActivate, FColor::White, FString("UsingPill"));
	GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this, HealthC, AbilitySystem]()
	{
		const FGameplayEffectContextHandle EffectContext = AbilitySystem->MakeEffectContext();
		const float OldSanity = HealthC->GetSanity();
		const float MaxSanity = HealthC->GetMaxSanity();
		const FGameplayEffectSpecHandle DamageSpecHandle = AbilitySystem->MakeOutgoingSpec(SanityGE, 1, EffectContext);
		const ASMPlayerCharacter* Player = Cast<ASMPlayerCharacter>(HealthC->GetOwner());
		const float ActualSanityToRestore = FMath::Min(SanityToRestore, MaxSanity - OldSanity);
		DamageSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.ValueToApply")), ActualSanityToRestore);
		AbilitySystem->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
		AsyncTask_WaitInputRelease->EndTask();
		K2_EndAbility();
		GEngine->AddOnScreenDebugMessage(-1, TimeToActivate, FColor::White, FString("PilluSED"));
		HealthC->OnSanityChanged.Broadcast(HealthC, OldSanity, HealthC->GetSanity(), nullptr);
		Player->InventoryComponent->UnEquipAndDestroy();
	}), TimeToActivate, false);
}

void USMGA_SanityPill::OnKeyReleased(float TimeHeld)
{
	GetWorld()->GetTimerManager().ClearTimer(Timer);
	K2_CancelAbility();
}
