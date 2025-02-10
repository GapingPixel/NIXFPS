// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/SMHealthDrainArea.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "AbilitySystemComponent.h" 
#include "Components/SMHealthComponent.h"
#include "Components/SMPlayerAttributesComponent.h"
#include "Possessables/SMPlayerCharacter.h"

ASMHealthDrainArea::ASMHealthDrainArea()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the trigger box component
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Bind overlap events
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASMHealthDrainArea::OnPlayerEnter);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ASMHealthDrainArea::OnPlayerExit);
}

void ASMHealthDrainArea::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}
	if (AffectedPlayer != nullptr)
	{
		return;
	}
	AffectedPlayer = Cast<ASMPlayerCharacter>(OtherActor);
	if (IsValid(AffectedPlayer) && !GetWorld()->GetTimerManager().IsTimerActive(MaxStaminaDrainTimerHandle))
	{
		UAbilitySystemComponent* AbilitySystemComponent = AffectedPlayer->GetAbilitySystemComponent();
		AffectedPlayer->HealthComp->StopStaminaRegeneration();
		GetWorld()->GetTimerManager().ClearTimer(MaxStaminaDrainTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(MaxStaminaDrainTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this,AbilitySystemComponent]()
		{
			if (AffectedPlayer)
			{
				GEngine->AddOnScreenDebugMessage(-1, .25f, FColor::Blue, TEXT("Attribute Depletion"));
				FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				const FGameplayEffectSpecHandle MaxStaminaSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(MaxStaminaDiminishGE, 1, EffectContext);
				const float CurrentMaxStamina = AffectedPlayer->HealthComp->GetMaxStamina();
				const float NewMaxStamina = FMath::Max(CurrentMaxStamina - MaxStaminaDepletionPerSecond, MaxStaminaMinValue); // Ensure it doesn't drop below 25
				MaxStaminaSpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.ValueToApply")), NewMaxStamina);
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*MaxStaminaSpecHandle.Data.Get());
				AffectedPlayer->HealthComp->OnMaxStaminaChanged.Broadcast(AffectedPlayer->HealthComp, CurrentMaxStamina, AffectedPlayer->HealthComp->GetMaxStamina(), this);

				const FGameplayEffectSpecHandle StaminaSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StaminaGE, 1, EffectContext);
				const float CurrentStamina = AffectedPlayer->HealthComp->GetStamina();
				const float NewStamina = FMath::Min(CurrentStamina, AffectedPlayer->HealthComp->GetMaxStamina());
				StaminaSpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.ValueToApply")), NewStamina);
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*StaminaSpecHandle.Data.Get());
				AffectedPlayer->HealthComp->OnStaminaChanged.Broadcast(AffectedPlayer->HealthComp, CurrentStamina, AffectedPlayer->HealthComp->GetStamina(), this);
				
				//EffectContext.AddSourceObject(this);
				const FGameplayEffectSpecHandle DamageSpec = AbilitySystemComponent->MakeOutgoingSpec(DamageGE, 1, EffectContext);
				const float OldHealth = AffectedPlayer->HealthComp->GetHealth();
				const float ActualDamageToApply = FMath::Min(DamagePerSecond, OldHealth);
				DamageSpec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.DamageToApply")), ActualDamageToApply);
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
				AffectedPlayer->HealthComp->OnHealthChanged.Broadcast(AffectedPlayer->HealthComp, OldHealth, AffectedPlayer->HealthComp->GetHealth(), this);
				
			}
		}), 1, true);
	}
}

void ASMHealthDrainArea::OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}
	if (AffectedPlayer == Cast<ASMPlayerCharacter>(OtherActor))
	{
		// Stop draining health
		GetWorld()->GetTimerManager().ClearTimer(HealthDrainTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(MaxStaminaDrainTimerHandle);
		AffectedPlayer->HealthComp->StartStaminaRegeneration();
		
		UAbilitySystemComponent* AbilitySystemComponent = AffectedPlayer->GetAbilitySystemComponent();
		const FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		const FGameplayEffectSpecHandle MaxStaminaSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(MaxStaminaResetGE, 1, EffectContext);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*MaxStaminaSpecHandle.Data.Get());
		AffectedPlayer = nullptr;
	}
}


