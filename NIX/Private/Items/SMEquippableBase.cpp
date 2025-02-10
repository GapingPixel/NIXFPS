// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SMEquippableBase.h"

#include "AbilitySystemGlobals.h"
#include "Components/SMEquippableInventoryComponent.h"
#include "Curves/CurveVector.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "GAS/SMGameplayAbility.h"
#include "Interfaces/SMFirstPersonInterface.h"
#include "Net/UnrealNetwork.h"
#include "Player/SMPlayerController.h"
#include "Possessables/SMBaseCharacter.h"
#include "NIX/NIX.h"

#if WITH_EDITOR
void ASMEquippableBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);   

	//StartingAmmo = FMath::Clamp(StartingAmmo, 0, MaxCurrentAmmo);
}
#endif

ASMEquippableBase::ASMEquippableBase()
{
	EquippableMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EquippableMesh1P"));
	EquippableMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippableMesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
	EquippableMesh1P->SetVisibility(false);
	EquippableMesh1P->bOnlyOwnerSee = true;
	
	EquippableMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	EquippableMesh1P->bEnableUpdateRateOptimizations = true;
	EquippableMesh1P->bSkipBoundsUpdateWhenInterpolating = true;
	EquippableMesh1P->bSkipKinematicUpdateWhenInterpolating = true;
	EquippableMesh1P->bComponentUseFixedSkelBounds = true;
	EquippableMesh1P->SetComponentTickEnabled(false);
	//EquippableMesh1P->SetEnableGravity(false);
	
	EquippableMesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EquippableMesh3P"));
	EquippableMesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippableMesh3P->SetCollisionResponseToAllChannels(ECR_Ignore);
	EquippableMesh3P->SetVisibility(false);
	EquippableMesh3P->bOwnerNoSee = true;
	EquippableMesh3P->bCastHiddenShadow = false;
	
	EquippableMesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	EquippableMesh3P->bEnableUpdateRateOptimizations = true;
	EquippableMesh3P->bSkipBoundsUpdateWhenInterpolating = true;
	EquippableMesh3P->bSkipKinematicUpdateWhenInterpolating = true;
	EquippableMesh3P->bComponentUseFixedSkelBounds = true;
	EquippableMesh3P->SetComponentTickEnabled(false);
	//EquippableMesh3P->SetEnableGravity(false);
}

void ASMEquippableBase::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	if (!HasAuthority())
	{
		return;
	}
	
	OnOwnerUpdated(NewOwner);
}

void ASMEquippableBase::OnRep_Owner()
{
	Super::OnRep_Owner();

	OnOwnerUpdated(GetOwner());

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		ASMBaseCharacter* Character = Cast<ASMBaseCharacter>(GetOwner());
		if (Character && Character->GetInventoryComponent()->GetCurrentEquippable() == nullptr)
		{
			Character->GetInventoryComponent()->NextEquippable();
		}
	}
}

void ASMEquippableBase::BeginPlay()
{
	Super::BeginPlay();

	OnExplicitlySpawnedIn();
}

void ASMEquippableBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CurrentAmmo = StartingAmmo == -1 ? MaxCurrentAmmo : StartingAmmo;
}

#if WITH_EDITOR

#endif

void ASMEquippableBase::OnPickUp(USMEquippableInventoryComponent* Inventory)
{
	Super::OnPickUp(Inventory);
	
	if (Inventory && CanPickUp())
	{
		Inventory->GiveExistingEquippable(this);
	}
}

FORCEINLINE bool ASMEquippableBase::CanEquip()
{
	return (GetOwner() != nullptr) && BP_CanEquip();
}

void ASMEquippableBase::AddOrRemoveFromInteractionCount(bool bAddToCount)
{
	if (bAddToCount)
	{
		EquippableInteractionCount += 1;
	}
	else
	{
		EquippableInteractionCount = FMath::Clamp(EquippableInteractionCount - 1, 0, EquippableInteractionCount);
	}

	bIsBeingInteracted = EquippableInteractionCount > 0;

	if (bIsBeingInteracted == false && GetOwner())
	{
		OnEquippableIsIdle.Broadcast();
	}
}

FEquippableAnimCluster ASMEquippableBase::DetermineEquipAnimation_Implementation() const
{
	return ShouldPlayFirstTimeAnimation() ? FirstTimeEquipAnimations : EquipAnimations;
}

void ASMEquippableBase::OnOwnerUpdated(AActor* NewOwner)
{
	if (NewOwner == nullptr)
	{
		// Ensure we are detached from pawn.
		DetachFromPawn(false, true);
		
		if (OwnerFirstPersonInterface)
		{
			USMAbilitySystemComponent* ASC = OwnerFirstPersonInterface->GetSMAbilitySystemComponent();
			check(ASC)
		
			for (FGameplayAbilitySpecHandle Handle : AbilitySpecHandles)
			{
				ASC->CancelAbilityHandle(Handle);
			}
		}

		// Owner has changed, maybe we have been dropped out of the player inventory, clear interface pointer.
		OwnerFirstPersonInterface = nullptr;
	}
	
	// We want to make sure we have a new owner.
	if (Owner && Owner == NewOwner)
	{
		if (NewOwner->Implements<USMFirstPersonInterface>())
		{
			OwnerFirstPersonInterface = Cast<ISMFirstPersonInterface>(NewOwner);
			check(OwnerFirstPersonInterface)
		}
		else
		{
			SM_LOG(Warning, TEXT("NewOwner (%s) in CanEquip does not have USMFirstPersonInterface."), *AActor::GetDebugName(NewOwner));
		}
	}

	// We use the "Owner" to determine if this equippable has been picked up.
	if (GetOwner() != nullptr)
	{
		WorldMeshComponent->SetVisibility(false);
		WorldMeshComponent->SetSimulatePhysics(false);
		WorldMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		EquippableMesh3P->SetCastHiddenShadow(true);

		SetReplicateMovement(false);
	}
	else
	{
		WorldMeshComponent->SetVisibility(true);
		WorldMeshComponent->SetSimulatePhysics(true);
		WorldMeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		
		EquippableMesh3P->SetCastHiddenShadow(false);

		SetReplicateMovement(true);
	}
}

void ASMEquippableBase::AttachToPawn(const bool bFirstPerson) const
{
	if (OwnerFirstPersonInterface != nullptr)
	{
		const FEquippableAnimCluster& EquipAnimsToUse = DetermineEquipAnimation();
		
		if (bFirstPerson)
		{
			USkeletalMeshComponent* ArmsMesh1P = OwnerFirstPersonInterface->GetMeshOfType(EMeshType::FirstPersonHands);
			if (ensure(ArmsMesh1P))
			{
				const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
				
				EquippableMesh1P->AttachToComponent(ArmsMesh1P, Rules, OwnerFirstPersonInterface->GetBoneAttachName(EMeshType::FirstPersonHands));
				EquippableMesh1P->SetVisibility(true);

				if (ensure(ArmsAnimMainBP))
				{
					ArmsMesh1P->SetAnimInstanceClass(ArmsAnimMainBP->GetAnimBlueprintGeneratedClass());
				}
				
				if (FirstPersonAnimLayerABP)
				{
					ArmsMesh1P->LinkAnimClassLayers(FirstPersonAnimLayerABP);
				}
				
				// Play equip montage on first person arms
				if (UAnimInstance* AnimInstanceMesh1P = ArmsMesh1P->GetAnimInstance())
				{
					float EquipReadyTime = AnimInstanceMesh1P->Montage_Play(EquipAnimsToUse.ArmsMontage1P);

					if (EquippableReadyTime == 0.0f)
					{
						// Default is 0.25 if no montage is found.
						EquipReadyTime = EquipReadyTime != 0.0f ? EquipReadyTime : 0.25f;
						SM_LOG(Warning, TEXT("First Person Arms animation failed to play! Equippable: %s, Owner: %s"), *GetDebugName(this), *GetDebugName(GetOwner()))
					}
					else
					{
						EquipReadyTime = EquippableReadyTime;
					}

					UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
					if (ASC)
					{
						const FName TagName = FName("Character.IsChangingEquippable");
						if (!ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TagName)))
						{
							ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TagName));
						}
					}
					const FTimerDelegate FunctionDelegate = FTimerDelegate::CreateUObject(this, &ASMEquippableBase::OnEquippableReadyToFire);
					GetWorld()->GetTimerManager().SetTimer(EquipTimerTimerHandle, FunctionDelegate, EquipReadyTime, false);
				}

				// Play equip montage on first person equippable
				if (UAnimInstance* AnimInstanceEquippableMesh1P = EquippableMesh1P->GetAnimInstance())
				{
					AnimInstanceEquippableMesh1P->Montage_Play(EquipAnimsToUse.EquippableMontage1P);
					SM_LOG(Warning, TEXT("First Person Equippable animation failed to play! Equippable: %s, Owner: %s"), *GetDebugName(this), *GetDebugName(GetOwner()))
				}
			}
		}

		// We want to attach the 3p mesh to the full body mesh anyways, as at the time of writing that will be used for 1p shadow.
		USkeletalMeshComponent* BodyMesh3P = OwnerFirstPersonInterface->GetMeshOfType(EMeshType::ThirdPersonBody);
		if (ensure(BodyMesh3P))
		{
			// Clear unequip timer incase we were already unequipping.
			GetWorld()->GetTimerManager().ClearTimer(UnEquipTimerTimerHandle);

			const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
				
			EquippableMesh3P->AttachToComponent(BodyMesh3P, Rules, OwnerFirstPersonInterface->GetBoneAttachName(EMeshType::ThirdPersonBody));
			EquippableMesh3P->SetVisibility(true);
				
			// Play equip montage on full body in third person
			if (UAnimInstance* AnimInstanceFullBodyMesh3P = BodyMesh3P->GetAnimInstance())
			{
				
				AnimInstanceFullBodyMesh3P->Montage_Play(EquipAnimsToUse.FullBodyMontage3P);
				SM_LOG(Warning, TEXT("Third Person Full Body animation failed to play! Equippable: %s, Owner: %s"), *GetDebugName(this), *GetDebugName(GetOwner()))
			}

			// Play equip montage on equippable in third person
			if (UAnimInstance* AnimInstanceEquippableMesh3P = EquippableMesh3P->GetAnimInstance())
			{
				AnimInstanceEquippableMesh3P->Montage_Play(EquipAnimsToUse.EquippableMontage3P);
				SM_LOG(Warning, TEXT("Third Person Equippable animation failed to play! Equippable: %s, Owner: %s"), *GetDebugName(this), *GetDebugName(GetOwner()))
			}

			// @TODO: link anim class layer to 3p mesh
			BodyMesh3P->LinkAnimClassLayers(ThirdPersonAnimLayerABP);
		}
	}
}

void ASMEquippableBase::DetachFromPawn(bool bFirstPerson, bool bInstant)
{
	if (bInstant)
	{
		OnUnEquipAnimationFinished(bFirstPerson);
		return;
	}
	
	if (!OwnerFirstPersonInterface)
	{
		SM_LOG(Warning, TEXT("OwnerFirstPersonInterface is null in DetachFromPawn. Class: %s, Owner: %s"), *GetDebugName(this), *GetDebugName(this))
		return;
	}
	
	// Incase we were already in the process of equipping this equippable, we don't want to keep this timer running when suddenly requesting to unequip it.
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EquipTimerTimerHandle);
	}
	
	// This function is called when the unequip animation finishes anyway.
	if (bFirstPerson)
	{
		USkeletalMeshComponent* ArmsMesh1P = OwnerFirstPersonInterface->GetMeshOfType(EMeshType::FirstPersonHands);
		if (ensure(ArmsMesh1P))
		{
			ArmsMesh1P->SetAnimInstanceClass(nullptr);
		}
	}

	float montageLength = 0.25f;
	
	if (UnEquipAnimations.ArmsMontage1P)
	{
		montageLength = UnEquipAnimations.ArmsMontage1P->GetPlayLength();
	}
	
	// This code will only run on simulated proxies.
	const USkeletalMeshComponent* BodyMesh3P = OwnerFirstPersonInterface->GetMeshOfType(EMeshType::ThirdPersonBody);
	if (ensure(BodyMesh3P))
	{
		if (UAnimInstance* AnimInstanceFullBodyMesh3P = BodyMesh3P->GetAnimInstance())
		{
			const float length = AnimInstanceFullBodyMesh3P->Montage_Play(UnEquipAnimations.FullBodyMontage3P);
			//montageLength = length == 0.0f ? 0.25 : length; // We want 0.25 seconds minimum for unequipping regardless.
		}
	}

	// Play unequip animation for equippable 3p
	if (UAnimInstance* AnimInstanceEquippableMesh3P = EquippableMesh3P->GetAnimInstance())
	{
		AnimInstanceEquippableMesh3P->Montage_Play(UnEquipAnimations.EquippableMontage3P);
	}

	// @TODO: can we use gameplay events, anim notifies or something similar instead of just a plain old timer?
	// having something event based would be less error prone, maybe even bind it to a 1p montage delegate?
	
	// We don't want to put a timer for first person.
	if (!bFirstPerson)
	{
		const FTimerDelegate functionDelegate = FTimerDelegate::CreateUObject(this, &ASMEquippableBase::OnUnEquipAnimationFinished, false);
		GetWorld()->GetTimerManager().SetTimer(UnEquipTimerTimerHandle, functionDelegate, montageLength, false);
	}
	else
	{
		OnUnEquipAnimationFinished(true);
	}
}

void ASMEquippableBase::AddAbilitiesToOwner()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (!OwnerFirstPersonInterface)
	{
		return;
	}

	USMAbilitySystemComponent* ASC = OwnerFirstPersonInterface->GetSMAbilitySystemComponent();
	check(ASC)

	for (TSubclassOf<USMGameplayAbility>& Ability : Abilities)
	{
		AbilitySpecHandles.Add(ASC->GiveAbility(
			FGameplayAbilitySpec(Ability, 0, INDEX_NONE, this)));
	}
}

void ASMEquippableBase::RemoveAbilitiesFromOwner()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (!OwnerFirstPersonInterface)
	{
		return;
	}
	
	USMAbilitySystemComponent* ASC = OwnerFirstPersonInterface->GetSMAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
	
	for (FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
	{
		ASC->ClearAbility(SpecHandle);
	}
}

void ASMEquippableBase::CancelEquippableAbilities()
{
	if (USMAbilitySystemComponent* ASC = OwnerFirstPersonInterface->GetSMAbilitySystemComponent())
	{
		for (FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
		{
			ASC->CancelAbilityHandle(SpecHandle);
		}
	}
}

void ASMEquippableBase::OnExplicitlySpawnedIn()
{
	if (HasAuthority() && ActorToReceivePostSpawn.IsValid())
	{
		if (USMEquippableInventoryComponent* inventory = USMEquippableInventoryComponent::GetInventoryComponent(ActorToReceivePostSpawn.Get()))
		{
			const bool bSuccess = inventory->GiveExistingEquippable(this);
			if (!bSuccess)
			{
				Destroy();
			}
			else
			{
				ActorToReceivePostSpawn.Reset();
			}
		}
	}
}

void ASMEquippableBase::OnUnEquipAnimationFinished(bool bFirstPerson)
{
	if (bFirstPerson)
	{
		const FDetachmentTransformRules Rules = FDetachmentTransformRules(EDetachmentRule::KeepRelative, false);
		EquippableMesh1P->DetachFromComponent(Rules);
		EquippableMesh1P->SetVisibility(false);

		if (FirstPersonAnimLayerABP)
		{
			USkeletalMeshComponent* armsMesh1P = OwnerFirstPersonInterface->GetMeshOfType(EMeshType::FirstPersonHands);
			if (ensure(armsMesh1P))
			{
				armsMesh1P->UnlinkAnimClassLayers(FirstPersonAnimLayerABP);
			}
		}
	}

	const FDetachmentTransformRules Rules = FDetachmentTransformRules(EDetachmentRule::KeepRelative, false);
	EquippableMesh3P->DetachFromComponent(Rules);
	EquippableMesh3P->SetVisibility(false);
	
	// @TODO: unlink anim class layer to 3p mesh, also maybe not do it here because OwnerFirstPersonInterface can be nullptr here sometimes
	if (OwnerFirstPersonInterface)
	{
		USkeletalMeshComponent* bodyMesh3P = OwnerFirstPersonInterface->GetMeshOfType(EMeshType::ThirdPersonBody);
		if (ensure(bodyMesh3P))
		{
			bodyMesh3P->UnlinkAnimClassLayers(ThirdPersonAnimLayerABP);
		}
	}
}

void ASMEquippableBase::OnEquippableReadyToFire() const
{
	const ISMFirstPersonInterface* Interface = GetOwnerFirstPersonInterface();
	Interface->GetSMAbilitySystemComponent()->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.IsChangingEquippable")));
	
	if (bIsBeingInteracted == false)
	{
		OnEquippableIsIdle.Broadcast();
	}
}

bool ASMEquippableBase::ShouldPlayFirstTimeAnimation() const
{
	return bAllowFirstTimeEquipAnimations == true && bHasBeenPickedUpBefore == false;
}

void ASMEquippableBase::ApplyRecoilToController()
{
	ASMPlayerController* controller = Cast<ASMPlayerController>(GetOwner()->GetInstigatorController());
	check(controller)
	
	if (controller != nullptr && controller->IsLocalController())
	{
		if (RecoilSettings.RecoilCurve)
		{
			float Xmultiplier = 1.0f;
			float Ymultiplier = 1.0f;

			if (bMultiplyRecoilToHeat)
			{
				// @TODO: put this bIsAiming in a function that SMGunBase can use too.
				ISMFirstPersonInterface* interface = GetOwnerFirstPersonInterface();
				check(interface)
	
				const bool bIsAiming = interface->GetSMAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Aiming")));
				const float recoilMultiplier = (GetRecoilHeatMultiplier() * (bIsAiming ? AimRecoilMultiplier : 1.0f));
				
				if (bMultiplyRecoilToHeatX)
				{
					Xmultiplier = 1.0f + recoilMultiplier;
				}

				if (bMultiplyRecoilToHeatY)
				{
					Ymultiplier = 1.0f + recoilMultiplier;
				}
			}
			
			float X = RecoilSettings.RecoilCurve->GetVectorValue(FMath::RandRange(0.f, 1.f)).X * Xmultiplier;
			float Y = RecoilSettings.RecoilCurve->GetVectorValue(FMath::RandRange(0.f, 1.f)).Y * Ymultiplier;
			const FVector2D RecoilAmount(X, Y);
			controller->ApplyRecoil(RecoilAmount, RecoilSettings.RecoilSpeed, RecoilSettings.RecoilResetSpeed);
		}
		else
		{
			SM_LOG(Verbose, TEXT("No Recoil Curve found in %s."), *GetDebugName(this))
		}
	}
}

void ASMEquippableBase::SetAmmo(float AmountToSet)
{
	const float OldAmmo = CurrentAmmo;
	CurrentAmmo = AmountToSet;
	OnCurrentAmmoChanged.Broadcast(OldAmmo, CurrentAmmo);
}

void ASMEquippableBase::AddAmmo(float AmountToAdd, bool bClamp)
{
	float OldAmmo = CurrentAmmo;
	
	if (!bClamp)
	{
		CurrentAmmo += AmountToAdd;
	}
	else
	{
		CurrentAmmo = FMath::Clamp(CurrentAmmo + AmountToAdd, 0, MaxCurrentAmmo);
	}

	OnCurrentAmmoChanged.Broadcast(OldAmmo, CurrentAmmo);
}

bool ASMEquippableBase::BP_CanEquip_Implementation()
{
	return true;
}

void ASMEquippableBase::PreReplication( IRepChangedPropertyTracker & ChangedPropertyTracker )
{
	//Super::PreReplication(ChangedPropertyTracker);
	
	//DOREPLIFETIME_ACTIVE_OVERRIDE(ASMEquippableBase, bHasBeenPickedUpBefore, !bAllowFirstTimeEquipAnimationsPerPlayer);
}

void ASMEquippableBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASMEquippableBase, CurrentAmmo, COND_OwnerOnly);
	DOREPLIFETIME(ASMEquippableBase, bHasBeenPickedUpBefore);
}