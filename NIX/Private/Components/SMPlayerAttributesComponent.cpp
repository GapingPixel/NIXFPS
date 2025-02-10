// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SMPlayerAttributesComponent.h"

#include "SMGA_Suicide.h"
#include "Actors/MonsterHallucination.h"
#include "Actors/SM_FalsePickUp.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "GAS/AttributeSets/SMHealthAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Possessables/SMPlayerCharacter.h"

USMPlayerAttributesComponent::USMPlayerAttributesComponent()
{
	 SanityBuckets = {
        // [BUCKET 5]
        FSanityEventBucket{
        	5,
            FHallucinationTypeParameters{5,{EHallucinationEvent::None},FIntPoint(0, 0), 0.0f, 0.0f}, // No Auditory Hallucinations
            FHallucinationTypeParameters{5,{},FIntPoint(0, 0), 0.0f, 0.0f}, // No Visual Hallucinations
            FHallucinationTypeParameters{5,{},FIntPoint(0, 0), 0.0f, 0.0f}, // No Mechanic Hallucinations
            //0.0f  No cooldown time
        },

        // [BUCKET 4]
        FSanityEventBucket{
        	4,
            FHallucinationTypeParameters{4,{}, FIntPoint(1, 1), 300.0f, 0.0f}, // 1 Auditory Hallucination
            FHallucinationTypeParameters{4,{},FIntPoint(0, 0), 0.0f, 0.0f},   // No Visual Hallucinations
            FHallucinationTypeParameters{4,{},FIntPoint(0, 0), 0.0f, 0.0f},   // No Mechanic Hallucinations
            //300.0f // Cooldown Time: 300 seconds
        },

        // [BUCKET 3]
        FSanityEventBucket{
        	3,
            FHallucinationTypeParameters{3,{},FIntPoint(1, 2), 300.0f, 0.0f}, // 1-2 Auditory Hallucinations
            FHallucinationTypeParameters{3,{},FIntPoint(0, 1), 300.0f, 0.0f}, // 0-1 Visual Hallucinations
            FHallucinationTypeParameters{3,{},FIntPoint(0, 0), 0.0f, 0.0f},   // No Mechanic Hallucinations
            //300.0f // Cooldown Time: 300 seconds
        },

        // [BUCKET 2]
        FSanityEventBucket{
        	2,
            FHallucinationTypeParameters{2,{},FIntPoint(1, 2), 275.0f, 0.0f}, // 1-2 Auditory Hallucinations
            FHallucinationTypeParameters{2,{},FIntPoint(1, 1), 275.0f, 0.0f}, // 1 Visual Hallucination
            FHallucinationTypeParameters{2,{},FIntPoint(0, 1), 275.0f, 0.0f}, // 0-1 Mechanic Hallucinations
            //275.0f // Cooldown Time: 275 seconds
        },

        // [BUCKET 1]
        FSanityEventBucket{
        	1,
            FHallucinationTypeParameters{1,{},FIntPoint(1, 2), 225.0f, 0.0f}, // 1-2 Auditory Hallucinations
            FHallucinationTypeParameters{1,{},FIntPoint(1, 2), 225.0f, 0.0f}, // 1-2 Visual Hallucinations
            FHallucinationTypeParameters{1,{},FIntPoint(1, 1), 225.0f, 0.0f}, // 1 Mechanic Hallucination
            //225.0f // Cooldown Time: 225 seconds
        },

        // [BUCKET 0]
        FSanityEventBucket{
        	0,
            FHallucinationTypeParameters{0,{},FIntPoint(1, 2), 150.0f, 0.0f}, // 1-2 Auditory Hallucinations
            FHallucinationTypeParameters{0,{},FIntPoint(1, 2), 150.0f, 0.0f}, // 1-2 Visual Hallucinations
            FHallucinationTypeParameters{0,{},FIntPoint(1, 1), 150.0f, 0.0f}, // 1 Mechanic Hallucination (including Suicide)
            //150.0f // Cooldown Time: 150 seconds
        }
    };

	AuditoryHallucination.Events = {
		FSanityEvent{TEXT("Hallucination_Footsteps"), FVector2D(1.0f, 1.5f), false},
		FSanityEvent{TEXT("Hallucination_ImpactNoises"), FVector2D(1.0f, 1.5f), false},
		FSanityEvent{TEXT("Hallucination_ZombieNoises"), FVector2D(1.0f, 1.5f), false},
		FSanityEvent{TEXT("Hallucination_CameraZombieNoise"), FVector2D(0.5f, 1.0f), false}
	};

	// Initialize Mechanic Hallucination Events
	MechanicHallucination.Events = {
		FSanityEvent{TEXT("Hallucination_Jitter"), FVector2D(1.8f, 3.0f), false},
		FSanityEvent{TEXT("Hallucination_GunJam"), FVector2D(2.0f, 5.0f), false},
		FSanityEvent{TEXT("Hallucination_Insomnia"), FVector2D(4.0f, 6.0f), false}
	};

	// Initialize Visual Hallucination Events
	VisualHallucination.Events = {
		FSanityEvent{TEXT("Hallucination_StartHitMarkers"), FVector2D(1.0f, 3.0f), false},
		FSanityEvent{TEXT("Hallucination_DisableAllOutlines"), FVector2D(1.0f, 3.0f), false},
		FSanityEvent{TEXT("Hallucination_DisableGlobalNewBarricadeOutlines"), FVector2D(1.0f, 3.0f), false}
	};
}

void USMPlayerAttributesComponent::ResetSanity()
{
	if (AbilitySystemComponent && SanitySet)
	{
		FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
		EffectContextHandle.AddSourceObject(this);
		const FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(SanityResetGE, 1, EffectContextHandle);
		if (NewHandle.IsValid()){
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
		}
		OnSanityChanged.Broadcast(this, 0.0f, SanitySet->GetSanity(), nullptr); // Broadcasting the change to ensure UI and other components are updated
	}
	bIsDead = false;
}


float USMPlayerAttributesComponent::GetSanity() const
{
	return (SanitySet ? SanitySet->GetSanity() : 0.0f);
}

float USMPlayerAttributesComponent::GetMaxSanity() const
{
	return (SanitySet ? SanitySet->GetMaxSanity() : 0.0f);
}


void USMPlayerAttributesComponent::InitializeWithAbilitySystemComponent(USMAbilitySystemComponent* InASC)
{
	Super::InitializeWithAbilitySystemComponent(InASC);
	SanitySet = AbilitySystemComponent->GetSet<USMSanityAttributeSet>();
	if (!StaminaSet)
	{
		return;
	}
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USMSanityAttributeSet::GetSanityAttribute()).AddUObject(this, &ThisClass::HandleSanityChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USMSanityAttributeSet::GetMaxSanityAttribute()).AddUObject(this, &ThisClass::HandleMaxSanityChanged);
	SanitySet->OnOutOfSanity.AddUObject(this, &ThisClass::HandleOutOfSanity);
	const FOnAttributeChangeData ChangeData;
	OnSanityChanged.Broadcast(this, GetSanity(), GetSanity(), GetInstigatorFromAttrChangeData(ChangeData));

	PlayerCharacterIns = Cast<ASMPlayerCharacter>(GetOwner());
	//Cast<APlayerController>(PlayerCharacterIns->Controller)->ClientStartCameraShake(CameraShake,100); 
	//UGameplayStatics::PlayWorldCameraShake(GetWorld(), CameraShake, GetOwner()->GetActorLocation(), 1,1 );
	
}

void USMPlayerAttributesComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
	SanitySet = nullptr;
}

void USMPlayerAttributesComponent::HandleSanityChanged(const FOnAttributeChangeData& ChangeData)
{
	OnSanityChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
	EvaluateSanity();
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString("EVALUATE SANITY"));
	if (GetSanity() <= .2)
	{
		GetWorld()->GetTimerManager().SetTimer(LowSanityHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			ASMPlayerCharacter* Player = Cast<ASMPlayerCharacter>(GetOwner());
			Player->bLowSanity = true;
			APlayerController* PlayerController = Cast<APlayerController>(Player->GetController());
			if (PlayerController && Player->ConstantCameraShake)
			{
				// Start the camera shake on the local player's camera
				PlayerController->ClientStartCameraShake(Player->ConstantCameraShake);
			}
		}), 20, true);
	} else 
	{
		GetWorld()->GetTimerManager().ClearTimer(LowSanityHandle);
		ASMPlayerCharacter* Player = Cast<ASMPlayerCharacter>(GetOwner());
		Player->bLowSanity = false;
		Cast<APlayerController>(Player->GetController())->ClientStopCameraShake(Player->ConstantCameraShake, false);
	}
}

void USMPlayerAttributesComponent::HandleMaxSanityChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMaxSanityChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue, GetInstigatorFromAttrChangeData(ChangeData));
}

void USMPlayerAttributesComponent::HandleOutOfSanity(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent)
	{
		
	}
#endif
}


void USMPlayerAttributesComponent::StartSanityRegeneration()
{
	//GEngine->AddOnScreenDebugMessage(-1,2.0f,FColor::Yellow,"Start SanityRegeneration");
	if (GetOwner()->HasAuthority() && !GetWorld()->GetTimerManager().IsTimerActive(SanityRegenTimerHandle)) // Server-side authority check
	{
		GetWorld()->GetTimerManager().SetTimer(SanityRegenTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
				if (SanitySet)
				{
					const float CurrentSanity = SanitySet->GetSanity();
					const float MaxSanity = SanitySet->GetMaxSanity();  
					if (CurrentSanity < MaxSanity)
					{
						const FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
						AbilitySystemComponent->BP_ApplyGameplayEffectToTarget(SanityRegenGE,AbilitySystemComponent,1.0f,EffectContext);
						const float NewSanity = SanitySet->GetSanity();
						OnStaminaChanged.Broadcast(this, CurrentSanity, NewSanity, nullptr);
						if (NewSanity >= MaxSanity)
						{
							StopStaminaRegeneration();
						}
					}
					else
					{
						StopStaminaRegeneration();
					}
				}
		}), 0.1f, true);
	}
}

void USMPlayerAttributesComponent::StopSanityRegeneration()
{
	//SanityRegenTimerHandle.Invalidate();
	if (GetWorld()->GetTimerManager().IsTimerActive(SanityRegenTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(SanityRegenTimerHandle);
	}
}

void USMPlayerAttributesComponent::EvaluateSanity()
{
	const float CurrentSanity = GetSanity();
	const int32 BucketIndex = FMath::Clamp(FMath::FloorToInt((100.0f - CurrentSanity) / 10.0f), 0, SanityBuckets.Num() - 1);
	const FSanityEventBucket& CurrentBucket = SanityBuckets[BucketIndex];

	
	
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString("evaluating sanity"));
	//CurrentBucket.BucketIndex = BucketIndex; 
	if (!IsCooldownActive(AuditoryCooldownHandle))
	{
		RollHallucinations(CurrentBucket.Auditory, TEXT("Auditory"),BucketIndex);
	}
	if (!IsCooldownActive(VisualCooldownHandle))
	{
		RollHallucinations(CurrentBucket.Visual, TEXT("Visual"),BucketIndex);
	}
	if (!IsCooldownActive(MechanicCooldownHandle))
	{
		//StartCooldown(MechanicCooldownHandle, 275.0f);
		RollHallucinations(CurrentBucket.Mechanic, TEXT("Mechanic"),BucketIndex);
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString("mechanic "));
	}
}

void USMPlayerAttributesComponent::RollHallucinations(const FHallucinationTypeParameters& Parameters,
	const FString& Type, const int8 BucketIndex)
{
	if (!GetOwner()->HasAuthority()) return;

	//Parameters.
	const int32 NumEvents = FMath::RandRange(Parameters.SimultaneousEventsRange.X, Parameters.SimultaneousEventsRange.Y);
	if (NumEvents > 0)
	{
		TriggerHallucinations(Type, NumEvents, BucketIndex);
		GetWorld()->GetTimerManager().SetTimer((Type == "Auditory" ? AuditoryCooldownHandle :
												Type == "Visual" ? VisualCooldownHandle :
												MechanicCooldownHandle),
											   Parameters.CooldownTime, false);
	}
}

void USMPlayerAttributesComponent::TriggerHallucinations(const FString& Type, int32 NumEvents, const int32 BucketIndex)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString("trigger hallucinations "));
	//int8 RandomIndex = 0;
	/*if (Type == "Auditory")
	{
		for (constexpr int8 i = 0; i < NumEvents; NumEvents++)
		{
			do
			{
				RandomIndex = FMath::RandRange(0, SanityBuckets[BucketIndex].Auditory.Events.Num() - 1);
			} while (RandomIndex == PreviousIndex);
			PreviousIndex = RandomIndex; 
			ProcessSanityEvent(SanityBuckets[BucketIndex].Auditory.Events[RandomIndex]);
			StartCooldown(AuditoryCooldownHandle,SanityBuckets[BucketIndex].Auditory.CooldownTime);
		}
	} else if (Type == "Visual")
	{
		for (constexpr int8 i = 0; i < NumEvents; NumEvents++)
		{
			do
			{
				RandomIndex = FMath::RandRange(0, SanityBuckets[BucketIndex].Visual.Events.Num() - 1);
			} while (RandomIndex == PreviousIndex);
			ProcessSanityEvent(SanityBuckets[BucketIndex].Visual.Events[RandomIndex]);
			StartCooldown(VisualCooldownHandle,SanityBuckets[BucketIndex].Visual.CooldownTime);
		}
	} else*/
	
	const int8 RandomIndex = FMath::RandRange(0, SanityBuckets[0].Mechanic.Events.Num() - 1);
	if (Type == "Mechanic") {
	
		for (uint8 i = 0; i < NumEvents; ++i)
		{
			ProcessSanityEvent(SanityBuckets[BucketIndex].Mechanic.Events[RandomIndex]);
			StartCooldown(MechanicCooldownHandle,SanityBuckets[BucketIndex].Mechanic.CooldownTime);
		}
	} 
}

bool USMPlayerAttributesComponent::IsCooldownActive(FTimerHandle& CooldownHandle) const
{
	// Check if the timer is active
	return GetWorld()->GetTimerManager().IsTimerActive(CooldownHandle);
}

void USMPlayerAttributesComponent::StartCooldown(FTimerHandle& CooldownHandle, float CooldownTime) const
{
	// Start the cooldown timer
	GetWorld()->GetTimerManager().SetTimer(CooldownHandle, CooldownTime, false);
}

void USMPlayerAttributesComponent::ProcessSanityEvent(const EHallucinationEvent& Event)
{
	
	switch (Event)
	{
	
		
	case EHallucinationEvent::Footsteps:
	case EHallucinationEvent::ImpactNoises:
	case EHallucinationEvent::ZombieNoises:
	case EHallucinationEvent::CameraZombieNoise:
		
		if (IsCooldownActive(AuditoryCooldownHandle))
		{
			UE_LOG(LogTemp, Warning, TEXT("Auditory cooldown is active, skipping event."));
			return;
		}
		//UE_LOG(LogTemp, Log, TEXT("Triggering auditory hallucination: %d"), (int)Event.EventType);
		StartCooldown(AuditoryCooldownHandle, 300.0f); // Example cooldown time
		break;

	case EHallucinationEvent::FakeMonster:
		if (!IsCooldownActive(MechanicCooldownHandle))
		{
			SpawnActorWithinRadius(FakeEnemy,4000);
		}
		break;
		
	case EHallucinationEvent::FakePickUp:
		if (!IsCooldownActive(MechanicCooldownHandle))
		{
			SpawnActorWithinRadius(FalsePickUp,2000);
		}
		break;

		//Mechanical
	case EHallucinationEvent::Suicide:
		PlayerCharacterIns->GetSMAbilitySystemComponent()->TryActivateAbilityByClass(USMGA_Suicide::StaticClass(), true);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, FString("suicide"));
		break;
		
	case EHallucinationEvent::GunJam:
		if (!IsCooldownActive(MechanicCooldownHandle))
		{
			PlayerCharacterIns->bGunJam = true;
			//UE_LOG(LogTemp, Warning, TEXT("GunJam aCTIVATE "));
			GetWorld()->GetTimerManager().SetTimer(PlayerCharacterIns->TimerGunJan, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				PlayerCharacterIns->bGunJam = false;
			}), 10.0f, false);
			
			StartCooldown(MechanicCooldownHandle, 275.0f);

			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, FString("GunJam aCTIVATE "));
		} else
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, FString("GunJam FAIL "));
		}
		
		break;
	case EHallucinationEvent::Insomnia:
	case EHallucinationEvent::Jitter:
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, FString("JITTER "));
		PlayerCharacterIns = Cast<ASMPlayerCharacter>(GetOwner());
		Cast<APlayerController>(PlayerCharacterIns->Controller)->ClientStartCameraShake(CameraShake,100); 
		
		if (IsCooldownActive(MechanicCooldownHandle))
		{
			UE_LOG(LogTemp, Warning, TEXT("Mechanic cooldown is active, skipping event."));
			return;
		}
		
		StartCooldown(MechanicCooldownHandle, 275.0f); // Example cooldown time
		break;

		
	case EHallucinationEvent::None:
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unknown or None hallucination event."));
		break;
	}
}

void USMPlayerAttributesComponent::SpawnActorWithinRadius(UClass* ActorClass, float Radius)
{
	UWorld* World = GetWorld();
	// Generate a random point within the radius
	const FVector Origin = PlayerCharacterIns->GetActorLocation();
	const FVector RandomPoint = Origin + FMath::VRand() * FMath::FRandRange(0, Radius);

	// Perform a downward trace to find the ground
	const FVector Start = RandomPoint + FVector(0.0f, 0.0f, 1000.0f); // Start high above the ground
	const FVector End = RandomPoint - FVector(0.0f, 0.0f, 1000.0f);   // Trace downward

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerCharacterIns); // Ignore the center actor during the trace

	if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		// Spawn the actor at the hit location
		FVector SpawnLocation = HitResult.Location;
		FRotator SpawnRotation = FRotator::ZeroRotator; // Adjust rotation if needed

		World->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation);

		UE_LOG(LogTemp, Log, TEXT("Spawned actor at location: %s"), *SpawnLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No ground found for spawning"));
	}
}


