// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMWPlayerHUD.generated.h"

class USMHealthComponent;
class UTextBlock;
class ASMEquippableBase;
class ASMPlayerCharacter;
/**
 * 
 */
UCLASS(meta=(DisableNativeTick))
class NIX_API USMWPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextHealth;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextAmmo;

	UPROPERTY()
	TObjectPtr<ASMPlayerCharacter> Character;
	
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnCurrentEquippableChanged(ASMEquippableBase* OldEquippable);
	UFUNCTION()
	void  OnCurrentAmmoChanged(float OldAmmo, float NewAmmo);
	UFUNCTION()
	void OnHealthChanged(USMHealthComponent* HealthComponent, float OldHealth, float NewHealth, AActor* Instigator);


	
};
