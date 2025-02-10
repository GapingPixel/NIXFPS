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

	virtual void PostInitializeComponents() override;

	UPROPERTY(Replicated)
	int8 XPCount;//

	UPROPERTY(Replicated)
	int8 KillCount; 

	UPROPERTY(Replicated)
	int8 DeathCount;//

	UPROPERTY(Replicated)
	int8 RescueCount;//

	UPROPERTY(Replicated)
	int8 FireAssist;

	UPROPERTY(Replicated)
	float FriendlyFire;//

	UPROPERTY(Replicated)
	float DamageDealt;//

	float XPPerKill = 20;
	float XPPerRescue = 50;
	float XPPerObjectiveClear = 200;
	float XPPerAssist = 5;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
#pragma region GAS

protected:
	
	// The ASC on the PlayerState.
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = GAS)
	USMAbilitySystemComponent* AbilitySystemComponent;

	// @TODO: Attribute Sets
	// After some thought about how we should tackle the spawn master attributes, i'm slowly coming
	// to the conclusion that we should just straight up stick it on the player state.
	// I believe the engine knows to do less frequent checks for attributes that are not changing much/at all.
	// I still need to do research on it and try to see if I can possibly work something out with it.

	// maybe we stick a separate attribute set on the spawn master pawn??? ...

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
