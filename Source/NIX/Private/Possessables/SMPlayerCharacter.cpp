// Fill out your copyright notice in the Description page of Project Settings.


#include "Possessables/SMPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/SMAnimHandsFP.h"
#include "Components/CapsuleComponent.h"
#include "Components/SMHealthComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

ASMPlayerCharacter::ASMPlayerCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FirstPersonHandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonHandsMesh"));
	FirstPersonHandsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonHandsMesh->SetOnlyOwnerSee(true);
	FirstPersonHandsMesh->SetCastShadow(false);
	FirstPersonHandsMesh->SetupAttachment(RootComponent);

	FirstPersonLegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonLegsMesh"));
	FirstPersonLegsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonLegsMesh->SetOnlyOwnerSee(true);
	FirstPersonLegsMesh->SetCastShadow(false);
	FirstPersonLegsMesh->SetupAttachment(RootComponent);

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetCastHiddenShadow(true);
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	FirstPersonCamera->SetupAttachment(FirstPersonHandsMesh, TEXT("head"));

	CameraController = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraController"));
	CameraController->bUsePawnControlRotation = true;
	CameraController->TargetArmLength = 0.0f;
	CameraController->bDoCollisionTest = false;
	CameraController->SetupAttachment(FirstPersonHandsMesh, TEXT("head"));

	ThirdPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonSpringArm"));
	ThirdPersonSpringArm->bEnableCameraLag = true;
	ThirdPersonSpringArm->CameraLagSpeed = 15.f;
	ThirdPersonSpringArm->TargetArmLength = 200.f;
	ThirdPersonSpringArm->TargetOffset = FVector(0.f, 0.f, 0.f);
	ThirdPersonSpringArm->SetupAttachment(RootComponent);

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(ThirdPersonSpringArm);

	bIsCrouching = false;
	CrouchSpeed = 5.0f; // Adjust speed as necessary
	CrouchDuration = 0.2f; // Time taken to fully crouch or stand up, adjust as needed
}

void ASMPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CameraController->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	FirstPersonHandsMesh->AttachToComponent(CameraController, FAttachmentTransformRules::KeepWorldTransform);
	CameraController->SetRelativeLocation(FVector(0.0f,0.0f,FirstPersonCameraHeight));

	DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	DefaultCameraZOffset = FirstPersonCamera->GetRelativeLocation().Z;

	// Set the target to the default values
	TargetCapsuleHalfHeight = DefaultCapsuleHalfHeight;
	TargetCameraZOffset = DefaultCameraZOffset;

	HealthComp->OnDeath.AddDynamic(this, &ThisClass::OnDeath);
}

void ASMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);
	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		PlayerEnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ThisClass::OnMoveAction);
		PlayerEnhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Started, this, &ThisClass::ToggleCrouch);
		PlayerEnhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Completed, this, &ThisClass::ToggleCrouch);
		PlayerEnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		PlayerEnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &ThisClass::StopJumping);
		PlayerEnhancedInputComponent->BindAction(WheelUpInputAction, ETriggerEvent::Started, this, &ThisClass::OnWheelUpAction);
		PlayerEnhancedInputComponent->BindAction(WheelDownInputAction, ETriggerEvent::Started, this, &ThisClass::OnWheelDownAction);
	}
}


void ASMPlayerCharacter::OnMoveAction(const FInputActionValue& Value)
{
	const FVector2d MovementInput = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), MovementInput.X);
	AddMovementInput(GetActorRightVector(), MovementInput.Y); 
}

void ASMPlayerCharacter::OnWheelUpAction()
{
	InventoryComponent->NextEquippable();
}

void ASMPlayerCharacter::OnWheelDownAction()
{
	InventoryComponent->PreviousEquippable();
}

void ASMPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();

			for (int32 Idx = 0; Idx < DefaultInputMappingContexts.Num(); Idx++)
			{
				Subsystem->AddMappingContext(DefaultInputMappingContexts[Idx], Idx);
			}
		}
	}
}


void ASMPlayerCharacter::SetEquippableFOV()
{
	const USMAnimHandsFP* AnimIns = Cast<USMAnimHandsFP>(FirstPersonHandsMesh->GetAnimInstance());
	if (IsValid(AnimIns))
	{
		if (IsValid(InventoryComponent->GetCurrentEquippable()))
		{
			FirstPersonCamera->SetFieldOfView(FMath::FInterpTo(FOVLastFrame,AnimIns->bIsADS ? InventoryComponent->GetCurrentEquippable()->EquippableAimFOV : DefaultFOV, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()),10));
			FOVLastFrame = FirstPersonCamera->FieldOfView;
		}
	}
}

void ASMPlayerCharacter::OnDeath(AActor* OwningActor)
{
	USkeletalMeshComponent* MeshIns = GetMesh();
	MeshIns->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshIns->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetSimulatePhysics(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	AttachmentRules.LocationRule = EAttachmentRule::KeepRelative;
	AttachmentRules.RotationRule = EAttachmentRule::KeepRelative;
	AttachmentRules.ScaleRule = EAttachmentRule::KeepRelative;
	ThirdPersonSpringArm->AttachToComponent(MeshIns, AttachmentRules, FName("spine_03") );
}

void ASMPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocallyControlled())
	{
		DefaultFOV = FirstPersonCamera->FieldOfView;
		FOVLastFrame = DefaultFOV;
		
	}
	if (IsValid(GetController()))
	{
		if (!HasAuthority() && GetController()->IsLocalPlayerController())
		{
			GetMesh()->HideBoneByName(FName("head"),PBO_None);
		}
	}
	/*FTimerHandle FOVTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(FOVTimerHandle, this, &ASMPlayerCharacter::SetEquippableFOV, GetWorld()->GetDeltaSeconds(), true);*/
}

void ASMPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetEquippableFOV();
}


void ASMPlayerCharacter::PerformDeath(AActor* OwningActor)
{
	Super::PerformDeath(OwningActor);

	SetThirdPerson(true);
}

void ASMPlayerCharacter::SetThirdPerson(bool bThirdPerson)
{
	if (bThirdPerson && bIsFirstPerson)
	{
		FirstPersonHandsMesh->SetHiddenInGame(true);
		FirstPersonLegsMesh->SetHiddenInGame(true);
		GetMesh()->SetOwnerNoSee(false);

		ThirdPersonCamera->SetActive(true);
		FirstPersonCamera->SetActive(false);

		bIsFirstPerson = false;
	}
	else if (!bThirdPerson && !bIsFirstPerson)
	{
		FirstPersonHandsMesh->SetHiddenInGame(false);
		FirstPersonLegsMesh->SetHiddenInGame(false);
		GetMesh()->SetOwnerNoSee(true);

		ThirdPersonCamera->SetActive(false);
		FirstPersonCamera->SetActive(true);

		bIsFirstPerson = true;
	}
}

void ASMPlayerCharacter::ToggleThirdPerson()
{
	if (bIsFirstPerson)
	{
		SetThirdPerson(true);
	}
	else
	{
		SetThirdPerson(false);
	}
}

void ASMPlayerCharacter::ToggleCrouch()
{
	const auto MoveComp = GetCharacterMovement();

	if (bIsCrouched || MoveComp->bWantsToCrouch)
	{
		UnCrouch();
	}
	else if (MoveComp->IsMovingOnGround())
	{
		Crouch();
	}
}

void ASMPlayerCharacter::StartCrouch()
{
	if (bIsCrouching) return;

	bIsCrouching = true;
	TargetCapsuleHalfHeight = DefaultCapsuleHalfHeight / 2.0f; // Target crouch height
	TargetCameraZOffset = DefaultCameraZOffset - 40.0f; // Adjust as needed

	GetWorld()->GetTimerManager().SetTimer(CrouchTimerHandle, this, &ASMPlayerCharacter::UpdateCrouch, GetWorld()->GetDeltaSeconds(), true);
}

void ASMPlayerCharacter::StopCrouch()
{
	if (!bIsCrouching) return;

	bIsCrouching = false;
	TargetCapsuleHalfHeight = DefaultCapsuleHalfHeight;
	TargetCameraZOffset = DefaultCameraZOffset;

	GetWorld()->GetTimerManager().SetTimer(CrouchTimerHandle, this, &ASMPlayerCharacter::UpdateCrouch, GetWorld()->GetDeltaSeconds(), true);
}

void ASMPlayerCharacter::UpdateCrouch()
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Interpolate the capsule half-height
	float CurrentCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float NewCapsuleHalfHeight = FMath::FInterpTo(CurrentCapsuleHalfHeight, TargetCapsuleHalfHeight, DeltaTime, CrouchSpeed);
	GetCapsuleComponent()->SetCapsuleHalfHeight(NewCapsuleHalfHeight);

	// Adjust the character's location to avoid visual popping
	float HalfHeightAdjustment = DefaultCapsuleHalfHeight - NewCapsuleHalfHeight;
	FVector NewLocation = GetActorLocation();
	NewLocation.Z += HalfHeightAdjustment; 
	SetActorLocation(NewLocation);

	// Interpolate the camera position
	FVector NewCameraLocation = FirstPersonCamera->GetRelativeLocation();
	NewCameraLocation.Z = FMath::FInterpTo(NewCameraLocation.Z, TargetCameraZOffset, DeltaTime, CrouchSpeed);
	FirstPersonCamera->SetRelativeLocation(NewCameraLocation);

	// Stop the timer if the target values are reached
	if (FMath::IsNearlyEqual(NewCapsuleHalfHeight, TargetCapsuleHalfHeight, 1.0f) &&
		FMath::IsNearlyEqual(NewCameraLocation.Z, TargetCameraZOffset, 1.0f))
	{
		GetWorld()->GetTimerManager().ClearTimer(CrouchTimerHandle);
	}
}


USkeletalMeshComponent* ASMPlayerCharacter::GetMeshOfType(EMeshType MeshType)
{
	switch (MeshType)
	{
	case EMeshType::FirstPersonHands:
		return FirstPersonHandsMesh;

	case EMeshType::ThirdPersonBody:
		return GetMesh();

	case EMeshType::FirstPersonLegs:
		return FirstPersonLegsMesh;
			
	default:
		return Super::GetMeshOfType(MeshType); // In case in the future the base function does something else funky, we call the super anyways to be sure.
	}
}
