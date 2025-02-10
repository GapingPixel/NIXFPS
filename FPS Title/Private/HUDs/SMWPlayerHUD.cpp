// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDs/SMWPlayerHUD.h"

#include "Components/SMPlayerAttributesComponent.h"
#include "Components/TextBlock.h"
#include "Possessables/SMPlayerCharacter.h"


void USMWPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();
	if ((Character = Cast<ASMPlayerCharacter>(GetOwningPlayerPawn())))
	{
		Character->HealthComp->OnHealthChanged.AddDynamic(this, &USMWPlayerHUD::OnHealthChanged);
		Character->HealthComp->OnStaminaChanged.AddDynamic(this, &USMWPlayerHUD::OnStaminaChanged);
		Character->HealthComp->OnSanityChanged.AddDynamic(this, &USMWPlayerHUD::OnSanityChanged);
		Character->InventoryComponent->OnCurrentEquippableChanged.AddDynamic(this, &USMWPlayerHUD::OnCurrentEquippableChanged);
		Character->OnDownedHealthChanged.AddDynamic(this, &USMWPlayerHUD::OnDownedHealthChanged);
		TextHealth->SetText(FText::AsNumber(Character->HealthComp->GetHealth()));
		TextStamina->SetText(FText::AsNumber(Character->HealthComp->GetStamina()));
		TextSanity->SetText(FText::AsNumber(Character->HealthComp->GetSanity()));
	}
}

void USMWPlayerHUD::OnCurrentEquippableChanged(ASMEquippableBase* OldEquippable)
{
	if (IsValid(Character->InventoryComponent->CurrentEquippable))
	{
		ASMEquippableBase* Wep = Character->InventoryComponent->CurrentEquippable;
		Wep->OnCurrentAmmoChanged.AddDynamic(this, &USMWPlayerHUD::OnCurrentAmmoChanged);
		TextAmmo->SetText(FText::AsNumber(Wep->CurrentAmmo)); 
	}
}

void USMWPlayerHUD::OnCurrentAmmoChanged(float OldAmmo, const float NewAmmo)
{
	TextAmmo->SetText(FText::AsNumber(NewAmmo)); 
}

void USMWPlayerHUD::OnHealthChanged(USMHealthComponent* HealthComponent, float OldHealth, const float NewHealth,
                                    AActor* Instigator)
{
	TextHealth->SetText(FText::AsNumber(NewHealth));
}

void USMWPlayerHUD::OnStaminaChanged(USMHealthComponent* StaminaComponent, float OldStamina, float NewStamina,
	AActor* Instigator)
{
	TextStamina->SetText(FText::AsNumber(NewStamina));
}

void USMWPlayerHUD::OnSanityChanged(USMPlayerAttributesComponent* SanityComponent, float OldSanity, float NewSanity,
	AActor* Instigator)
{
	TextSanity->SetText(FText::AsNumber(NewSanity));
}

void USMWPlayerHUD::OnDownedHealthChanged(float OldHealth, float NewHealth)
{
	TextHealth->SetText(FText::AsNumber(NewHealth));
}

void USMWPlayerHUD::SetUpReferences_Implementation(ASMPlayerCharacter* NewPlayer)
{
	Character = NewPlayer;
	Character->HealthComp->OnHealthChanged.RemoveAll(this);
	Character->HealthComp->OnStaminaChanged.RemoveAll(this);
	Character->HealthComp->OnSanityChanged.RemoveAll(this);
	Character->InventoryComponent->OnCurrentEquippableChanged.RemoveAll(this);
	Character->HealthComp->OnHealthChanged.AddDynamic(this, &USMWPlayerHUD::OnHealthChanged);
	Character->HealthComp->OnStaminaChanged.AddDynamic(this, &USMWPlayerHUD::OnStaminaChanged);
	Character->HealthComp->OnSanityChanged.AddDynamic(this, &USMWPlayerHUD::OnSanityChanged);
	Character->InventoryComponent->OnCurrentEquippableChanged.AddDynamic(this, &USMWPlayerHUD::OnCurrentEquippableChanged);
}


