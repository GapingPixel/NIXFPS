// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "SMPlayerState.generated.h"

class USMCharacterAttributeSet;
class USMCombatAttributeSet;
class UGameplayEffect;
class USMAbilitySystemComponent;
class UGameplayAbility;

/**
 * 
 */
UCLASS()
class NIX_API ASMPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ASMPlayerState();

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	
#pragma region GAS

protected:
	
	// The ASC on the PlayerState.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = GAS)
	USMAbilitySystemComponent* AbilitySystemComponent;
	

	void SetupAttributes();

	UPROPERTY()
	USMCharacterAttributeSet* CharacterAttributeSet;

	UPROPERTY()
	USMCombatAttributeSet* CombatAttributeSet;
	
public:

	USMCharacterAttributeSet* GetCharacterAttributeSet() const { return CharacterAttributeSet; }
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
#pragma endregion GAS
};
