// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/SMGA_GunReload.h"
#include "SMGA_RechamberGunReload.generated.h"

class ASMManualRechamberGunBase;
class UAbilityTask_WaitGameplayEvent;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_RechamberGunReload : public USMGA_GunReload
{
	GENERATED_BODY()

public:
	USMGA_RechamberGunReload();

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default", meta=(MultiLine="true"))
	float ReloadAnimationPlayRate;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default", meta=(MultiLine="true"))
	float ExitReloadTime;

	float TimeToDisplay = 1;
	
	UPROPERTY()
	TObjectPtr<ASMManualRechamberGunBase> RechamberGun;
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitEventTask;
	UFUNCTION()
	void OnWaitNotify(FGameplayEventData Payload);
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* CancelReloadTask;
	UFUNCTION()
	void OnCancelReload(FGameplayEventData Payload);
	UPROPERTY()
	USMPlayMontageForMesh* AsyncTaskPlayMontageForMeshEnterReload;
	UFUNCTION()
	void InterruptedReload(FGameplayTag EventTag, FGameplayEventData EventData);
	UFUNCTION()
	void OnBlendOut(FGameplayTag EventTag, FGameplayEventData EventData);
	UPROPERTY()
	USMPlayMontageForMesh* AsyncTaskPlayMontageForMeshReloadLoopHands;
	UPROPERTY()
	USMPlayMontageForMesh* AsyncTaskPlayMontageForMeshReloadLoopWeapon;
	UPROPERTY()
	USMPlayMontageForMesh* AsyncTaskPlayMontageForMeshEndReload;

	FTimerHandle ExitAnimTimer;
	uint8 AmountToReload;
	bool bShouldCancelReload;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	UFUNCTION()
	void OnBulletReload(FGameplayTag EventTag, FGameplayEventData EventData);

	void EnterReload();
	void ReloadLoop();
	void EndReload();
	void SetAmountToReload();

	virtual float DetermineReloadTime() const override;
	virtual void SetupCachables() override;
};
