// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/AttributeSets/SMCharacterAttributeSet.h"
#include "SMHealthComponent.generated.h"



class USMHealthAttributeSet;
class USMAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSMHealth_AttributeChanged, USMHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSMHealth_DeathEvent, AActor*, OwningActor);

UENUM()
enum class ETeamID : uint8
{
	
	NoTeam = 0,
	Survivor,
	Zombie
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamChanged, ETeamID, OldTeam, ETeamID, NewTeam);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USMHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	USMHealthComponent();


	UPROPERTY(BlueprintAssignable)
	FSMHealth_AttributeChanged OnHealthChanged;

	// Delegate fired when the max health value has changed.
	UPROPERTY(BlueprintAssignable)
	FSMHealth_AttributeChanged OnMaxHealthChanged;

	// Delegate fired when the character dies.
	UPROPERTY(BlueprintAssignable)
	FSMHealth_DeathEvent OnDeath;

	// Returns the health component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "NIX|Health")
	static USMHealthComponent* FindHealthComponent(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<USMHealthComponent>() : nullptr;  }

#pragma region GAS
	
	/* GAS
	***********************************************************************************/

public:
	
	void InitializeWithAbilitySystemComponent(USMAbilitySystemComponent* InASC);
	void UnInitializeFromAbilitySystemComponent();

	// Returns the current health value.
	UFUNCTION(BlueprintCallable, Category = "NIX|Health")
	float GetHealth() const;

	// Returns the current maximum health value.
	UFUNCTION(BlueprintCallable, Category = "NIX|Health")
	float GetMaxHealth() const;


protected:

	virtual void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);

	// The ASC that owns this Health Component.
	UPROPERTY()
	USMAbilitySystemComponent* AbilitySystemComponent;

	// The Health Attribute Set that the owning ASC has.
	UPROPERTY()
	const USMHealthAttributeSet* HealthSet;

#pragma endregion GAS

#pragma region Team
	
	/* Team
	***********************************************************************************/

public:
	
	UFUNCTION(BlueprintCallable, Category = HealthComponent)
	void SetTeam(ETeamID newTeam);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = HealthComponent)
	ETeamID GetTeam();
	
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = HealthComponent)
	static ETeamID GetTeamFromActor(const AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = HealthComponent)
	static bool IsFriendly(const AActor* ActorA, const AActor* ActorB);

	// @TODO: can we change this type of delegate to something more performant?
	// Called after this component owner team changes.
	UPROPERTY(BlueprintAssignable, Category = HealthComponent)
	FOnTeamChanged OnTeamChange;

private:
	
	UPROPERTY(EditDefaultsOnly, Category = HealthComponent, ReplicatedUsing=OnRep_TeamID, meta=(AllowPrivateAccess=true))
	ETeamID TeamID = ETeamID::NoTeam;

protected:
	UFUNCTION()
	virtual void OnRep_TeamID(ETeamID OldTeam);

#pragma endregion Team

public:

	UFUNCTION()
	void OnRep_IsDead(bool bOldIsDead);
	
private:

	UPROPERTY(ReplicatedUsing=OnRep_IsDead)
	bool bIsDead = false;
};
