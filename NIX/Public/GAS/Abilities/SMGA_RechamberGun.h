// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/SMEquippableAbilityBase.h"
#include "SMGA_RechamberGun.generated.h"

class USMPlayMontageForMesh;
class UAbilityTask_WaitGameplayEvent;
class ASMPlayerCharacter;
class ASMManualRechamberGunBase;
/**
 * 
 */
UCLASS()
class NIX_API USMGA_RechamberGun : public USMEquippableAbilityBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default", meta=(MultiLine="true"))
	float TimeAfterRechamberToEndAbility;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnSprintReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnRechamberReceived(FGameplayEventData Payload);

	
	UFUNCTION()
	void OnInterrupted(FGameplayTag EventTag, FGameplayEventData EventData);
	UFUNCTION()
	void OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData);
	
	void DoMontages();
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	bool CanContinueWithAbility(const AActor* AvatarActor) const;
	
	UPROPERTY()
	TObjectPtr<ASMManualRechamberGunBase> Gun;
	UPROPERTY()
	TObjectPtr<ASMPlayerCharacter> Character;
	UPROPERTY()
	USMPlayMontageForMesh* MontageTask;
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitForSprintTask;
	
};
