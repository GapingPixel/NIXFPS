// Fill out your copyright notice in the Description page of Project Settings.


#include "Possessables/SMBaseCharacter.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "Components/SMCharacterMovementComponent.h"
#include "Components/SMEquippableInventoryComponent.h"
#include "Player/SMPlayerState.h"
#include "Components/SMHealthComponent.h"
#include "Components/SMPawnComponent.h"
#include "Components/SMPlayerAttributesComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "GAS/AttributeSets/SMCharacterAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "NIX/NIX.h"

// Sets default values
ASMBaseCharacter::ASMBaseCharacter(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<USMCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
	{
	HealthComp = CreateDefaultSubobject<USMPlayerAttributesComponent>(TEXT("HealthComp"));
	
	PawnComponent = CreateDefaultSubobject<USMPawnComponent>(TEXT("PawnComponent"));
	PawnComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ASMBaseCharacter::OnAbilitySystemComponentInitialized));

	InventoryComponent = CreateDefaultSubobject<USMEquippableInventoryComponent>(TEXT("InventoryComponent"));

	CharacterAttributeSet = CreateDefaultSubobject<USMCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	
	GetCapsuleComponent()->SetCollisionObjectType(COLLISION_SMCHARACTERBASE);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_SMCHARACTERBASE, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(TRACECHANNEL_BULLET, ECR_Ignore);

	GetMesh()->SetCollisionResponseToChannel(TRACECHANNEL_BULLET, ECR_Block);
	
	SetReplicates(true);
}

/* AActor Functions 
***********************************************************************************/

void ASMBaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// We do this check here for players who join late.
	if (!InventoryComponent->GetCurrentEquippable())
	{
		// Start out unarmed.
		GetMesh()->LinkAnimClassLayers(UnarmedAnimationBlueprint);
	}
	
	InventoryComponent->OnCurrentEquippableChanged.AddDynamic(this, &ASMBaseCharacter::OnCurrentEquippableChanged);
	//HealthComp->OnDeath.AddDynamic(this, &ASMBaseCharacter::PerformDeath);
}

// Called on client only
void ASMBaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	SetupGas();
}

void ASMBaseCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	const bool bIsListenServerOrStandalone = GetNetMode() == NM_ListenServer || GetNetMode() == NM_Standalone;
	if (!HasAuthority() && !bIsListenServerOrStandalone)
	{
		return;
	}
	
	if (OtherActor->IsA(ASMBaseCharacter::StaticClass()))
	{
		if (ASMBaseCharacter* Character = Cast<ASMBaseCharacter>(OtherActor))
		{
			OverlappingCharacters.Emplace(Character);
			GetWorld()->GetTimerManager().SetTimer(PushTimerHandle, this, &ASMBaseCharacter::ApplyPersonalSpacePush, ForceApplicationTime, true, 0.f);
		}
	}
}

void ASMBaseCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	const bool bIsListenServerOrStandalone = GetNetMode() == NM_ListenServer || GetNetMode() == NM_Standalone;
	if (!HasAuthority() && !bIsListenServerOrStandalone)
	{
		return;
	}
	
	if (OtherActor->IsA(ASMBaseCharacter::StaticClass()))
	{
		if (ASMBaseCharacter* Character = Cast<ASMBaseCharacter>(OtherActor))
		{
			if (OverlappingCharacters.Contains(Character))
			{
				OverlappingCharacters.RemoveSingle(Cast<ASMBaseCharacter>(OtherActor));
			}

			if (OverlappingCharacters.Num() < 1)
			{
				GetWorld()->GetTimerManager().ClearTimer(PushTimerHandle);
			}
		}
	}
}

void ASMBaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		InventoryComponent->DropAllEquippables(true);
		InventoryComponent->ForceDropCurrentEquippableBeforeDestroy();
	}
	
	OnAbilitySystemComponentUnInitialized();
	
	Super::EndPlay(EndPlayReason);
}

void ASMBaseCharacter::ApplyPersonalSpacePush()
{
	// @TODO: do this pls
	// the code below technically works, but fucking sucks over a connection (it lags like fucking crazy)
	
	
	for (const ASMBaseCharacter* Character : OverlappingCharacters)
	{
		if (Character)
		{
			// This is what pushes characters away from each other.
			
			FVector DistanceBetweenCharacters = Character->GetActorLocation() - this->GetActorLocation();
			const float Distance = DistanceBetweenCharacters.Length();
			const float CapsuleDiameter = Character->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2;
			const float DistanceToMultiplyBy = FMath::Max(0.0f, (CapsuleDiameter - Distance) * DistanceMultiplier);
			 
			FVector Direction = DistanceBetweenCharacters;
			Direction.Normalize();
			Character->GetCharacterMovement()->AddForce(Direction * PushForce * DistanceToMultiplyBy);
		}
	}
}

/* APawn Functions 
***********************************************************************************/

// Called on server only (or standalone)
void ASMBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	SetupGas();
	AddDefaultAbilities(PawnComponent->GetSMAbilitySystemComponent(), DefaultAbilities);
}

void ASMBaseCharacter::PerformDeath(AActor* OwningActor)
{
	if (HasAuthority())
	{
		InventoryComponent->DropAllEquippables(true);
	}
}

void ASMBaseCharacter::OnCurrentEquippableChanged(ASMEquippableBase* OldEquippable)
{
	if (!UnarmedAnimationBlueprint)
	{
		SM_LOG(Warning, TEXT("UnarmedAnimationBlueprint is nullptr in %s. Please make sure it is set in the blueprint derived class."), *GetDebugName(this))
		return;
	}
	
	if (InventoryComponent->GetCurrentEquippable())
	{
		GetMesh()->UnlinkAnimClassLayers(UnarmedAnimationBlueprint);
	}
	else
	{
		GetMesh()->LinkAnimClassLayers(UnarmedAnimationBlueprint);
	}
}

USMCharacterMovementComponent* ASMBaseCharacter::GetMyMovementComponent() const
{
	// checking if the movement component returns is the custom one, at compile time
	return static_cast<USMCharacterMovementComponent*>(GetCharacterMovement());
}

/* GAS
***********************************************************************************/

UAbilitySystemComponent* ASMBaseCharacter::GetAbilitySystemComponent() const
{
	return PawnComponent->GetSMAbilitySystemComponent();
}

void ASMBaseCharacter::OnAbilitySystemComponentInitialized()
{
	// @TODO: instead of using GetSMAbilitySystemComponent, how about we actually store the variable on this character?
	HealthComp->InitializeWithAbilitySystemComponent(GetSMAbilitySystemComponent());
	//BP_OnAbilitySystemComponentInitialized();
}

void ASMBaseCharacter::OnAbilitySystemComponentUnInitialized() const
{
	HealthComp->UnInitializeFromAbilitySystemComponent();
}

void ASMBaseCharacter::SetupGas()
{
	/* Get the ASC from the Player State and cache it here on the pawn. */
	SMPlayerStateCache = GetPlayerState<ASMPlayerState>();
	if (SMPlayerStateCache)
	{
		USMAbilitySystemComponent* Asc = Cast<USMAbilitySystemComponent>(SMPlayerStateCache->GetAbilitySystemComponent());
		if (!Asc)
		{
			UE_LOG(LogNIX, Error, TEXT("AbilitySystemComponent in %s is nullptr. Did the PlayerState setup its ASC correctly? Server/Client: %i"), *GetNameSafe(this), (int32)HasAuthority())
		}
		
		PawnComponent->InitializeAbilitySystemComponent(Asc, SMPlayerStateCache);

		CharacterAttributeSet = SMPlayerStateCache->GetCharacterAttributeSet();

		SetupAttributes();
	}
	else
	{
		UE_LOG(LogNIX, Error, TEXT("Player State cast failed in %s in SetupGas(). Server/Client: %i"), *GetNameSafe(this), (int32)HasAuthority())
	}

	// @TODO: GET FUCKING RID OF THIS SHIT
	// Move this to one of the following places:
	// - AssetManager class’s start initial loading function
	// - UEngineSubsystem::Initialize()
	// - UEngine::Init()
	static int32 DoOnce = 0;
	if (DoOnce == 0)
	{
		DoOnce++;
		UAbilitySystemGlobals::Get().InitGlobalData();
	}
}

void ASMBaseCharacter::SetupAttributes() const
{
	if (USMAbilitySystemComponent* ASC = GetSMAbilitySystemComponent())
	{
		FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
		EffectContextHandle.AddSourceObject(this);

		const FGameplayEffectSpecHandle NewHandle = ASC->MakeOutgoingSpec(StartingAttributes, 1, EffectContextHandle);
		if (NewHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
		}
		HealthComp->StartStaminaRegeneration();
	}
}

void ASMBaseCharacter::AddDefaultAbilities(USMAbilitySystemComponent* ASC, TArray<FAbilityClassWithLevel>& AbilitiesToAdd)
{
	if (ASC)
	{
		check(ASC)
		// Give default abilities.
		for (const FAbilityClassWithLevel& AbilityInfo : AbilitiesToAdd)
		{
			// The input will be assigned in OnGiveAbility in the custom ASC assuming that a USMGameplayAbility is being passed through.
			ASC->K2_GiveAbility(AbilityInfo.Ability, AbilityInfo.Level);
		}
	}
}

/* Getters
***********************************************************************************/

float ASMBaseCharacter::GetMovementSpeed() const
{
	if (CharacterAttributeSet)
	{
		return CharacterAttributeSet->GetMovementSpeed();
	}
	
	return 0.0f;
}

USkeletalMeshComponent* ASMBaseCharacter::GetMeshOfType(EMeshType MeshType)
{
	switch (MeshType)
	{

	case EMeshType::ThirdPersonBody:
		return GetMesh();
			
	default:
		return nullptr;
	}
}

FName ASMBaseCharacter::GetBoneAttachName(EMeshType MeshType)
{
	switch (MeshType)
	{
		case EMeshType::ThirdPersonBody:
			return ThirdPersonEquippableBone;
			
		case EMeshType::FirstPersonHands:
			return FirstPersonEquippableBone;
			
		default:
			return FName("NULL");
	}
}

USMEquippableInventoryComponent* ASMBaseCharacter::GetInventoryComponent()
{
	return InventoryComponent;
}

void ASMBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASMBaseCharacter, bIsSprinting, COND_SimulatedOnly);
	DOREPLIFETIME(ASMBaseCharacter, HealthComp);
	//DOREPLIFETIME(ASMBaseCharacter, HealthComp);
}