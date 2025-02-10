// Fill out your copyright notice in the Description page of Project Settings.


#include "Possessables/SMSpawnMaster.h"

#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "GAS/AttributeSets/SMNIXAttributeSet.h"
// Sets default values
ASMSpawnMaster::ASMSpawnMaster()
{
	AbilitySystemComponent = CreateDefaultSubobject<USMAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->ReplicationMode = EGameplayEffectReplicationMode::Minimal;
	
	SpawnMasterAttributeSet = CreateDefaultSubobject<USMNIXAttributeSet>(TEXT("NIXAttributeSet"));

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetSphereRadius(16.f, false);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = SphereComponent;
	
	ArmsMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh1P"));
	ArmsMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArmsMesh1P->SetOnlyOwnerSee(true);
	ArmsMesh1P->SetCastShadow(false);
	ArmsMesh1P->SetupAttachment(SphereComponent);
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	FirstPersonCamera->SetupAttachment(ArmsMesh1P, TEXT("head"));

	CameraController = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraController"));
	CameraController->bUsePawnControlRotation = true;
	CameraController->TargetArmLength = 0.0f;
	CameraController->bDoCollisionTest = false;
	CameraController->SetupAttachment(ArmsMesh1P, TEXT("head"));
}

void ASMSpawnMaster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FAttachmentTransformRules CameraControllerRules(EAttachmentRule::KeepRelative, false);
	CameraControllerRules.RotationRule = EAttachmentRule::KeepRelative;
	CameraControllerRules.LocationRule = EAttachmentRule::SnapToTarget;
	
	CameraController->AttachToComponent(SphereComponent, CameraControllerRules);

	ArmsMesh1P->SetWorldLocation(SphereComponent->GetComponentLocation());
	
	FVector newLoc = SphereComponent->GetComponentLocation();
	newLoc.Z = newLoc.Z - (FVector::Dist(FirstPersonCamera->GetComponentLocation(), newLoc));
	ArmsMesh1P->SetWorldLocation(newLoc);

	if (!bFreeLookMovement)
	{
		FAttachmentTransformRules ArmsMesh1PRules(EAttachmentRule::KeepRelative, false);
		ArmsMesh1PRules.RotationRule = EAttachmentRule::KeepRelative;
		ArmsMesh1PRules.LocationRule = EAttachmentRule::KeepRelative;
	
		ArmsMesh1P->AttachToComponent(CameraController, ArmsMesh1PRules);

		bUseControllerRotationPitch = false;
	}
	else
	{
		bUseControllerRotationPitch = true;
	}
}

void ASMSpawnMaster::PawnClientRestart()
{
	Super::PawnClientRestart();

	// Make sure that we have a valid PlayerController.
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller.
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// PawnClientRestart can run more than once in an Actor's lifetime, so start by clearing out any leftover mappings.
			Subsystem->ClearAllMappings();

			for (int32 Idx = 0; Idx < DefaultInputMappingContexts.Num(); Idx++)
			{
				Subsystem->AddMappingContext(DefaultInputMappingContexts[Idx], Idx);
			}
		}
	}
}

UAbilitySystemComponent* ASMSpawnMaster::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

