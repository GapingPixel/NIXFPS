// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SMGameplayAbility.generated.h"

class ASMEquippableBase;

USTRUCT()
struct NIX_API FAbilityMeshMontage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	class USkeletalMeshComponent* Mesh;

	UPROPERTY()
	class UAnimMontage* Montage;

	FAbilityMeshMontage() : Mesh(nullptr), Montage(nullptr)
	{
	}

	FAbilityMeshMontage(class USkeletalMeshComponent* InMesh, class UAnimMontage* InMontage) 
		: Mesh(InMesh), Montage(InMontage)
	{
	}
};

/**
 * 
 */
UCLASS()
class NIX_API USMGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	USMGameplayAbility();
	
	// The Input Action that will trigger this ability.
	UPROPERTY(EditAnywhere, Category = BindInfo)
	class UInputAction* InputAction;

	UFUNCTION(BlueprintCallable, Category = "SM|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SM|Ability")
	class USMAbilitySystemComponent* GetSMAbilitySystemComponentFromOwner() const;

	void SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* InMontage);

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	bool SMCheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;
	virtual bool SMCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void SMApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;
	virtual void SMApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const {};
	
protected:
	bool FindAbillityMeshMontage(USkeletalMeshComponent* InMesh, FAbilityMeshMontage& InAbilityMontage);

	
	/** Active montages being played by this ability */
	UPROPERTY()
	TArray<FAbilityMeshMontage> CurrentAbilityMeshMontages;
};
