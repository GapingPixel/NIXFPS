// Fill out your copyright notice in the Description page of Project Settings.


#include "Possessables/SMPlayerCharacter.h"

#include "AIController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Actors/BarricadeClasses.h"
#include "Actors/SMInteractableActor.h"
#include "Actors/SMLiftableObject.h"
#include "Animation/SMAnimHandsFP.h"
#include "Components/CapsuleComponent.h"
#include "Components/SMHealthComponent.h"
#include "Components/SMPlayerAttributesComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameModes/SMTestGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Player/SMPlayerController.h"
#include "Player/SMPlayerState.h"
#include "Possessables/SMBaseHorror.h"

ASMPlayerCharacter::ASMPlayerCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	FirstPersonHandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonHandsMesh"));
	FirstPersonHandsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonHandsMesh->SetOnlyOwnerSee(true);
	FirstPersonHandsMesh->SetCastShadow(false);
	FirstPersonHandsMesh->SetupAttachment(RootComponent);
	FirstPersonHandsMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	FirstPersonHandsMesh->bEnableUpdateRateOptimizations = true;
	FirstPersonHandsMesh->bSkipBoundsUpdateWhenInterpolating = true;
	FirstPersonHandsMesh->bSkipKinematicUpdateWhenInterpolating = true;
	FirstPersonHandsMesh->bComponentUseFixedSkelBounds = true;
	FirstPersonHandsMesh->SetComponentTickEnabled(false);

	FirstPersonLegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonLegsMesh"));
	FirstPersonLegsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonLegsMesh->SetOnlyOwnerSee(true);
	FirstPersonLegsMesh->SetCastShadow(false);
	FirstPersonLegsMesh->SetupAttachment(RootComponent);
	FirstPersonLegsMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	FirstPersonLegsMesh->bEnableUpdateRateOptimizations = true;
	FirstPersonLegsMesh->bSkipBoundsUpdateWhenInterpolating = true;
	FirstPersonLegsMesh->bSkipKinematicUpdateWhenInterpolating = true;
	FirstPersonLegsMesh->bComponentUseFixedSkelBounds = true;
	FirstPersonLegsMesh->SetComponentTickEnabled(false);

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetCastHiddenShadow(false);
	
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

	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));
	StimuliSourceComponent->RegisterForSense(UAISense_Hearing::StaticClass());
	StimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
	StimuliSourceComponent->RegisterWithPerceptionSystem();
	
	ReviveArea = CreateDefaultSubobject<USphereComponent>("ReviveArea");
	ReviveArea->SetupAttachment(GetCapsuleComponent());
	FGenericTeamId(0);

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	PhysicsHandle->LinearDamping = 6000.0f;  // Prevents jittering and wobbling
	PhysicsHandle->LinearStiffness = 10000.0f; // Holds the object firmly in place

	PhysicsHandle->AngularDamping = 6000.0f;  // Prevents rotational jitter
	PhysicsHandle->AngularStiffness = 10000.0f; 
	PhysicsHandle->SetInterpolationSpeed(500.0f);
	MaxPlayerWounds = 3;

	//Hearing
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	// Configure the hearing sense
	HearingConfig->HearingRange = 2000.0f; // Set range to hear sounds
	HearingConfig->DetectionByAffiliation.bDetectEnemies = false;  // Adjust as necessary
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerceptionComponent->ConfigureSense(*HearingConfig);
	AIPerceptionComponent->Activate();
}

void ASMPlayerCharacter::NoiseEvent()
{
	UAISense_Hearing::ReportNoiseEvent(GetWorld(),GetActorLocation(),1,this,0,FName("NoiseTag"));
}

void ASMPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CameraController->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	FirstPersonHandsMesh->AttachToComponent(CameraController, FAttachmentTransformRules::KeepWorldTransform);
	CameraController->SetRelativeLocation(FVector(0.0f,0.0f,FirstPersonCameraHeight));

	HealthComp->OnDeath.AddDynamic(this, &ThisClass::OnDowned);
	ReviveArea->Deactivate();
	SMPlayerState = Cast<ASMPlayerState>(GetPlayerState());
	PC = Cast<ASMPlayerController>(GetController());
	//ReviveArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		PlayerEnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ThisClass::OnMoveAction);
		PlayerEnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Completed, this, &ThisClass::OnStopMoveAction);
		PlayerEnhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Started, this, &ThisClass::ToggleCrouch);
		PlayerEnhancedInputComponent->BindAction(CrouchInputAction, ETriggerEvent::Completed, this, &ThisClass::ToggleCrouch);
		PlayerEnhancedInputComponent->BindAction(WheelUpInputAction, ETriggerEvent::Started, this, &ThisClass::OnWheelUpAction);
		PlayerEnhancedInputComponent->BindAction(WheelDownInputAction, ETriggerEvent::Started, this, &ThisClass::OnWheelDownAction);
	}
}

void ASMPlayerCharacter::OnMoveAction(const FInputActionValue& Value)
{
	if (bIsDowned)
	{
		return; 
	}
	CurrentMovementInput = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), CurrentMovementInput.X);
	AddMovementInput(GetActorRightVector(), CurrentMovementInput.Y); 
}

void ASMPlayerCharacter::OnStopMoveAction(const FInputActionValue& Value)
{
	CurrentMovementInput = FVector2D::ZeroVector;
}

void ASMPlayerCharacter::OnWheelUpAction()
{
	if (bIsDowned)
	{
		return;
	}
	InventoryComponent->NextEquippable();
}

void ASMPlayerCharacter::OnWheelDownAction()
{
	if (bIsDowned)
	{
		return;
	}
	InventoryComponent->PreviousEquippable();
}

void ASMPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	PC = Cast<ASMPlayerController>(GetController());
	if (PC)
	{
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

bool ASMPlayerCharacter::CanInteractWithBarricadeObject()
{
	if (BarricadeObjectsToEvaluate.IsEmpty()) return false;
	if (BarricadeObjectsToEvaluate.Num() == 1)
	{
		CurrentBarricadeObject = BarricadeObjectsToEvaluate[0];
		return true;
	};
	
	int32 ScreenX, ScreenY;
	PC->GetViewportSize(ScreenX, ScreenY);
	const FVector2D ScreenCenter = FVector2D(ScreenX * 0.5f, ScreenY * 0.5f); // Middle of the screen
	float MinDistance = FLT_MAX;
	for (ASMBarricadeBase* Barricade : BarricadeObjectsToEvaluate)
	{
		if (!Barricade) continue;

		FVector2D ScreenLocation;
		const bool bIsOnScreen = PC->ProjectWorldLocationToScreen(Barricade->GetActorLocation(), ScreenLocation);
		if (bIsOnScreen)
		{
			const float DistanceToCenter = FVector2D::Distance(ScreenLocation, ScreenCenter);
			if (DistanceToCenter < MinDistance)
			{
				MinDistance = DistanceToCenter;
				CurrentBarricadeObject  = Barricade;
			}
		}
	}
	return true;
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
	OriginalSpawnLocation = GetActorLocation();
	OriginalSpawnRotation = GetActorRotation();
	SMPlayerState = Cast<ASMPlayerState>(GetPlayerState());
	if (HasAuthority() && AIPerceptionComponent)
	{
		AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ASMPlayerCharacter::OnPerceptionUpdated);
	}
	FTimerHandle HealthTimer;
	/*GetWorld()->GetTimerManager().SetTimer(HealthTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		const FString FloatAsString = FString::SanitizeFloat(HealthComp->GetSanity());
		GEngine->AddOnScreenDebugMessage(-1,0.4f,FColor::Yellow,FloatAsString);
	}), 1, true);*/
	/*FTimerHandle ExitAnimTimer;
	GetWorld()->GetTimerManager().SetTimer(ExitAnimTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		const FString FloatAsString = FString::SanitizeFloat(HealthComp->GetHealth());
		// Display text on the screen
		GEngine->AddOnScreenDebugMessage(-1,0.4f,FloatAsString);
	}), 0.5, true);*/
	/*FTimerHandle FovTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(FovTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		
		
	}), GetWorld()->DeltaTimeSeconds, true);*/
}

void ASMPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsLocallyControlled())
	{
		SetEquippableFOV();
	}
	if (GrabbedObject.IsA(ASMInteractableActor::StaticClass()))
	{
		if (FirstPersonCamera->GetComponentRotation().Pitch >= -85.0f)
		{
			FVector CameraLocation = FirstPersonCamera->GetComponentLocation();
			FRotator CameraRotation = FirstPersonCamera->GetComponentRotation();
			GrabbedObjectComponent->SetLinearDamping(0.0f); 
			GrabbedObjectComponent->SetAngularDamping(0.0f); 
			PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
			const FVector HoldLocation = CameraLocation + (CameraRotation.Vector() * GrabbedObjectDistance);
			//GrabbedObject->SetActorLocation(HoldLocation);
			PhysicsHandle->SetTargetLocation(HoldLocation);
			PhysicsHandle->SetTargetRotation(CameraRotation);
		} else
		{
			const float Delta = GetWorld()->DeltaTimeSeconds;
			const float CurrentDamping = FMath::Lerp(GrabbedObjectComponent->GetLinearDamping(), 100.0f,  Delta * 10.0f);
			const float CurrentAngularDamping = FMath::Lerp(GrabbedObjectComponent->GetAngularDamping(), 100.0f, Delta * 10.0f);
			GrabbedObjectComponent->SetLinearDamping(CurrentDamping);  // High damping to prevent movement
			GrabbedObjectComponent->SetAngularDamping(CurrentAngularDamping ); // High damping to prevent rotation
		}
	}



	// Define parameters for raycast and sanity mechanics
	constexpr float RayDistance = 1000.0f;      // Distance of the ray
	constexpr float SanityDecreaseRate = 5.0f;  // Sanity decrease rate per second
	float CooldownTime = 2.0f;        // Cooldown time for enemies

	// Raycast setup
	const FVector Start = GetActorLocation();
	const FVector ForwardVector = GetActorForwardVector();
	const FVector End = Start + ForwardVector * RayDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignore the player itself

	// Perform the line trace
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		// Get the actor hit by the raycast
		AActor* HitActor = HitResult.GetActor();

		// Check if the hit actor is an enemy (ASMBase_Horror)
		if (true/*ASMBaseHorror* HorrorEnemy = Cast<ASMBaseHorror>(HitActor)*/)
		{
			// Decrease sanity
			
			const FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
			const float OldSanity = HealthComp->GetSanity();
			const float MaxSanity = HealthComp->GetMaxSanity();
			const FGameplayEffectSpecHandle DamageSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(HealthComp->SanityCostGE, 1, EffectContext);
			//const ASMPlayerCharacter* Player = Cast<ASMPlayerCharacter>(HealthComp->GetOwner());
			const float ActualSanityToRestore = FMath::Min(-0.1, MaxSanity - OldSanity);
			DamageSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.ValueToApply")), ActualSanityToRestore);
			GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
			//GEngine->AddOnScreenDebugMessage(-1, TimeToActivate, FColor::White, FString("PilluSED"));
			HealthComp->OnSanityChanged.Broadcast(HealthComp, OldSanity, HealthComp->GetSanity(), nullptr);
			
		}
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
	if (bIsDowned)
	{
		return;
	}
	const UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	if (bIsCrouched || MoveComp->bWantsToCrouch)
	{
		UnCrouch();
	}
	else if (MoveComp->IsMovingOnGround())
	{
		Crouch();
	}
}

void ASMPlayerCharacter::Jump()
{
	if (bIsDowned)
	{
		return;
	}
	Super::Jump();
}

void ASMPlayerCharacter::PerformDeath(AActor* OwningActor)
{
	Super::PerformDeath(OwningActor);
	//SetThirdPerson(true);
}

void ASMPlayerCharacter::SetThirdPerson_Implementation(bool bThirdPerson)
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
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("GO to first person client"));
	}
}

void ASMPlayerCharacter::OnDowned(AActor* OwningActor)
{
	SMPlayerState = Cast<ASMPlayerState>(GetPlayerState());
	ReviveArea->OnComponentBeginOverlap.AddDynamic(this, &ASMPlayerCharacter::RescueSphereBeginOverlap);
	ReviveArea->OnComponentEndOverlap.AddDynamic(this, &ASMPlayerCharacter::RescueSphereEndOverlap);
	SetThirdPerson(true);
	bIsDowned = true;
	//SMPlayerState->DeathCount++;
	if (PlayerWounds < MaxPlayerWounds)
	{
		PlayerWounds++;
		ReviveArea->Activate();
		//ReviveArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		OnDownedHealthChanged.Broadcast(DownedPlayerHP, DownedPlayerHP);
		GetWorld()->GetTimerManager().SetTimer(DownedPlayerHPTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			const float OldDownedPlayerHP = DownedPlayerHP;
			DownedPlayerHP--;
			if (DownedPlayerHP == 0)
			{
				//DownedPlayerHPTimer.Invalidate();
				GetWorld()->GetTimerManager().ClearTimer(DownedPlayerHPTimer);
				ClientDownedPlayerHPTimerClear();
				DieForGood();
			}
			const FString FloatAsString = FString::SanitizeFloat(DownedPlayerHP);
			// Display text on the screen
			//GEngine->AddOnScreenDebugMessage(-1,1.0f, FColor::Cyan,FloatAsString);
			OnDownedHealthChanged.Broadcast(OldDownedPlayerHP, DownedPlayerHP);
		}), 1, true);
	} else//Die for good
	{
		DieForGood();
		SMPlayerState->DeathCount++;
	}
}

void ASMPlayerCharacter::Rescued_Implementation()
{
	ReviveArea->OnComponentBeginOverlap.RemoveDynamic(this, &ASMPlayerCharacter::RescueSphereBeginOverlap);
	ReviveArea->OnComponentEndOverlap.RemoveDynamic(this, &ASMPlayerCharacter::RescueSphereEndOverlap);
	ReviveArea->Deactivate();
	HealthComp->ResetHealth();
	HealthComp->bIsDead = false;
	bIsDowned = false;
	//DownedPlayerHPTimer.Invalidate();
	GetWorld()->GetTimerManager().ClearTimer(DownedPlayerHPTimer);
	ClientDownedPlayerHPTimerClear();
	DownedPlayerHP = DownedPlayerMaxHP;
	//ReviveArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetThirdPerson(false);
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Runs on Server Rescue"));
}

void ASMPlayerCharacter::ClientDownedPlayerHPTimerClear_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(DownedPlayerHPTimer);
}

void ASMPlayerCharacter::DieForGood()
{
	if (HasAuthority())
	{
		const auto GameMode = Cast<ASMTestGameMode>(GetWorld()->GetAuthGameMode());
		GameMode->QueueRespawnPlayer(PC, OriginalSpawnLocation, OriginalSpawnRotation);
		if (InventoryComponent)
		{
			InventoryComponent->DropAllEquippables(true);
		}
		SetLifeSpan(GameMode->RespawnTime-0.1);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("PLAYER DIED"));
	}
}

void ASMPlayerCharacter::TestResetHealth()
{
	HealthComp->ResetHealth();
}

void ASMPlayerCharacter::RescueAlly() 
{
	PlayerToRescue->Rescued();
	
	SMPlayerState = Cast<ASMPlayerState>(GetPlayerState());
	SMPlayerState->RescueCount++;
	SMPlayerState->XPCount += SMPlayerState->XPPerRescue;
	
}

void ASMPlayerCharacter::ClientRescuePlayerHandleTimerClear_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(RescuePlayerHandle);
}

void ASMPlayerCharacter::RescueSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASMPlayerCharacter* OtherPlayer = Cast<ASMPlayerCharacter>(OtherActor))
	{
		//OtherPlayer->bAbleToRescuePlayer = true;
		OtherPlayer->PlayerToRescue = this;
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString("AbleToRescuePlayer."));
	}
}

void ASMPlayerCharacter::RescueSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ASMPlayerCharacter* OtherPlayer = Cast<ASMPlayerCharacter>(OtherActor))
	{
		//OtherPlayer->bAbleToRescuePlayer = false;
		OtherPlayer->PlayerToRescue = nullptr;
		GetWorld()->GetTimerManager().ClearTimer(RescuePlayerHandle);
		ClientRescuePlayerHandleTimerClear();
	}
}

AActor* ASMPlayerCharacter::CanGrabObject()
{
	if (GrabbedObject != nullptr)
	{
		return nullptr;
	}
	FVector CameraLocation = FirstPersonCamera->GetComponentLocation();
	FRotator CameraRotation = FirstPersonCamera->GetComponentRotation();
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(CameraLocation, CameraRotation);
	const FVector StartPoint = CameraLocation;
	const FVector EndPoint = StartPoint + CameraRotation.Vector() * GrabbedObjectDistance;  

	FHitResult HitResult;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(LineTrace), false, this);
	TraceParams.bReturnPhysicalMaterial = false; 
	if (GetWorld()->LineTraceSingleByChannel(HitResult,StartPoint,EndPoint,ECC_PhysicsBody,TraceParams))
	{
		if ((GrabbedObject = Cast<ASMInteractableActor>(HitResult.GetActor())))
		{
			GrabbedObjectComponent = HitResult.GetComponent();
			PhysicsHandle->GrabComponentAtLocationWithRotation(GrabbedObjectComponent, NAME_None, HitResult.ImpactPoint, GrabbedObjectComponent->GetComponentRotation());
			GrabbedObjectComponent->SetLinearDamping(100.0f);
			GrabbedObjectComponent->SetAngularDamping(100.0f);
			
			GrabbedObjectComponent->SetEnableGravity(false);
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Grabbed Object"));
			return GrabbedObject;
		}
		if ((GrabbedObject = Cast<ASMLiftableObject>(HitResult.GetActor())))
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Grabbed Liftable Object"));
			return GrabbedObject;
		} 
		//DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 1.0f, 0, 1.0f);
	}
	GrabbedObject = nullptr;
	return nullptr;
}

void ASMPlayerCharacter::DropObject()
{
	/*GrabbedObject->StaticMeshComponent->SetSimulatePhysics(true);
	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, false);
	GrabbedObject->GetRootComponent()->DetachFromComponent(DetachmentRules);*/
	
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Drop Object"));
		if (GrabbedObject->IsA<ASMInteractableActor>())
		{
			GrabbedObjectComponent->SetLinearDamping(0.0f);
			GrabbedObjectComponent->SetAngularDamping(0.0f);
			GrabbedObjectComponent->SetEnableGravity(true);
			PhysicsHandle->ReleaseComponent();
		} else //Liftable Obj
		{
		
		}

		GrabbedObject = nullptr;
		GrabbedObjectComponent = nullptr;
	
}


void ASMPlayerCharacter::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (!HasAuthority()) return;

	for (AActor* Actor : UpdatedActors)
	{
		if (!Cast<ASMPlayerCharacter>(Actor))
		{
			break;
		}
		FActorPerceptionBlueprintInfo PerceptionInfo;
		AIPerceptionComponent->GetActorsPerception(Actor, PerceptionInfo);

		// Check each sensed information for the actor
		for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
		{
			if (Stimulus.IsActive())
			{
				//Has Heard Something
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Detected by Hearing!"));
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("Actor Detected: %s"), *Actor->GetName()));
				// Activate a state or move based on detection (sight/hearing)
				const FGameplayEffectContextHandle EffectContext = GetSMAbilitySystemComponent()->MakeEffectContext();
				const FGameplayEffectSpecHandle SanitySpecHandle = GetSMAbilitySystemComponent()->MakeOutgoingSpec(SanityCostGE, 1, EffectContext);
				SanitySpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.DamageToApply")), HeardHorrorSanityCost);
				GetSMAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SanitySpecHandle.Data.Get());
				//NotifyClientsOfSound(Actor);
			} else
			{
					//STOP
			}
		}
	}
}

void ASMPlayerCharacter::NotifyClientsOfSound_Implementation(AActor* SoundSource)
{
	if (SoundSource)
	{
		UE_LOG(LogTemp, Warning, TEXT("Client notified of sound from: %s"), *SoundSource->GetName());

		// Here, the client can respond to the event (e.g., play a sound, show a UI, etc.)
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

void ASMPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASMPlayerCharacter, bIsDowned);
	DOREPLIFETIME(ASMPlayerCharacter, PlayerWounds);
	DOREPLIFETIME(ASMPlayerCharacter, MaxPlayerWounds);
	DOREPLIFETIME(ASMPlayerCharacter, DownedPlayerHP);
	DOREPLIFETIME(ASMPlayerCharacter, ReviveArea);
	DOREPLIFETIME(ASMPlayerCharacter, HealthComp);
	DOREPLIFETIME(ASMPlayerCharacter, DownedPlayerHPTimer);
	DOREPLIFETIME(ASMPlayerCharacter, GrabbedObject);
	DOREPLIFETIME(ASMPlayerCharacter, GrabbedObjectComponent);
	DOREPLIFETIME(ASMPlayerCharacter, bLowSanity);
}