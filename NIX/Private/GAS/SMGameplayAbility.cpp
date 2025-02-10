// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/SMGameplayAbility.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "Items/SMEquippableBase.h"

USMGameplayAbility::USMGameplayAbility()
{
	
}

AController* USMGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (const APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

USMAbilitySystemComponent* USMGameplayAbility::GetSMAbilitySystemComponentFromOwner() const
{
	return static_cast<USMAbilitySystemComponent*>(GetAbilitySystemComponentFromActorInfo());
}

void USMGameplayAbility::SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* InMontage)
{
	ensure(IsInstantiated());

	FAbilityMeshMontage AbilityMeshMontage;
	if (FindAbillityMeshMontage(InMesh, AbilityMeshMontage))
	{
		AbilityMeshMontage.Montage = InMontage;
	}
	else
	{
		CurrentAbilityMeshMontages.Add(FAbilityMeshMontage(InMesh, InMontage));
	}
}

bool USMGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) && SMCheckCost(Handle, *ActorInfo);
}

bool USMGameplayAbility::SMCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo& ActorInfo) const
{
	return true;
}

void USMGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	SMApplyCost(Handle, *ActorInfo, ActivationInfo);
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

bool USMGameplayAbility::FindAbillityMeshMontage(const USkeletalMeshComponent* InMesh, FAbilityMeshMontage& InAbilityMontage)
{
	for (const FAbilityMeshMontage& MeshMontage : CurrentAbilityMeshMontages)
	{
		if (MeshMontage.Mesh == InMesh)
		{
			InAbilityMontage = MeshMontage;
			return true;
		}
	}
	return false;
}
