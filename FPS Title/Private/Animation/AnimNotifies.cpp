// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifies.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Animation/SMAnimThirdPerson.h"
#include "Possessables/SMPlayerCharacter.h"

void USMAN_RightFootStep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	Cast<USMAnimThirdPerson>(Cast<ACharacter>(MeshComp->GetOwner())->GetMesh()->GetAnimInstance())->PlayFootstepEffect(false);
}

void USMAN_LeftFootStep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	Cast<USMAnimThirdPerson>(Cast<ACharacter>(MeshComp->GetOwner())->GetMesh()->GetAnimInstance())->PlayFootstepEffect(true);
}

void USMAN_ChamberGun::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	const FGameplayEventData Payload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		MeshComp->GetOwner(), FGameplayTag::RequestGameplayTag(FName("GameplayEvent.ReChamber")), Payload);
}

void USMAN_ReloadSingleBullet::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	const FGameplayEventData Payload;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		MeshComp->GetOwner(), FGameplayTag::RequestGameplayTag(FName("GameplayEvent.ReloadSingleBullet")), Payload);
}

