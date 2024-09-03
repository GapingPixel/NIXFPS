// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDs/SMWPlayerHUD.h"

#include "Components/SMHealthComponent.h"
#include "Components/TextBlock.h"
#include "Possessables/SMPlayerCharacter.h"


void USMWPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();
	Character = Cast<ASMPlayerCharacter>(GetOwningPlayerPawn());
	Character->HealthComp->OnHealthChanged.AddDynamic(this, &USMWPlayerHUD::OnHealthChanged);
	Character->InventoryComponent->OnCurrentEquippableChanged.AddDynamic(this, &USMWPlayerHUD::OnCurrentEquippableChanged);
	
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

