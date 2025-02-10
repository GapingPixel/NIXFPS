// Fill out your copyright notice in the Description page of Project Settings.


#include "SMGA_Suicide.h"

#include "Components/SMPlayerAttributesComponent.h"
#include "GameModes/SMTestGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Possessables/SMPlayerCharacter.h"

void USMGA_Suicide::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	
	GEngine->AddOnScreenDebugMessage(-1, 2, FColor::White, FString("SUICIDE HAPPENING"));
	
	ASMPlayerCharacter* Char = Cast<ASMPlayerCharacter>(GetAvatarActorFromActorInfo());
	if ( !Char->bResting)
	{
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

	const ASMTestGameMode* GameMode = Cast<ASMTestGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	if (GameMode->bAdultContent)
	{
		//Change Visuals
	}
	
	if (CameraManager)
	{
		CameraManager->StartCameraFade(0.f, 1.f, 5.0f, FLinearColor::Black, true, true);
	}
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this, ActorInfo]()
	{
		USMPlayerAttributesComponent* HealthC = Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get())->HealthComp;
		UAbilitySystemComponent* AbilitySystem = ActorInfo->AbilitySystemComponent.Get();

		const FGameplayEffectContextHandle EffectContext = AbilitySystem->MakeEffectContext();
		const float OldHealth = HealthC->GetHealth();
		const float MaxHealth = HealthC->GetMaxHealth();
		const FGameplayEffectSpecHandle DamageSpecHandle = AbilitySystem->MakeOutgoingSpec(HealthGE, 1, EffectContext);
		const float ActualHealthToSubstract = FMath::Min(1, MaxHealth - OldHealth);
		DamageSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.ValueToApply")), ActualHealthToSubstract);
		AbilitySystem->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
		HealthC->OnHealthChanged.Broadcast(HealthC, OldHealth, HealthC->GetHealth(), nullptr);
		
		APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0); // 0 for the first player
		if (PlayerCameraManager)
		{
			PlayerCameraManager->StartCameraFade(1.f, 0.f, 1.2f, FLinearColor::Black, true, true);
		}
	}), TimeToActivate, false);
	
}
