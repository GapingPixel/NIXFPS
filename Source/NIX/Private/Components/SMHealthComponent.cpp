// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SMHealthComponent.h"

#include "GameplayEffectExtension.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "GAS/AttributeSets/SMHealthAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "NIX/NIX.h"

// Sets default values for this component's properties
USMHealthComponent::USMHealthComponent()
{
	SetIsReplicatedByDefault(true);
}

void USMHealthComponent::InitializeWithAbilitySystemComponent(USMAbilitySystemComponent* InASC)
{
	const AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		SM_LOG(Error, TEXT("SMHealthComponent: Health component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		SM_LOG(Error, TEXT("SMHealthComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	HealthSet = AbilitySystemComponent->GetSet<USMHealthAttributeSet>();
	if (!HealthSet)
	{
		SM_LOG(Error, TEXT("SMHealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USMHealthAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USMHealthAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::HandleMaxHealthChanged);

	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);
}

void USMHealthComponent::UnInitializeFromAbilitySystemComponent()
{
	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

float USMHealthComponent::GetHealth() const
{
	return (HealthSet ? HealthSet->GetHealth() : 0.0f);
}

float USMHealthComponent::GetMaxHealth() const
{
	return (HealthSet ? HealthSet->GetMaxHealth() : 0.0f);
}

static AActor* GetInstigatorFromAttrChangeData(const FOnAttributeChangeData& ChangeData)
{
	if (ChangeData.GEModData != nullptr)
	{
		const FGameplayEffectContextHandle& EffectContext = ChangeData.GEModData->EffectSpec.GetEffectContext();
		return EffectContext.GetOriginalInstigator();
	}

	return nullptr;
}

void USMHealthComponent::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	OnHealthChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void USMHealthComponent::HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMaxHealthChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void USMHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent)
	{
		if (!bIsDead)
		{
			bIsDead = true;
			OnRep_IsDead(false);
		}
	}
#endif
}

void USMHealthComponent::SetTeam(ETeamID newTeam)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (TeamID == newTeam)
		{
			return; // Already the same team, no need to continue.
		}

		const ETeamID oldTeam = TeamID;
		TeamID = newTeam;
		OnTeamChange.Broadcast(oldTeam, newTeam);
	}
	else
	{
		UE_LOG(LogNIX, Warning, TEXT("SetTeam called on Client."))
	}
}

ETeamID USMHealthComponent::GetTeam()
{
	return TeamID;
}

ETeamID USMHealthComponent::GetTeamFromActor(const AActor* actor)
{
	if (actor)
	{
		if (const USMHealthComponent* healthComponent = actor->FindComponentByClass<USMHealthComponent>())
		{
			return healthComponent->TeamID;
		}
	
		UE_LOG(LogNIX, Warning, TEXT("Actor found in GetTeam, but no HealthComponent was found."))
	
		return ETeamID::NoTeam;
	}

	UE_LOG(LogNIX, Warning, TEXT("Actor in function GetTeam is nullptr. Returning NoTeam."))
	
	return ETeamID::NoTeam;
}

bool USMHealthComponent::IsFriendly(const AActor* actorA, const AActor* actorB) // @TODO: possible we can do this in a cheaper manner? it's fine for now though. (maybe interface?)
{
	USMHealthComponent* healthComponentA = actorA->FindComponentByClass<USMHealthComponent>();
	USMHealthComponent* healthComponentB = actorB->FindComponentByClass<USMHealthComponent>();
	if (healthComponentA && healthComponentB)
	{
		if (healthComponentA->TeamID == ETeamID::NoTeam || healthComponentB->TeamID == ETeamID::NoTeam)
		{
			return false; // Teamless = hostile to everyone.
		}
		
		return (healthComponentA->GetTeam() == healthComponentB->GetTeam());
	}
	
	return true; // Assume friendly if no Health Component was found.
}

void USMHealthComponent::OnRep_TeamID(ETeamID oldTeam)
{
	OnTeamChange.Broadcast(oldTeam, TeamID);
}

void USMHealthComponent::OnRep_IsDead(bool bOldIsDead)
{
	if (bIsDead)
	{
		OnDeath.Broadcast(GetOwner());
	}
}

void USMHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USMHealthComponent, TeamID);
	DOREPLIFETIME(USMHealthComponent, bIsDead);
}
