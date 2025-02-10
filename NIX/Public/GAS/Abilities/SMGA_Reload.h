// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SMEquippableAbilityBase.h"
#include "Items/SMEquippableBase.h"
#include "SMGA_Reload.generated.h"

class UGameplayTask_WaitDelay;
class USMPlayMontageForMesh;
class ASMBaseCharacter;
class ASMGunBase;
class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_Reload : public USMEquippableAbilityBase 
{
	GENERATED_BODY()

public:

	USMGA_Reload();
	UPROPERTY()
	TObjectPtr<UGameplayTask_WaitDelay> ReloadTask;

	UPROPERTY()
	TObjectPtr<ASMBaseCharacter> Character;

	//UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default", AdvancedDisplay, meta=(MultiLine="true"))
	UPROPERTY()
	TObjectPtr<ASMEquippableBase> Equippable;

	UPROPERTY()
	TObjectPtr<ASMGunBase> Gun;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Reload Settings", meta=(MultiLine="true"))
	double ReloadTime;

	virtual UAnimMontage* DetermineReloadMontageArms1P() const;
	virtual UAnimMontage* DetermineReloadMontageEquippable1P() const;
	virtual UAnimMontage* DetermineReloadMontageArms3P() const;
	virtual UAnimMontage* DetermineReloadMontageEquippable3P() const;
	virtual float DetermineReloadTime() const;
	ASMGunBase* GetGunEquippable() const;
	virtual void SetupCachables();

	void DoReload();
	UFUNCTION()
	void TaskWaitDelayFinish();

	UFUNCTION()
	void PlayMontageInterrupted(FGameplayTag Tag, FGameplayEventData Data);
	UFUNCTION()
	void PlayMontageCompleted(FGameplayTag Tag, FGameplayEventData Data);
public:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UPROPERTY()
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageFullBodyMontage3p;

	UPROPERTY()
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageDetermineReloadMontangeArms1p;

	UPROPERTY()
	TObjectPtr<USMPlayMontageForMesh> AsyncTaskPlayMontageDetermineReloadMontageEquippable1p;

	
};
