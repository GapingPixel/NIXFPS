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
	PrimaryComponentTick.bCanEverTick = false;
}

void USMHealthComponent::ResetHealth()
{
	if (AbilitySystemComponent && HealthSet)
	{
		FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
		EffectContextHandle.AddSourceObject(this);
		const FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(StartingAttributes, 1, EffectContextHandle);
		if (NewHandle.IsValid()){
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
		}
		OnHealthChanged.Broadcast(this, 0.0f, HealthSet->GetHealth(), nullptr); // Broadcasting the change to ensure UI and other components are updated
	}
	bIsDead = false;
}

void USMHealthComponent::ResetStamina()
{
	if (AbilitySystemComponent && StaminaSet)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		AbilitySystemComponent->BP_ApplyGameplayEffectToTarget(
			StaminaResetGE,  // The class of the effect
			AbilitySystemComponent,             // Target (self in this case)
			1.0f,                               // Level of the effect (can be adjusted as needed)
			EffectContext                       // Effect context
		);
	}
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
	StaminaSet = AbilitySystemComponent->GetSet<USMStaminaAttributeSet>();
	if (!StaminaSet)
	{
		SM_LOG(Error, TEXT("SMHealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}
	if (!GetOwner()->HasAuthority())
	{
		
	} 
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USMHealthAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USMHealthAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USMStaminaAttributeSet::GetStaminaAttribute()).AddUObject(this, &ThisClass::HandleStaminaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USMStaminaAttributeSet::GetMaxStaminaAttribute()).AddUObject(this, &ThisClass::HandleMaxStaminaChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);
	StaminaSet->OnOutOfStamina.AddUObject(this, &ThisClass::HandleOutOfStamina);
	FOnAttributeChangeData ChangeData;
	OnHealthChanged.Broadcast(this, GetHealth(), GetHealth(), GetInstigatorFromAttrChangeData(ChangeData));
	OnStaminaChanged.Broadcast(this, GetStamina(), GetStamina(), GetInstigatorFromAttrChangeData(ChangeData));
}

void USMHealthComponent::UnInitializeFromAbilitySystemComponent()
{
	HealthSet = nullptr;
	StaminaSet = nullptr;
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

float USMHealthComponent::GetStamina() const
{
	return (StaminaSet ? StaminaSet->GetStamina() : 0.0f);
}

float USMHealthComponent::GetMaxStamina() const
{
	return (StaminaSet ? StaminaSet->GetMaxStamina() : 0.0f);
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

void USMHealthComponent::HandleStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
	OnStaminaChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
	/*if (ChangeData.NewValue < 100.0f)
	{
		StartStaminaRegeneration();
	}*/
}

void USMHealthComponent::HandleMaxStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMaxStaminaChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void USMHealthComponent::HandleOutOfStamina(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent)
	{
		//Do Something
	}
#endif
}

void USMHealthComponent::StartStaminaRegeneration()
{
	GEngine->AddOnScreenDebugMessage(-1,0.5f,FColor::Yellow,"Start StaminaRegeneration");
	if (GetOwner()->HasAuthority() && !GetWorld()->GetTimerManager().IsTimerActive(StaminaRegenTimerHandle)) // Server-side authority check
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(StaminaRegenTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
				if (StaminaSet)
				{
					const float CurrentStamina = StaminaSet->GetStamina();
					const float MaxStamina = StaminaSet->GetMaxStamina();  // Max stamina is 100
					if (CurrentStamina < MaxStamina)
					{
						const FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
						AbilitySystemComponent->BP_ApplyGameplayEffectToTarget(StaminaRegenGE,AbilitySystemComponent,1.0f,EffectContext);
						const float NewStamina = StaminaSet->GetStamina();
						OnStaminaChanged.Broadcast(this, CurrentStamina, NewStamina, nullptr);
						if (NewStamina >= MaxStamina)
						{
							//StopStaminaRegeneration();
						}
					}
					else
					{
						//StopStaminaRegeneration();
					}
				}
		}), 0.1f, true);
	}
}

void USMHealthComponent::StopStaminaRegeneration()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(StaminaRegenTimerHandle) && GetOwner()->HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimerHandle);
	}
}

void USMHealthComponent::SetTeam(ETeamID newTeam)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (TeamID == newTeam)
		{
			return; // Already the same team, no need to continue.
		}

		const ETeamID OldTeam = TeamID;
		TeamID = newTeam;
		OnTeamChange.Broadcast(OldTeam, newTeam);
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
		// We want to be able to inflict damage on anything without a team.
		if (healthComponentA->TeamID == ETeamID::NoTeam || healthComponentB->TeamID == ETeamID::NoTeam)
		{
			return false; // Teamless = hostile to everyone.
		}
		
		return (healthComponentA->GetTeam() == healthComponentB->GetTeam());
	}
	
	return true; // Assume friendly if no Health Component was found.
}

AActor* USMHealthComponent::GetInstigatorFromAttrChangeData(const FOnAttributeChangeData& ChangeData)
{
	if (ChangeData.GEModData != nullptr)
	{
		const FGameplayEffectContextHandle& EffectContext = ChangeData.GEModData->EffectSpec.GetEffectContext();
		return EffectContext.GetOriginalInstigator();
	}
	return nullptr;
}

void USMHealthComponent::OnRep_TeamID(ETeamID oldTeam)
{
	// This is how OnTeamChange gets broadcasted for clients.
	OnTeamChange.Broadcast(oldTeam, TeamID);
}

void USMHealthComponent::OnRep_IsDead(bool bOldIsDead)
{
	if (bIsDead)
	{
		OnDeath.Broadcast(GetOwner());
	}
}

void USMHealthComponent::DecreaseMaxStamina()
{
	float CurrentMaxStamina = GetMaxStamina();

	// Ensure we don't decrease MaxStamina below a certain minimum threshold, like 0
	if (CurrentMaxStamina > 0)
	{
		/*float NewMaxStamina = FMath::Max(CurrentMaxStamina - 1.0f, 0.0f);
		SetMaxStamina(NewMaxStamina);

		// Clamp current stamina to not exceed the reduced MaxStamina
		ClampAttribute(GetMaxStaminaAttribute(), Stamina.GetCurrentValue());*/
	}
}

void USMHealthComponent::StartDecreasingMaxStamina()
{
	if (!bShouldDecreaseMaxStamina)
	{
		bShouldDecreaseMaxStamina = true;
		// Start a timer that decreases MaxStamina by 1 every second
		GetWorld()->GetTimerManager().SetTimer(MaxStaminaTimerHandle, this, &USMHealthComponent::DecreaseMaxStamina, 1.0f, true);
	}
}

void USMHealthComponent::StopDecreasingMaxStamina()
{
	if (bShouldDecreaseMaxStamina)
	{
		bShouldDecreaseMaxStamina = false;
		GetWorld()->GetTimerManager().ClearTimer(MaxStaminaTimerHandle);
	}
}

void USMHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USMHealthComponent, TeamID);
	DOREPLIFETIME(USMHealthComponent, bIsDead);
}
