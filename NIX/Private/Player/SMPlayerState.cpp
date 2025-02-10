// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SMPlayerState.h"

#include "GAS/SMAbilitySystemComponent.h"
#include "GAS/AttributeSets/SMCharacterAttributeSet.h"
#include "GAS/AttributeSets/SMCombatAttributeSet.h"
#include "GAS/AttributeSets/SMHealthAttributeSet.h"
#include "GAS/AttributeSets/SMSurvivorAttributeSet.h"
#include "Net/UnrealNetwork.h"

ASMPlayerState::ASMPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<USMAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	CharacterAttributeSet = CreateDefaultSubobject<USMCharacterAttributeSet>(TEXT("BaseAttributeSet"));

	CreateDefaultSubobject<USMHealthAttributeSet>(TEXT("HealthSet"));
	CreateDefaultSubobject<USMStaminaAttributeSet>(TEXT("StaminaSet"));
	CreateDefaultSubobject<USMSurvivorAttributeSet>(TEXT("SurvivorSet"));
	CreateDefaultSubobject<USMCombatAttributeSet>(TEXT("CombatSet"));
	CreateDefaultSubobject<USMSanityAttributeSet>(TEXT("SanitySet"));

	SetReplicates(true);
	NetUpdateFrequency = 100.f; // @TODO: Check if this is a suitable number to have.
}

void ASMPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
	
}


UAbilitySystemComponent* ASMPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


void ASMPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASMPlayerState, XPCount);
	DOREPLIFETIME(ASMPlayerState, KillCount);
	DOREPLIFETIME(ASMPlayerState, DeathCount);
	DOREPLIFETIME(ASMPlayerState, RescueCount);
	DOREPLIFETIME(ASMPlayerState, FireAssist);
	DOREPLIFETIME(ASMPlayerState, FriendlyFire);
	DOREPLIFETIME(ASMPlayerState, DamageDealt);
}