// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/SMAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "EnhancedInputComponent.h"
#include "GameplayCueManager.h"
#include "GameFramework/PlayerState.h"
#include "GAS/SMGameplayAbility.h"
#include "Net/UnrealNetwork.h"

static TAutoConsoleVariable<float> CVarReplayMontageErrorThreshold(
	TEXT("GS.replay.MontageErrorThreshold"),
	0.5f,
	TEXT("Tolerance level for when montage playback position correction occurs in replays")
);

float USMAbilitySystemComponent::PlayMontageForMesh(USkeletalMeshComponent* InMesh, USMGameplayAbility* InAnimatingAbility,
                                                    FGameplayAbilityActivationInfo ActivationInfo,
                                                    UAnimMontage* NewAnimMontage, float InPlayRate, FName StartSectionName,
                                                    float StartTimeSeconds, bool bReplicateMontage)
{
	float Duration = -1.f;

	UAnimInstance* AnimInstance = InMesh != nullptr ? InMesh->GetAnimInstance() : nullptr;
	if (AnimInstance && NewAnimMontage)
	{
		Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate, EMontagePlayReturnType::MontageLength, StartTimeSeconds);
		if (Duration > 0.f)
		{

			FGameplayAbilityLocalAnimMontageForMesh& LocalAnimMontageForMeshInfo = GetLocalAnimMontageInfoForMesh(InMesh);

			FGameplayAbilityLocalAnimMontage& LocalMontageInfo = LocalAnimMontageForMeshInfo.LocalMontageInfo;
			
			/*if (LocalMontageInfo.AnimatingAbility != nullptr && LocalMontageInfo.AnimatingAbility != InAnimatingAbility)
			{
			}*/

			if (NewAnimMontage->HasRootMotion() && AnimInstance->GetOwningActor())
			{
				UE_LOG(LogRootMotion, Log, TEXT("UAbilitySystemComponent::PlayMontage %s, Role: %s")
					, *GetNameSafe(NewAnimMontage)
					, *UEnum::GetValueAsString(TEXT("Engine.ENetRole"), AnimInstance->GetOwningActor()->GetLocalRole())
					);
			}

			LocalMontageInfo.AnimMontage = NewAnimMontage;
			LocalMontageInfo.AnimatingAbility = InAnimatingAbility;
			LocalMontageInfo.PlayInstanceId = (LocalMontageInfo.PlayInstanceId < UINT8_MAX ? LocalMontageInfo.PlayInstanceId + 1 : 0);
			
			if (InAnimatingAbility)
			{
				InAnimatingAbility->SetCurrentMontageForMesh(InMesh, NewAnimMontage);
			}
			
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, NewAnimMontage);
			}

			// Replicate to non owners
			if (IsOwnerActorAuthoritative())
			{
				if (bReplicateMontage)
				{
					FGameplayAbilityRepAnimMontageForMesh& MutableRepAnimMontageInfo = GetGameplayAbilityRepAnimMontageForMesh(InMesh);

					MutableRepAnimMontageInfo.RepMontageInfo.Animation = NewAnimMontage; 
					MutableRepAnimMontageInfo.RepMontageInfo.PlayInstanceId = (MutableRepAnimMontageInfo.RepMontageInfo.PlayInstanceId < UINT8_MAX ? MutableRepAnimMontageInfo.RepMontageInfo.PlayInstanceId + 1 : 0);

					MutableRepAnimMontageInfo.RepMontageInfo.SectionIdToPlay = 0;
					if (MutableRepAnimMontageInfo.RepMontageInfo.GetAnimMontage() && StartSectionName != NAME_None)
					{
						MutableRepAnimMontageInfo.RepMontageInfo.SectionIdToPlay = MutableRepAnimMontageInfo.RepMontageInfo.GetAnimMontage()->GetSectionIndex(StartSectionName) + 1;
					}
					
					AnimMontage_UpdateReplicatedDataForMesh(InMesh);

					// Force net update on our avatar actor
					if (AbilityActorInfo->AvatarActor != nullptr)
					{
						AbilityActorInfo->AvatarActor->ForceNetUpdate();
					}
				}
			}
			else
			{
				// If this prediction key is rejected, we need to end the preview
				FPredictionKey PredictionKey = GetPredictionKeyForNewAction();
				if (PredictionKey.IsValidKey())
				{
					PredictionKey.NewRejectedDelegate().BindUObject(this, &USMAbilitySystemComponent::OnPredictiveMontageRejectedForMesh, InMesh, NewAnimMontage);
				}
			}
		}
	}

	return Duration;
}

float USMAbilitySystemComponent::PlayMontageSimulatedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* NewAnimMontage,
	float InPlayRate, FName StartSectionName)
{
	float Duration = -1.f;
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor ? InMesh->GetAnimInstance() : nullptr;
	if (AnimInstance && NewAnimMontage)
	{
		Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate);
		if (Duration > 0.f)
		{
			FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
			AnimMontageInfo.LocalMontageInfo.AnimMontage = NewAnimMontage;
		}
	}

	return Duration;
}

void USMAbilitySystemComponent::CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor ? InMesh->GetAnimInstance() : nullptr;
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	const UAnimMontage* MontageToStop = AnimMontageInfo.LocalMontageInfo.AnimMontage;
	const bool bShouldStopMontage = AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);

	if (bShouldStopMontage)
	{
		const float BlendOutTime = (OverrideBlendOutTime >= 0.0f ? OverrideBlendOutTime : MontageToStop->BlendOut.GetBlendTime());

		AnimInstance->Montage_Stop(BlendOutTime, MontageToStop);

		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedDataForMesh(InMesh);
		}
	}
}

UGameplayAbility* USMAbilitySystemComponent::GetAnimatingAbilityFromAnyMesh()
{
	for (FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes)
	{
		
		if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility != nullptr)
		{
			return &*GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility;
		}
	}

	return nullptr;
}

UAnimMontage* USMAbilitySystemComponent::GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh)
{
	const UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor ? InMesh->GetAnimInstance() : nullptr;
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (AnimMontageInfo.LocalMontageInfo.AnimMontage && AnimInstance
		&& AnimInstance->Montage_IsActive(AnimMontageInfo.LocalMontageInfo.AnimMontage))
	{
		return AnimMontageInfo.LocalMontageInfo.AnimMontage;
	}

	return nullptr;
}

FGameplayAbilityLocalAnimMontageForMesh& USMAbilitySystemComponent::GetLocalAnimMontageInfoForMesh(USkeletalMeshComponent* Mesh)
{
	for (FGameplayAbilityLocalAnimMontageForMesh& LocalAnimMontage : LocalAnimMontageInfoForMeshes)
	{
		if (LocalAnimMontage.Mesh == Mesh)
		{
			return LocalAnimMontage;
		}
	}

	const FGameplayAbilityLocalAnimMontageForMesh NewAbilityLocalAnimMontageForMesh = FGameplayAbilityLocalAnimMontageForMesh(Mesh);
	LocalAnimMontageInfoForMeshes.Add(NewAbilityLocalAnimMontageForMesh);

	return LocalAnimMontageInfoForMeshes.Last();
}

FGameplayAbilityRepAnimMontageForMesh& USMAbilitySystemComponent::GetGameplayAbilityRepAnimMontageForMesh(
	USkeletalMeshComponent* InMesh)
{
	for (FGameplayAbilityRepAnimMontageForMesh& RepMontageInfo : RepAnimMontageInfoForMeshes)
	{
		if (RepMontageInfo.Mesh == InMesh)
		{
			return RepMontageInfo;
		}
	}

	FGameplayAbilityRepAnimMontageForMesh RepMontageInfo = FGameplayAbilityRepAnimMontageForMesh(InMesh);
	RepAnimMontageInfoForMeshes.Add(RepMontageInfo);
	return RepAnimMontageInfoForMeshes.Last();
}

void USMAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh(USkeletalMeshComponent* InMesh)
{
	check(IsOwnerActorAuthoritative());

	AnimMontage_UpdateReplicatedDataForMesh(GetGameplayAbilityRepAnimMontageForMesh(InMesh));
}

void USMAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh(FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo)
{
	const UAnimInstance* AnimInstance = IsValid(OutRepAnimMontageInfo.Mesh) ? OutRepAnimMontageInfo.Mesh->GetAnimInstance() : nullptr;
	if (AnimInstance && LocalAnimMontageInfo.AnimMontage)
	{
		FGameplayAbilityRepAnimMontage& RepMontageInfo = OutRepAnimMontageInfo.RepMontageInfo;
		
		RepMontageInfo.Animation = LocalAnimMontageInfo.AnimMontage; 
		
		// Compressed Flags
		const bool bIsStopped = AnimInstance->Montage_GetIsStopped(LocalAnimMontageInfo.AnimMontage);

		if (!bIsStopped)
		{
			RepMontageInfo.PlayRate = AnimInstance->Montage_GetPlayRate(LocalAnimMontageInfo.AnimMontage);
			RepMontageInfo.Position = AnimInstance->Montage_GetPosition(LocalAnimMontageInfo.AnimMontage);
			RepMontageInfo.BlendTime = AnimInstance->Montage_GetBlendTime(LocalAnimMontageInfo.AnimMontage);
		}

		if (RepMontageInfo.IsStopped != bIsStopped)
		{
			RepMontageInfo.IsStopped = bIsStopped;
			if (AbilityActorInfo->AvatarActor != nullptr)
			{
				AbilityActorInfo->AvatarActor->ForceNetUpdate();
			}
			
			UpdateShouldTick();
		}

		// Replicate NextSectionID to keep it in sync.
		// We actually replicate NextSectionID+1 on a BYTE to put INDEX_NONE in there.
		const int32 CurrentSectionID = LocalAnimMontageInfo.AnimMontage->GetSectionIndexFromPosition(RepMontageInfo.Position);
		if (CurrentSectionID != INDEX_NONE)
		{
			const int32 NextSectionID = AnimInstance->Montage_GetNextSectionID(LocalAnimMontageInfo.AnimMontage, CurrentSectionID);
			if (NextSectionID >= (256 - 1))
			{
				UE_LOG(LogTemp,  Error, TEXT("AnimMontage_UpdateReplicatedData. NextSectionID = %d.  RepAnimMontageInfo.Position: %.2f, CurrentSectionID: %d. LocalAnimMontageInfo.AnimMontage %s"), 
					NextSectionID, RepMontageInfo.Position, CurrentSectionID, *GetNameSafe(LocalAnimMontageInfo.AnimMontage) );
				ensure(NextSectionID < (256 - 1));
			}
			RepMontageInfo.NextSectionID = static_cast<uint8>(NextSectionID + 1);
		}
		else
		{
			RepMontageInfo.NextSectionID = 0;
		}
	}
}

void USMAbilitySystemComponent::OnPredictiveMontageRejectedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* PredictiveMontage)
{
	static const float MONTAGE_PREDICTION_REJECT_FADETIME = 0.25f;

	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor ? InMesh->GetAnimInstance() : nullptr;
	if (AnimInstance && PredictiveMontage)
	{
		if (AnimInstance->Montage_IsPlaying(PredictiveMontage))
		{
			AnimInstance->Montage_Stop(MONTAGE_PREDICTION_REJECT_FADETIME, PredictiveMontage);
		}
	}
}

bool USMAbilitySystemComponent::IsReadyForReplicatedMontageForMesh(USkeletalMeshComponent* InMesh)
{
	return true;
}

void USMAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}

void USMAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters);
}

void USMAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
}

void USMAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	const USMGameplayAbility* ability = Cast<USMGameplayAbility>(AbilitySpec.Ability);
	if (ability && GetNetMode() != NM_DedicatedServer) // @TODO: stop this code running on listen servers for simulated proxies
	{
		if (!ability->InputAction)
		{
			UE_LOG(LogTemp, Log, TEXT("Input Action in ability %s does not have an Input Action bound to it."), *GetNameSafe(ability))
			Super::OnGiveAbility(AbilitySpec);
			return;
		}

		// Bind the Input Action to the AbilityLocalInputX functions with the correct InputIDs
		OwnerPlayerInputComponent = GetInputComponent();
		if (OwnerPlayerInputComponent)
		{
			FAbilityInput inputStruct;

			AbilitySpec.InputID = GetNextInputID();
			inputStruct.InputID = AbilitySpec.InputID;

			inputStruct.PressedHandle = OwnerPlayerInputComponent->BindAction(ability->InputAction, ETriggerEvent::Started, this, &UAbilitySystemComponent::AbilityLocalInputPressed, AbilitySpec.InputID).GetHandle();
			inputStruct.ReleasedHandle = OwnerPlayerInputComponent->BindAction(ability->InputAction, ETriggerEvent::Completed,this, &UAbilitySystemComponent::AbilityLocalInputReleased, AbilitySpec.InputID).GetHandle();
			inputStruct.CancelledHandle = OwnerPlayerInputComponent->BindAction(ability->InputAction, ETriggerEvent::Canceled,this, &UAbilitySystemComponent::AbilityLocalInputReleased, AbilitySpec.InputID).GetHandle();

			AbilityInputArray.AddUnique(inputStruct);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("InputComponent not found in OnGiveAbility in %s"), *GetNameSafe(this))
		}
	}

	Super::OnGiveAbility(AbilitySpec);
}

void USMAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);
	
	if (bCachedIsNetSimulated && GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	
	
	UEnhancedInputComponent* InputComponent = GetInputComponent();
	if (InputComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("inputComponent in OnRemoveAbility returned nullptr in %s. Has the owner changed or something somehow? Server/Client: %i"), *GetNameSafe(this), (int32)bCachedIsNetSimulated)
		return;
	}

	for (const FAbilityInput abilityInput : AbilityInputArray)
	{
		const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(abilityInput.Handle);
		if (Spec)
		{
			if (Spec->InputID == abilityInput.InputID)
			{
				InputComponent->RemoveBindingByHandle(abilityInput.PressedHandle);
				InputComponent->RemoveBindingByHandle(abilityInput.ReleasedHandle);
				InputComponent->RemoveBindingByHandle(abilityInput.CancelledHandle);

				AbilityInputArray.Remove(abilityInput);
				break;
			}
		}
	}
}

void USMAbilitySystemComponent::OnRep_ReplicatedAnimMontageForMesh()
{
	for (FGameplayAbilityRepAnimMontageForMesh& NewRepMontageInfoForMesh : RepAnimMontageInfoForMeshes)
	{
		const UWorld* World = GetWorld();

		FGameplayAbilityRepAnimMontageForMesh& AnimMontageInfo = GetGameplayAbilityRepAnimMontageForMesh(NewRepMontageInfoForMesh.Mesh);
		FGameplayAbilityRepAnimMontage& RepAnimMontage = AnimMontageInfo.RepMontageInfo;
		
		if (RepAnimMontage.bSkipPlayRate)
		{
			GetGameplayAbilityRepAnimMontageForMesh(NewRepMontageInfoForMesh.Mesh).RepMontageInfo.PlayRate = 1.f;
		}

		const bool bIsPlayingReplay = World && World->IsPlayingReplay();

		const float MONTAGE_REP_POS_ERR_THRESH = bIsPlayingReplay ? CVarReplayMontageErrorThreshold.GetValueOnGameThread() : 0.1f;

		UAnimInstance* AnimInstance = IsValid(NewRepMontageInfoForMesh.Mesh) && NewRepMontageInfoForMesh.Mesh->GetOwner()
			== AbilityActorInfo->AvatarActor ? NewRepMontageInfoForMesh.Mesh->GetAnimInstance() : nullptr;
		if (AnimInstance == nullptr || !IsReadyForReplicatedMontageForMesh(NewRepMontageInfoForMesh.Mesh))
		{
			bPendingMontageRep = true;
			return;
		}
		bPendingMontageRep = false;

		if (!AbilityActorInfo->IsLocallyControlled())
		{
			static const auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("net.Montage.Debug"));
			const bool DebugMontage = (CVar && CVar->GetValueOnGameThread() == 1);
			if (DebugMontage)
			{
				UE_LOG(LogTemp,  Warning, TEXT("\n\nOnRep_ReplicatedAnimMontageForMesh, %s"), *GetNameSafe(this));
				UE_LOG(LogTemp,  Warning, TEXT("\tAnimMontage: %s\n\tPlayRate: %f\n\tPosition: %f\n\tBlendTime: %f\n\tNextSectionID: %d\n\tIsStopped: %d\n\tPlayInstanceId: %d"),
					*GetNameSafe(RepAnimMontage.GetAnimMontage()),
					RepAnimMontage.PlayRate,
					RepAnimMontage.Position,
					RepAnimMontage.BlendTime,
					RepAnimMontage.NextSectionID,
					RepAnimMontage.IsStopped,
					RepAnimMontage.PlayInstanceId);
				UE_LOG(LogTemp, Warning, TEXT("\tLocalAnimMontageInfo.AnimMontage: %s\n\tPosition: %f"),
					*GetNameSafe(RepAnimMontage.GetAnimMontage()), AnimInstance->Montage_GetPosition(RepAnimMontage.GetAnimMontage()));
			}

			if (NewRepMontageInfoForMesh.RepMontageInfo.GetAnimMontage())
			{
				if ((RepAnimMontage.GetAnimMontage() != NewRepMontageInfoForMesh.RepMontageInfo.GetAnimMontage()) || 
				    (RepAnimMontage.PlayInstanceId != NewRepMontageInfoForMesh.RepMontageInfo.PlayInstanceId))
				{
					RepAnimMontage.PlayInstanceId = NewRepMontageInfoForMesh.RepMontageInfo.PlayInstanceId;
					PlayMontageSimulatedForMesh(NewRepMontageInfoForMesh.Mesh, NewRepMontageInfoForMesh.RepMontageInfo.GetAnimMontage(), NewRepMontageInfoForMesh.RepMontageInfo.PlayRate);
				}

				if (RepAnimMontage.GetAnimMontage()== nullptr)
				{ 
					
				}
				
				if (AnimInstance->Montage_GetPlayRate(RepAnimMontage.GetAnimMontage()) != NewRepMontageInfoForMesh.RepMontageInfo.PlayRate)
				{
					AnimInstance->Montage_SetPlayRate(RepAnimMontage.GetAnimMontage(), NewRepMontageInfoForMesh.RepMontageInfo.PlayRate);
				}

				const int32 SectionIdToPlay = (static_cast<int32>(NewRepMontageInfoForMesh.RepMontageInfo.SectionIdToPlay) - 1);
				if (SectionIdToPlay != INDEX_NONE)
				{
					const FName SectionNameToJumpTo = RepAnimMontage.GetAnimMontage()->GetSectionName(SectionIdToPlay);
					if (SectionNameToJumpTo != NAME_None)
					{
						AnimInstance->Montage_JumpToSection(SectionNameToJumpTo);
					}
				}

				const bool bIsStopped = AnimInstance->Montage_GetIsStopped(RepAnimMontage.GetAnimMontage());
				const bool bReplicatedIsStopped = static_cast<bool>(NewRepMontageInfoForMesh.RepMontageInfo.IsStopped);
				
				if (bReplicatedIsStopped)
				{
					if (!bIsStopped)
					{
						CurrentMontageStopForMesh(NewRepMontageInfoForMesh.Mesh, NewRepMontageInfoForMesh.RepMontageInfo.BlendTime);
					}
				}
				else if (!NewRepMontageInfoForMesh.RepMontageInfo.SkipPositionCorrection)
				{
					const int32 RepSectionID = RepAnimMontage.GetAnimMontage()->GetSectionIndexFromPosition(NewRepMontageInfoForMesh.RepMontageInfo.Position);
					const int32 RepNextSectionID = static_cast<int32>(NewRepMontageInfoForMesh.RepMontageInfo.NextSectionID) - 1;
					
					if( RepSectionID != INDEX_NONE )
					{
						const int32 NextSectionID = AnimInstance->Montage_GetNextSectionID(RepAnimMontage.GetAnimMontage(), RepSectionID);
						
						if( NextSectionID != RepNextSectionID )
						{
							AnimInstance->Montage_SetNextSection(RepAnimMontage.GetAnimMontage()->GetSectionName(RepSectionID), RepAnimMontage.GetAnimMontage()->GetSectionName(RepNextSectionID), RepAnimMontage.GetAnimMontage());
						}
						
						const int32 CurrentSectionID = RepAnimMontage.GetAnimMontage()->GetSectionIndexFromPosition(AnimInstance->Montage_GetPosition(RepAnimMontage.GetAnimMontage()));
						if ((CurrentSectionID != RepSectionID) && (CurrentSectionID != RepNextSectionID))
						{
							// Client is in a wrong section, teleport him into the begining of the right section
							const float SectionStartTime = RepAnimMontage.GetAnimMontage()->GetAnimCompositeSection(RepSectionID).GetTime();
							AnimInstance->Montage_SetPosition(RepAnimMontage.GetAnimMontage(), SectionStartTime);
						}
					}
					
					const float CurrentPosition = AnimInstance->Montage_GetPosition(RepAnimMontage.GetAnimMontage());
					const int32 CurrentSectionID = RepAnimMontage.GetAnimMontage()->GetSectionIndexFromPosition(CurrentPosition);
					const float DeltaPosition = NewRepMontageInfoForMesh.RepMontageInfo.Position - CurrentPosition;

					
					if ((CurrentSectionID == RepSectionID) && (FMath::Abs(DeltaPosition) > MONTAGE_REP_POS_ERR_THRESH) && (NewRepMontageInfoForMesh.RepMontageInfo.IsStopped == 0))
					{
						// fast forward to server position and trigger notifies
						if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(NewRepMontageInfoForMesh.RepMontageInfo.GetAnimMontage()))
						{
							// Skip triggering notifies if we're going backwards in time, we've already triggered them.
							const float DeltaTime = !FMath::IsNearlyZero(NewRepMontageInfoForMesh.RepMontageInfo.PlayRate) ? (DeltaPosition / NewRepMontageInfoForMesh.RepMontageInfo.PlayRate) : 0.f;
							if (DeltaTime >= 0.f)
							{
								MontageInstance->UpdateWeight(DeltaTime);
								MontageInstance->HandleEvents(CurrentPosition, NewRepMontageInfoForMesh.RepMontageInfo.Position, nullptr);
								AnimInstance->TriggerAnimNotifies(DeltaTime);
							}
						}
						AnimInstance->Montage_SetPosition(RepAnimMontage.GetAnimMontage(), NewRepMontageInfoForMesh.RepMontageInfo.Position);
					}
				}
			}
		}
	}
}

UEnhancedInputComponent* USMAbilitySystemComponent::GetInputComponent() const
{

	if (OwnerPlayerInputComponent)
	{
		return OwnerPlayerInputComponent;
	}
	
	if (GetOwnerActor())
	{
		if (APlayerState* state = Cast<APlayerState>(GetOwnerActor()))
		{
			if (APlayerController* controller = Cast<APlayerController>(state->GetPlayerController()))
			{
				return Cast<UEnhancedInputComponent>(controller->InputComponent.Get());
			}
		}
	}

	return nullptr;
}

void USMAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USMAbilitySystemComponent, RepAnimMontageInfoForMeshes);
}