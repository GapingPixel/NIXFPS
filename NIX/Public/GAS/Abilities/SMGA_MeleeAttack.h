// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SMEquippableAbility.h"
#include "NIX/GlobalFunctions.h"
#include "GameplayEffect.h"
#include "SMGA_MeleeAttack.generated.h"

class UAbilityTask_WaitInputRelease;
class USMAnimHandsFP;
class ASMPlayerCharacter;
class USMPlayMontageForMesh;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_MeleeAttack : public USMEquippableAbility
{
	GENERATED_BODY()
public:

	USMGA_MeleeAttack();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	void Attack();
	void AttackComplete();
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	
	virtual UAnimMontage* DetermineAttackAnimation(EEMeshType MeshType = EEMeshType::Arms1P) const;
	UPROPERTY()
	USMPlayMontageForMesh* AsyncTaskPlayMontageForMeshArms1P;
	UPROPERTY()
	USMPlayMontageForMesh* AsyncTaskPlayMontageForMeshOverlayFire;
	UPROPERTY()
	USMPlayMontageForMesh* AsyncTaskPlayMontageForMeshArms1PEquippableMesh;
	UPROPERTY()
	USMPlayMontageForMesh* AsyncTaskPlayMontageFireAnim;
	UPROPERTY()
	UAbilityTask_WaitInputRelease* AsyncTask_WaitInputRelease;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GEStaminaCost;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Costs")
	float StaminaCost = -20.0f; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	FGameplayTag FireGameplayCueTag;
	
	UPROPERTY()
	TObjectPtr<USMAnimHandsFP> ArmsAnimInstance;
	
	UPROPERTY()
	TObjectPtr<ASMPlayerCharacter> Char;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> DamageGE; //GE DAMAGE GENERIC

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TObjectPtr<UAnimMontage> OverlayFireSingle;

	virtual void OnRangedWeaponTargetDataReadyCPP(const FGameplayAbilityTargetDataHandle& TargetData) override;
	
};
