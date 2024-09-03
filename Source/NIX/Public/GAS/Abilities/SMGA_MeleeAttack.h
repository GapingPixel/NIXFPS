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
	

	
	virtual UAnimMontage* DetermineAttackAnimation(EEMeshType MeshType = EEMeshType::Arms1P) const;
	
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageForMeshArms1P;
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageForMeshOverlayFire;
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageForMeshArms1PEquippableMesh;
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageFireAnim;
	TObjectPtr<UAbilityTask_WaitInputRelease> AsyncTask_WaitInputRelease;
	
	
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
