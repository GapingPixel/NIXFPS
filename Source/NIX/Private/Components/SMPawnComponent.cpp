// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SMPawnComponent.h"

#include "AbilitySystemInterface.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "NIX/NIX.h"

USMPawnComponent::USMPawnComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void USMPawnComponent::OnRegister()
{
	Super::OnRegister();

	APawn* pawn = GetPawn<APawn>();
	if (pawn && pawn->Implements<UAbilitySystemComponent>())
	{
		IAbilitySystemInterface* interface = Cast<IAbilitySystemInterface>(pawn);
		if (interface)
		{
			USMAbilitySystemComponent* asc = Cast<USMAbilitySystemComponent>(interface->GetAbilitySystemComponent());
			if (!asc)
			{
				UE_LOG(LogNIX, Error, TEXT("Ability System Component could not be found in OnRegister in %s"), *GetNameSafe(this))
				return;
			}

			//InitializeAbilitySystemComponent(asc, GetPlayerState<ASMPlayerState>());
		}
		else
		{
			UE_LOG(LogNIX, Error, TEXT("Ability System Interface could not be found in OnRegister in %s"), *GetNameSafe(this))
		}
	}
}

void USMPawnComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent)
	{
		Delegate.Execute();
	}
}

void USMPawnComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

void USMPawnComponent::InitializeAbilitySystemComponent(USMAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC)
	check(InOwnerActor)

	if (AbilitySystemComponent)
	{
		return;
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	const AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogNIX, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);
	AbilitySystemComponent->RefreshAbilityActorInfo();

	OnAbilitySystemInitialized.Broadcast();
}

void USMPawnComponent::UninitializeAbilitySystemComponent()
{
	if (!AbilitySystemComponent)
	{
		return;
	}
	
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		AbilitySystemComponent->CancelAbilities(nullptr, nullptr);
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystemComponent = nullptr;
}
