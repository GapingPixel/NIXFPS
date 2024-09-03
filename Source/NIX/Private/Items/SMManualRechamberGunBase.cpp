// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SMManualRechamberGunBase.h"

#include "GAS/SMAbilitySystemComponent.h"
#include "Interfaces/SMFirstPersonInterface.h"
#include "Net/UnrealNetwork.h"
#include "NIX/NIX.h"

ASMManualRechamberGunBase::ASMManualRechamberGunBase()
{
	
}

void ASMManualRechamberGunBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASMManualRechamberGunBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	bNeedsRechambering = bStartUnChambered;

	OnEquippableIsIdle.AddDynamic(this, &ASMManualRechamberGunBase::CheckForReChamber);//Red ERROR
}

void ASMManualRechamberGunBase::CheckForReChamber()
{
	if (ReChamberAbilityTag.IsValid() == false)
	{
		SM_LOG(Warning, TEXT("ReChamberAbilityTag is not valid in CheckForReChamber."))
		return;
	}

	if (bNeedsRechambering == false)
	{
		return;
	}

	const ISMFirstPersonInterface* Interface = GetOwnerFirstPersonInterface();
	check(Interface)
	
	USMAbilitySystemComponent* ASC = Interface->GetSMAbilitySystemComponent();
	check(ASC)

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(ReChamberAbilityTag);
	
	ASC->TryActivateAbilitiesByTag(TagContainer);
}

void ASMManualRechamberGunBase::OnExplicitlySpawnedIn()
{
	Super::OnExplicitlySpawnedIn();

	bNeedsRechambering = bSpawnUnChambered;
}

void ASMManualRechamberGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASMManualRechamberGunBase, bNeedsRechambering, COND_SkipOwner);
}
