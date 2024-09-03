// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_MeleeAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Abilities/AbilityTasks/SMPlayMontageForMesh.h"
#include "Possessables/SMPlayerCharacter.h"

USMGA_MeleeAttack::USMGA_MeleeAttack()
{
	
	static ConstructorHelpers::FObjectFinder<UAnimMontage> MontageObj(TEXT("/Game/NIX/Animations/Overlay/AM_Overlay_FireSingle.AM_Overlay_FireSingle")); //Update Melee Attack
	if (MontageObj.Succeeded())
	{
		OverlayFireSingle = MontageObj.Object;
	}
}

void USMGA_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	Attack();
	FTimerHandle AttackCompleteHandle;
	GetWorld()->GetTimerManager().SetTimer(AttackCompleteHandle, this, &ThisClass::AttackComplete, 0.1, false, 0);
}

void USMGA_MeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (IsValid(AsyncTaskPlayMontageForMeshArms1P))
	{
		AsyncTaskPlayMontageForMeshArms1P->EndTask();
	}
	
	if (IsValid(AsyncTaskPlayMontageForMeshOverlayFire))
	{
		AsyncTaskPlayMontageForMeshOverlayFire->EndTask();
	}
	if (IsValid(AsyncTaskPlayMontageForMeshArms1PEquippableMesh))
	{
		AsyncTaskPlayMontageForMeshArms1PEquippableMesh->EndTask();
	}
	if (IsValid(AsyncTaskPlayMontageFireAnim))
	{
		AsyncTaskPlayMontageFireAnim->EndTask();
	}
}

void USMGA_MeleeAttack::Attack()
{
	if (IsLocallyControlled())
	{
		StartRangedTargeting();
		GetEquippable()->ApplyRecoilToController();
		const FGameplayTagContainer EventTags;
		AsyncTaskPlayMontageForMeshArms1P = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),DetermineAttackAnimation(EEMeshType::Arms1P),EventTags,Char->FirstPersonHandsMesh,1,FName(NAME_None), false);
		AsyncTaskPlayMontageForMeshArms1P->ReadyForActivation();
		
		AsyncTaskPlayMontageForMeshArms1PEquippableMesh = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),DetermineAttackAnimation(EEMeshType::Equippable1P),EventTags,GetEquippable()->GetEquippableMesh1P(),1,FName(NAME_None), false);
		AsyncTaskPlayMontageForMeshArms1PEquippableMesh->ReadyForActivation();
	}
}

void USMGA_MeleeAttack::AttackComplete()
{
	K2_EndAbility();
}

UAnimMontage* USMGA_MeleeAttack::DetermineAttackAnimation(EEMeshType MeshType) const
{
	switch (MeshType) {
	case EEMeshType::Arms1P:
		return GetEquippable()->FireAnimations.ArmsMontage1P;
		break;
	case EEMeshType::Equippable1P:
		return GetEquippable()->FireAnimations.EquippableMontage1P;
		break;
	case EEMeshType::Arms3P:
		return GetEquippable()->FireAnimations.FullBodyMontage3P;
		break;
	case EEMeshType::Equippable3P:
		return GetEquippable()->FireAnimations.EquippableMontage3P;
		break;
	}
	return nullptr;
}

void USMGA_MeleeAttack::OnRangedWeaponTargetDataReadyCPP(const FGameplayAbilityTargetDataHandle& TargetData)
{
	const FHitResult Hit = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetData, 0);
	const FGameplayEffectContextHandle EffectContext;
	const FGameplayTag MatchedTagName;
	const FGameplayTag OriginalTag;
	const FGameplayTagContainer AggregatedSourceTags;
	const FGameplayTagContainer AggregatedTargetTags;
	K2_ExecuteGameplayCueWithParams(FireGameplayCueTag, UAbilitySystemBlueprintLibrary::MakeGameplayCueParameters(0,0,EffectContext,MatchedTagName,OriginalTag,
		AggregatedSourceTags,AggregatedTargetTags, Hit.Location, Hit.Normal, GetAvatarActorFromActorInfo(), nullptr, nullptr,  Hit.PhysMaterial.Get(), 1,1, nullptr, false) );
	const FGameplayTagContainer EventTags;
	AsyncTaskPlayMontageFireAnim = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),DetermineAttackAnimation(EEMeshType::Arms3P),EventTags,nullptr,1,FName(NAME_None), false);
	AsyncTaskPlayMontageFireAnim->ReadyForActivation();
	if (K2_HasAuthority())
	{
		BP_ApplyGameplayEffectToTarget(TargetData, DamageGE);
	}
}
