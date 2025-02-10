// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/SMGA_Rest.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Components/SMPlayerAttributesComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Possessables/SMPlayerCharacter.h"

bool USMGA_Rest::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                    FGameplayTagContainer* OptionalRelevantTags) const
{
	const USMPlayerAttributesComponent* HealthC = Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get())->HealthComp;
	if (HealthC->GetSanity() >= HealthC->GetMaxSanity())
	{
		GEngine->AddOnScreenDebugMessage(-1, TimeToActivate, FColor::White, FString("Canceled Rest"));
		return false;
	} 
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void USMGA_Rest::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	USMPlayerAttributesComponent* HealthC = Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get())->HealthComp;
	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();//Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get())->GetAbilitySystemComponent();
	AsyncTask_WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
	AsyncTask_WaitInputRelease->OnRelease.AddDynamic(this, &USMGA_Rest::OnKeyReleased);
	AsyncTask_WaitInputRelease->ReadyForActivation();
	GEngine->AddOnScreenDebugMessage(-1, TimeToActivate, FColor::White, FString("Rest"));
	
	ASMPlayerCharacter* Char = Cast<ASMPlayerCharacter>(GetAvatarActorFromActorInfo());
	if ( !Char->bResting)
	{
		Char->bResting = true;
		UCharacterMovementComponent* MovementComponent = Char->GetCharacterMovement();
		if (MovementComponent)
		{
			MovementComponent->StopMovementImmediately(); // Stop any current movement
			MovementComponent->DisableMovement();        // Completely disable movement
		}
		// Disable inputs
		APlayerController* PlayerController = Cast<APlayerController>(Char->GetController());
		if (PlayerController)
		{
			PlayerController->SetIgnoreMoveInput(true);  // Ignore movement input
			PlayerController->SetIgnoreLookInput(true);  // Ignore look input
		}
	}

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0); // 0 for the first player
	if (CameraManager)
	{
		CameraManager->StartCameraFade(0.f, 1.f, 5.0f, FLinearColor::Black, true, true);
	}
	
	GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this, HealthC, AbilitySystem, Char ]()
	{
		APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0); // 0 for the first player
		if (CameraManager)
		{
			CameraManager->StartCameraFade(1.f, 0.f, 1.2f, FLinearColor::Black, true, true);
		}

		
		const FGameplayEffectContextHandle EffectContext = AbilitySystem->MakeEffectContext();
		const float OldSanity = HealthC->GetSanity();
		const float MaxSanity = HealthC->GetMaxSanity();
		const FGameplayEffectSpecHandle DamageSpecHandle = AbilitySystem->MakeOutgoingSpec(SanityGE, 1, EffectContext);
		const float ActualSanityToRestore = FMath::Min(1, MaxSanity - OldSanity);
		DamageSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.ValueToApply")), ActualSanityToRestore);
		AbilitySystem->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
		HealthC->OnSanityChanged.Broadcast(HealthC, OldSanity, HealthC->GetSanity(), nullptr);
		
		GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this, HealthC, AbilitySystem, Char ]()
		{
			const FGameplayEffectContextHandle EffectContext = AbilitySystem->MakeEffectContext();
			const float OldSanity = HealthC->GetSanity();
			const float MaxSanity = HealthC->GetMaxSanity();
			const FGameplayEffectSpecHandle DamageSpecHandle = AbilitySystem->MakeOutgoingSpec(SanityGE, 1, EffectContext);
			const float ActualSanityToRestore = FMath::Min(1, MaxSanity - OldSanity);
			DamageSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.ValueToApply")), ActualSanityToRestore);
			AbilitySystem->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
			HealthC->OnSanityChanged.Broadcast(HealthC, OldSanity, HealthC->GetSanity(), nullptr);
		}), 1, true);
	}), TimeToActivate, false);
}

void USMGA_Rest::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	ASMPlayerCharacter* Char = Cast<ASMPlayerCharacter>(GetAvatarActorFromActorInfo());
	UCharacterMovementComponent* MovementComponent = Char->GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->SetMovementMode(MOVE_Walking); // Re-enable walking
	}

	// Enable inputs
	APlayerController* PlayerController = Cast<APlayerController>(Char->GetController());
	if (PlayerController)
	{
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
	}
	
	Char->bResting = false;
}

void USMGA_Rest::OnKeyReleased(float TimeHeld)
{
	GetWorld()->GetTimerManager().ClearTimer(Timer);
	K2_EndAbility();
}
