// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_RescuePlayer.h"

#include "Possessables/SMPlayerCharacter.h"

USMGA_RescuePlayer::USMGA_RescuePlayer()
{
	TimeToRescuePlayer = 2;
}

void USMGA_RescuePlayer::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString("Started Rescue."));
	Player = Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get()); 
	if (!Player->PlayerToRescue or Player->bIsDowned)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, FString("CANCELED REVIVE."));
		return;
	}
	GetWorld()->GetTimerManager().SetTimer(Player->RescuePlayerHandle,FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		Player->PlayerToRescue->Rescued();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString("Revived Player."));
		K2_EndAbility();
	}), TimeToRescuePlayer, false);
}

void USMGA_RescuePlayer::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	GetWorld()->GetTimerManager().ClearTimer(Player->RescuePlayerHandle);
	Player->ClientRescuePlayerHandleTimerClear();
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, FString("CANCELED RESCUE."));
	CancelAbility(Handle, ActorInfo, ActivationInfo, false);
}
