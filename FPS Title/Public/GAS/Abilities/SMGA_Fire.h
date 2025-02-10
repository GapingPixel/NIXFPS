// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/SMEquippableAbility.h"
#include "NIX/GlobalFunctions.h"
#include "GameplayEffect.h"
#include "SMGA_Fire.generated.h"

class ASMBaseHorror;
class ASMPlayerState;
class UAbilityTask_WaitInputRelease;
class USMAnimHandsFP;
class ASMPlayerCharacter;
class USMPlayMontageForMesh;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_Fire : public USMEquippableAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	void Fire();
	void FireComplete();
	void AutoFire();

	UFUNCTION()
	void SemiAutoOnRelease(float TimeHeld);

	virtual void SMApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual bool SMCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const override;
	
	USMGA_Fire();
	
	virtual UAnimMontage* DetermineFireAnimation(EEMeshType MeshType = EEMeshType::Arms1P) const;

	UPROPERTY()
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageForMeshArms1P;
	UPROPERTY()
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageForMeshOverlayFire;
	UPROPERTY()
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageForMeshArms1PEquippableMesh;
	UPROPERTY()
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageFireAnim;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputRelease> AsyncTask_WaitInputRelease;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	float WeaponDamage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	float FireDelayTimeSecs;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	float AmountToDeduct;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default|Advanced")
	bool bShouldStopFire;

	FTimerHandle AutoFireTimerHandle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	FGameplayTag FireGameplayCueTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	EFireType FireType;
	UPROPERTY()
	TObjectPtr<USMAnimHandsFP> ArmsAnimInstance;
	
	UPROPERTY()
	TObjectPtr<ASMPlayerCharacter> Char;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> DamageGE; //GE DAMAGE GENERIC

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TObjectPtr<UAnimMontage> OverlayFireSingle;

	virtual void OnRangedWeaponTargetDataReadyCPP(const FGameplayAbilityTargetDataHandle& TargetData)  override;
	void ProcessAssists(ASMBaseHorror* KilledEnemy, const ASMPlayerState* KillerPlayerState);
};
