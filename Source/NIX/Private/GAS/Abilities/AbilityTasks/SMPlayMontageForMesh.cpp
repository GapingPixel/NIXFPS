// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/AbilityTasks/SMPlayMontageForMesh.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "GAS/SMGameplayAbility.h"

USMPlayMontageForMesh::USMPlayMontageForMesh(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Rate = 1.f;
	bStopWhenAbilityEnds = true;
}

void USMPlayMontageForMesh::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
	{
		if (Montage == MontageToPlay)
		{
			AbilitySystemComponent->ClearAnimatingAbility(Ability);

			// Reset AnimRootMotionTranslationScale
			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
				(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
			{
				Character->SetAnimRootMotionTranslationScale(1.f);
			}

		}
	}

	if (bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnInterrupted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void USMPlayMontageForMesh::OnAbilityCancelled()
{

	if (StopPlayingMontage(OverrideBlendOutTimeForCancelAbility))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void USMPlayMontageForMesh::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	EndTask();
}

void USMPlayMontageForMesh::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempData = *Payload;
		TempData.EventTag = EventTag;

		EventReceived.Broadcast(EventTag, TempData);
	}
}

USMPlayMontageForMesh* USMPlayMontageForMesh::CreatePlayMontageForMeshAndWaitProxy(UGameplayAbility* OwningAbility,
                                                                                   FName TaskInstanceName,
                                                                                   UAnimMontage* MontageToPlay,
                                                                                   FGameplayTagContainer EventTags,
                                                                                   USkeletalMeshComponent* InMesh,
                                                                                   float Rate,
                                                                                   FName StartSection,
                                                                                   bool bStopWhenAbilityEnds,
                                                                                   float AnimRootMotionTranslationScale,
                                                                                   float StartTimeSeconds,
                                                                                   bool bReplicateMontage,
                                                                                   float OverrideBlendOutTimeForCancelAbility,
                                                                                   float OverrideBlendOutTimeForStopWhenEndAbility)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

	USMPlayMontageForMesh* MyObj = NewAbilityTask<USMPlayMontageForMesh>(OwningAbility, TaskInstanceName);
	MyObj->Mesh = InMesh;
	MyObj->MontageToPlay = MontageToPlay;
	MyObj->EventTags = EventTags;
	MyObj->Rate = Rate;
	MyObj->StartSection = StartSection;
	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
	MyObj->StartTimeSeconds = StartTimeSeconds;
	MyObj->bReplicateMontage = bReplicateMontage;
	MyObj->OverrideBlendOutTimeForCancelAbility = OverrideBlendOutTimeForCancelAbility;
	MyObj->OverrideBlendOutTimeForStopWhenEndAbility = OverrideBlendOutTimeForStopWhenEndAbility;
	
	return MyObj;
}

void USMPlayMontageForMesh::Activate()
{
	Super::Activate();
	
	USMGameplayAbility* SMAbility = Cast<USMGameplayAbility>(Ability);
	if (SMAbility == nullptr)
	{
		return;
	}

	bool bPlayedMontage = false;

	if (USMAbilitySystemComponent* SMAbilitySystemComponent = Cast<USMAbilitySystemComponent>(AbilitySystemComponent))
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();

		if (!Mesh)
		{
			return;
		}
		
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			EventHandle = SMAbilitySystemComponent->AddGameplayEventTagContainerDelegate(EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &USMPlayMontageForMesh::OnGameplayEvent));
			
			if (SMAbilitySystemComponent->PlayMontageForMesh(Mesh, SMAbility, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection, StartTimeSeconds, bReplicateMontage) > 0.f)
			{
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				CancelledHandle = SMAbility->OnGameplayAbilityCancelled.AddUObject(this, &USMPlayMontageForMesh::OnAbilityCancelled);

				BlendingOutDelegate.BindUObject(this, &USMPlayMontageForMesh::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(this, &USMPlayMontageForMesh::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
								  (Character->GetLocalRole() == ROLE_AutonomousProxy && SMAbility->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;
			}
		}
	}
	else
	{
	}

	if (!bPlayedMontage)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	SetWaitingOnAvatar();
}

void USMPlayMontageForMesh::ExternalCancel()
{
	check(AbilitySystemComponent->IsValidLowLevel());

	OnAbilityCancelled();

	Super::ExternalCancel();
}

FString USMPlayMontageForMesh::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	if (Ability && Mesh)
	{
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? MontageToPlay : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}

void USMPlayMontageForMesh::OnDestroy(bool AbilityEnded)
{
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage(OverrideBlendOutTimeForStopWhenEndAbility);
		}
	}

	USMAbilitySystemComponent* SMAbilitySystemComponent = GetTargetAsc();
	if (SMAbilitySystemComponent)
	{
		SMAbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

bool USMPlayMontageForMesh::StopPlayingMontage(float OverrideBlendOutTime)
{
	if (Mesh == nullptr)
	{
		return false;
	}

	USMAbilitySystemComponent* SMAbilitySystemComponent = GetTargetAsc();
	if (!SMAbilitySystemComponent)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return false;
	}

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}
	
	if (SMAbilitySystemComponent && Ability)
	{
		if (SMAbilitySystemComponent->GetAnimatingAbilityFromAnyMesh() == Ability
			&& SMAbilitySystemComponent->GetCurrentMontageForMesh(Mesh) == MontageToPlay)
		{
			// Unbind delegates 
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			SMAbilitySystemComponent->CurrentMontageStopForMesh(Mesh, OverrideBlendOutTime);
			return true;
		}
	}

	return false;
}

USMAbilitySystemComponent* USMPlayMontageForMesh::GetTargetAsc()
{
	return Cast<USMAbilitySystemComponent>(AbilitySystemComponent);
}
