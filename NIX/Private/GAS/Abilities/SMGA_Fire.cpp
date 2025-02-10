// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_Fire.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Animation/SMAnimHandsFP.h"
#include "Components/SMHealthComponent.h"
#include "Components/SMPlayerAttributesComponent.h"
#include "GameModes/SMTestGameMode.h"
#include "GAS/Abilities/AbilityTasks/SMPlayMontageForMesh.h"
#include "Items/SMEquippableBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SMPlayerState.h"
#include "Possessables/SMBaseHorror.h"
#include "Possessables/SMPlayerCharacter.h"



USMGA_Fire::USMGA_Fire()
{
	bShouldStopFire = false;
	FireDelayTimeSecs = 0.1;
	AmountToDeduct = 1.0;
	FireType = EFireType::SemiAuto;
	static ConstructorHelpers::FObjectFinder<UAnimMontage> MontageObj(TEXT("/Game/NIX/Animations/Overlay/AM_Overlay_FireSingle.AM_Overlay_FireSingle"));
	if (MontageObj.Succeeded())
	{
		OverlayFireSingle = MontageObj.Object;
	}
}

UAnimMontage* USMGA_Fire::DetermineFireAnimation(const EEMeshType MeshType) const
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

void USMGA_Fire::OnRangedWeaponTargetDataReadyCPP(const FGameplayAbilityTargetDataHandle& TargetData) 
{
	Super::OnRangedWeaponTargetDataReadyCPP(TargetData);
	//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString("RUUUUNS"));
	const FHitResult Hit = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetData, 0);
	const UAbilitySystemComponent* AbilitySystem = Char->GetAbilitySystemComponent();
	FGameplayEffectContextHandle EffectContext = AbilitySystem->MakeEffectContext();
	EffectContext.AddHitResult(Hit);
	EffectContext.AddSourceObject(this);
	AActor* TargetActor = Cast<ACharacter>(Hit.GetActor());
	ASMPlayerCharacter* InstigatorActor = Cast<ASMPlayerCharacter>(GetAvatarActorFromActorInfo());
	
	if (K2_HasAuthority() && TargetActor) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Instigator: %s, Target: %s"), 
			*InstigatorActor->GetName(), *TargetActor->GetName()));
		const FGameplayEffectSpecHandle DamageSpecHandle = AbilitySystem->MakeOutgoingSpec(DamageGE, 1, EffectContext);
		DamageSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.DamageToApply")), WeaponDamage);
		ASMPlayerState* PlayerState = Cast<ASMPlayerState>(InstigatorActor->GetPlayerState());
		if (const ASMPlayerCharacter* OtherPlayer = Cast<ASMPlayerCharacter>(TargetActor))
		{
			const float PlayerHealth = OtherPlayer->HealthComp->GetHealth();
			if (PlayerHealth > 0.0f)
			{
				const float ActualDamageDealt = FMath::Min(WeaponDamage, PlayerHealth);
				OtherPlayer->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
				PlayerState->FriendlyFire += ActualDamageDealt;
				//InstigatorActor->SMPlayerState->FriendlyFire += WeaponDamage; TODO I dont know why this crashes
				GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString("Friendly Fire"));
			}
		} else //Enemy
		{
			//Run Hit Marker todo
			ASMBaseHorror* Enemy = Cast<ASMBaseHorror>(TargetActor);
			const float EnemyHealth = Enemy->HealthComp->GetHealth();
			if (EnemyHealth > 0)
			{
				const float ActualDamageDealt = FMath::Min(WeaponDamage, EnemyHealth);
				PlayerState->DamageDealt += ActualDamageDealt;
				Enemy->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
				float& DamageContribution = Enemy->DamageContributors.FindOrAdd(PlayerState, 0.0f);
				DamageContribution += ActualDamageDealt;
				if (Enemy->HealthComp->GetHealth() <= 0)
				{
					GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString("Enemy Killed"));
					PlayerState->KillCount +=1;
					PlayerState->XPCount += PlayerState->XPPerKill;
					Cast<ASMTestGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->IncrementKillCount();
					ProcessAssists(Enemy, PlayerState);
				}
			}
		}
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Damage Amount: %f"),WeaponDamage));
	}
	else 
	{
		if  (K2_HasAuthority() && !TargetActor)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString("No valid target or instigator"));
		}
	}
	const FGameplayTag MatchedTagName;
	const FGameplayTag OriginalTag;
	const FGameplayTagContainer AggregatedSourceTags;
	const FGameplayTagContainer AggregatedTargetTags;
	K2_ExecuteGameplayCueWithParams(FireGameplayCueTag, UAbilitySystemBlueprintLibrary::MakeGameplayCueParameters(0,0,EffectContext,MatchedTagName,OriginalTag,
		AggregatedSourceTags,AggregatedTargetTags, Hit.Location, Hit.Normal, InstigatorActor, nullptr, nullptr,  Hit.PhysMaterial.Get(), 1,1, nullptr, false) );
	const FGameplayTagContainer EventTags;
	AsyncTaskPlayMontageFireAnim = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),DetermineFireAnimation(EEMeshType::Arms3P),EventTags,nullptr,1,FName(NAME_None), false);
	AsyncTaskPlayMontageFireAnim->ReadyForActivation();
}

void USMGA_Fire::ProcessAssists(ASMBaseHorror* KilledEnemy, const ASMPlayerState* KillerPlayerState)
{
	constexpr float AssistThreshold = 0.25f; // Players who deal 25% or more of total damage get an assist

	// Calculate the total damage dealt to the enemy
	float TotalDamage = 0.0f;
	for (const auto& Pair : KilledEnemy->DamageContributors)
	{
		TotalDamage += Pair.Value;
	}
	// Loop through damage contributors to assign assists
	for (const auto& Pair : KilledEnemy->DamageContributors)
	{
		APlayerState* Contributor = Pair.Key;
		const float DamageDealt = Pair.Value;

		// Skip the player who got the final kill
		if (Contributor == KillerPlayerState) continue;

		// Calculate percentage of total damage
		const float DamagePercentage = DamageDealt / TotalDamage;

		// If damage percentage is above the threshold, award an assist
		if (DamagePercentage >= AssistThreshold)
		{
			ASMPlayerState* ContributorPlayerState = Cast<ASMPlayerState>(Contributor);
			if (ContributorPlayerState)
			{
				ContributorPlayerState->FireAssist++;
				ContributorPlayerState->XPCount += ContributorPlayerState->XPPerAssist; // Award XP for assists

				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, FString::Printf(TEXT("Player %s awarded an assist"), *ContributorPlayerState->GetName()));
			}
		}
	}
	KilledEnemy->DamageContributors.Empty();
}

void USMGA_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	Char = Cast<ASMPlayerCharacter>(GetAvatarActorFromActorInfo());
	FTimerHandle FireCompleteHandle;
	switch (FireType) {
		case EFireType::SemiAuto:
			Fire();
			GetWorld()->GetTimerManager().SetTimer(FireCompleteHandle, this, &ThisClass::FireComplete, FireDelayTimeSecs, false, 0);
			break;
		case EFireType::FullAuto:
			bShouldStopFire = false;
			GetWorld()->GetTimerManager().SetTimer(AutoFireTimerHandle, this, &ThisClass::AutoFire, FireDelayTimeSecs, true, 0);
			AsyncTask_WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
			AsyncTask_WaitInputRelease->OnRelease.AddDynamic(this, &USMGA_Fire::SemiAutoOnRelease);
			AsyncTask_WaitInputRelease->ReadyForActivation();
			ArmsAnimInstance = Cast<USMAnimHandsFP>(Char->FirstPersonHandsMesh->GetAnimInstance()); 
			AutoFire();
			break;
	}
	Char->NoiseEvent();
}

void USMGA_Fire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (FireType == EFireType::FullAuto)
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
		if (IsLocallyControlled())
		{
			ArmsAnimInstance->bShouldPlayFireLoop = false;
		}
	}
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

void USMGA_Fire::AutoFire()
{
	if (bShouldStopFire)
	{
		K2_EndAbility();
	} else
	{
		Fire();
	}
}

void USMGA_Fire::SemiAutoOnRelease(float TimeHeld)
{
	bShouldStopFire = true;
	AsyncTask_WaitInputRelease->EndTask();
}

void USMGA_Fire::SMApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::SMApplyCost_Implementation(Handle, ActorInfo, ActivationInfo);
	GetEquippable()->AddAmmo(FMath::Abs(AmountToDeduct)*-1, true);
}

bool USMGA_Fire::SMCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo& ActorInfo) const
{
	return Super::SMCheckCost_Implementation(Handle, ActorInfo) && GetEquippable()->GetCurrentAmmo() > 0 && !Cast<ASMPlayerCharacter>(ActorInfo.AvatarActor.Get())->bGunJam;
}

void USMGA_Fire::Fire()
{
	if (IsLocallyControlled())
	{
		StartRangedTargeting();
		GetEquippable()->ApplyRecoilToController();
		const FGameplayTagContainer EventTags;
		AsyncTaskPlayMontageForMeshArms1P = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),DetermineFireAnimation(EEMeshType::Arms1P),EventTags,Char->FirstPersonHandsMesh,1,FName(NAME_None), false);
		AsyncTaskPlayMontageForMeshArms1P->ReadyForActivation();
		if (!Char->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Aiming"))))
		{
			AsyncTaskPlayMontageForMeshOverlayFire = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),OverlayFireSingle,EventTags,Char->FirstPersonHandsMesh,1,FName(NAME_None), false);
			AsyncTaskPlayMontageForMeshOverlayFire->ReadyForActivation();
		}
		AsyncTaskPlayMontageForMeshArms1PEquippableMesh = USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(this,FName("None"),DetermineFireAnimation(EEMeshType::Equippable1P),EventTags,GetEquippable()->GetEquippableMesh1P(),1,FName(NAME_None), false);
		AsyncTaskPlayMontageForMeshArms1PEquippableMesh->ReadyForActivation();
	}
}

void USMGA_Fire::FireComplete()
{
	K2_EndAbility();
}

