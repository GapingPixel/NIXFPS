// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AttributeSets/SMHealthAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

USMHealthAttributeSet::USMHealthAttributeSet()
	: Health(100.0f)
	, MaxHealth(100.0f)
{
}

void USMHealthAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void USMHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		if ((GetHealth() <= 0.0f) && !bOutOfHealth)
		{
			if (OnOutOfHealth.IsBound())
			{
				const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
				AActor* Instigator = EffectContext.GetOriginalInstigator();
				AActor* Causer = EffectContext.GetEffectCauser();

				OnOutOfHealth.Broadcast(Instigator, Causer, Data.EffectSpec, Data.EvaluatedData.Magnitude);
			}
		}

		// Check health again in case an event above changed it.
		bOutOfHealth = (GetHealth() <= 0.0f);
	}
}

void USMHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void USMHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMHealthAttributeSet, Health, OldValue);
}

void USMHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMHealthAttributeSet, MaxHealth, OldValue);
}

void USMHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USMHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}


//////////////////////////////////*STAMINA*//////////////////////////////////
USMStaminaAttributeSet::USMStaminaAttributeSet()
	: Stamina(100.0f)
	, MaxStamina(100.0f)
{
}

void USMStaminaAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetStaminaAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void USMStaminaAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		if ((GetStamina() <= 0.0f))
		{
			if (OnOutOfStamina.IsBound())
			{
				const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
				AActor* Instigator = EffectContext.GetOriginalInstigator();
				AActor* Causer = EffectContext.GetEffectCauser();

				OnOutOfStamina.Broadcast(Instigator, Causer, Data.EffectSpec, Data.EvaluatedData.Magnitude);
			}
		}
	}
}

void USMStaminaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void USMStaminaAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMStaminaAttributeSet, Stamina, OldValue);
}

void USMStaminaAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMStaminaAttributeSet, MaxStamina, OldValue);
}

void USMStaminaAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USMStaminaAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMStaminaAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
}


//////////////////////////////////*SANITY*//////////////////////////////////

USMSanityAttributeSet::USMSanityAttributeSet()
	: Sanity(100.0f)
	, MaxSanity(100.0f)
{
}

void USMSanityAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void USMSanityAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void USMSanityAttributeSet::OnRep_Sanity(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMSanityAttributeSet, Sanity, OldValue);
}

void USMSanityAttributeSet::OnRep_MaxSanity(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMSanityAttributeSet, MaxSanity, OldValue);
}

void USMSanityAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetSanityAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxSanity());
	} 
	else if (Attribute == GetMaxSanityAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void USMSanityAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USMSanityAttributeSet, Sanity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMSanityAttributeSet, MaxSanity, COND_None, REPNOTIFY_Always);
}
