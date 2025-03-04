// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/SMAnimThirdPerson.h"
#include "AnimationStateMachineLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Possessables/SMPlayerCharacter.h"

void FTPAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);
	Character = Cast<ASMPlayerCharacter>(InAnimInstance->TryGetPawnOwner());
	if (!Character.Get()) return;
	//MovementComponent = Cast<USMCharacterMovementComponent>(Owner->GetMovementComponent());
}

void FTPAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, const float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
}

void FTPAnimInstanceProxy::Update(const float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}

void USMAnimThirdPerson::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningCharacter = Cast<ASMPlayerCharacter>(TryGetPawnOwner());
	FTimerHandle TimerSleep;
	GetWorld()->GetTimerManager().SetTimer(TimerSleep, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		bWaitForCompAssign = true;
		Proxy.AbilitySystemComponent = OwningCharacter->GetSMAbilitySystemComponent(); 
	}), 0.1, false);
}

void USMAnimThirdPerson::NativeThreadSafeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!Proxy.Character.Get()) return;
	UpdateRotationData(DeltaSeconds);
	UpdateVelocityData();
	UpdateRootYawOffset(DeltaSeconds);
	
	bShouldBlendLegs = Proxy.Character->InventoryComponent->IsUnEquippingCurrentEquippable() || !Proxy.Character->InventoryComponent->HasEquippableEquipped();
	if (bWaitForCompAssign)
	{
		bIsSprinting = Proxy.AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Sprinting")));
	}
}

void USMAnimThirdPerson::UpdateRotationData(float Delta)
{
	if (const APawn* PawnOwner = TryGetPawnOwner(); PawnOwner)
	{
		const FRotator InterpolatedRotator = FMath::RInterpTo(FRotator(AimPitch, 0.0, 0.0f), FRotator(FRotator::NormalizeAxis(PawnOwner->GetBaseAimRotation().Pitch), 0.0f, 0.0f), Delta, 30.0f);
		AimPitch = FMath::Clamp(InterpolatedRotator.Pitch, -90.0f, 90.0f);
		const FRotator CurrentRotation = PawnOwner->GetActorRotation();
		YawDeltaSinceLastUpdate = CurrentRotation.Yaw - WorldRotation.Yaw;
		YawDeltaSpeed = (Delta != 0.0f) ? (YawDeltaSinceLastUpdate / Delta) : 0.0f;
		WorldRotation = CurrentRotation;
	}
}

void USMAnimThirdPerson::UpdateVelocityData()
{
	WorldVelocity = Proxy.Character->GetVelocity();
	WorldVelocity2D = FVector2d(WorldVelocity.X, WorldVelocity.Y);
	Speed = WorldVelocity.Length();
	bIsMoving = Speed > 0;
	const FVector UnrotatedVector = Proxy.Character->GetActorRotation().UnrotateVector(WorldVelocity);
	ForwardVelocity = UnrotatedVector.X / 265.0;
	StrafeVelocity = UnrotatedVector.Y / 265.0;
}

void USMAnimThirdPerson::UpdateRootYawOffset(const float Delta)
{
	if (Speed < 1.0 && !bTurnLegsBack)
	{
		SetRootYawOffset(RootYawOffset-YawDeltaSinceLastUpdate);
		bTurnLegsBack = FMath::Abs(RootYawOffset) > 65.0;
	}
	else
	{
		SetRootYawOffset(UKismetMathLibrary::FloatSpringInterp(
				RootYawOffset,                // Current value
				0.0,              // Target value
				RootYawOffsetSpringState,     // Spring state
				250.0,              // Stiffness
				1.0,  // Critical damping factor
				Delta,                    // Delta time
				1.0,                   // Mass
				0.5    // Target velocity amount
			));
		bTurnLegsBack = !(FMath::Abs(RootYawOffset) < 5.0f);
	}
}

void USMAnimThirdPerson::SetRootYawOffset(const float InRootYawOffset)
{
	RootYawOffset = FMath::ClampAngle( FRotator::NormalizeAxis(InRootYawOffset), -90.0, 90.0);
	AimYaw = RootYawOffset*-1.0;
}

void USMAnimThirdPerson::ProcessTurnYawCurve()
{
	PreviousTurnYawCurveValue = TurnYawCurveValue;
	const float CurveValue = GetCurveValue(FName("TurnYawWeight"));
	if (FMath::IsNearlyEqual(CurveValue, 0.0, 0.0001) )
	{
		TurnYawCurveValue = 0.0;
		PreviousTurnYawCurveValue = 0.0;
		return;
	}
	TurnYawCurveValue =  GetCurveValue(FName("RemainingTurnYaw"))/CurveValue;
	if (PreviousTurnYawCurveValue != 0.0)
	{
		SetRootYawOffset(RootYawOffset - (TurnYawCurveValue - PreviousTurnYawCurveValue));
	}
}

void USMAnimThirdPerson::UpdateIdleState(const  FAnimUpdateContext& Context, const  FAnimNodeReference& Node)
{
	FAnimationStateResultReference AnimationState;
	bool bConversionSucceeded = false;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResultPure(Node, AnimationState,  bConversionSucceeded);
	if (UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationState))
	{
		TurnYawCurveValue = 0;
	} else
	{
		ProcessTurnYawCurve();
	}
}

void USMAnimThirdPerson::PlayFootstepEffect(const bool bIsLeftFoot) const
{
	const FVector Start = bIsLeftFoot ? OwningCharacter->GetMesh()->GetSocketLocation("foot_l") : OwningCharacter->GetMesh()->GetSocketLocation("foot_r");
	const UWorld* W = GetWorld();
	
	TArray<AActor*> ToIgnore;
	ToIgnore.Reserve(1);
	ToIgnore.Add(OwningCharacter);
	//FCollisionQueryParams TraceParams = ConfigureCollisionParams(FName(NoInit), false, ToIgnore, true, W);
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(TraceFootstep), false);
	TraceParams.AddIgnoredActors(ToIgnore);
	TraceParams.bReturnPhysicalMaterial = true;
	FHitResult HitResult;
	const bool bHit = W->LineTraceSingleByChannel(HitResult,Start,FVector(Start.X, Start.Y, Start.Z-75.0),ECC_Visibility,TraceParams);
	if (bHit) {
		const FVector ImpactPoint = HitResult.ImpactPoint;
		const FVector ImpactNormal = HitResult.ImpactNormal;
		const EPhysicalSurface SurfaceType = HitResult.PhysMaterial.Get()->SurfaceType;
		USoundBase* FootSound = nullptr;
		UNiagaraSystem* FootSurface;
		switch (SurfaceType) {
			case SurfaceType_Default:
				FootSound = FootSoundDefault;
				FootSurface = FootSurfaceDefault;
				break;
			case SurfaceType1:
				FootSurface = FootSurfaceAsphalt;
				break;
			case SurfaceType2:
				FootSound = FootSoundConcrete;
				FootSurface = FootSurfaceConcrete;
				break;
			case SurfaceType3:
				FootSound = FootSoundDirt;
				FootSurface = FootSurfaceDirt;
				break;
			case SurfaceType4:
				FootSound = FootSoundMetal;
				FootSurface = FootSurfaceMetal;
				break;
			case SurfaceType5:
				FootSurface = FootSurfaceWater;
				break;
			case SurfaceType6:
				FootSound = FootSoundWood;
				FootSurface = FootSurfaceWood;
				break;
			case SurfaceType7:
				FootSound = FootSoundGlass;
				FootSurface = FootSurfaceGlass;
				break;
			case SurfaceType8:
				FootSound = FootSoundFlesh;
				FootSurface = FootSurfaceFlesh;
				break;
			case SurfaceType9:
				FootSound = FootSoundGrass;
				FootSurface = FootSurfaceGrass;
				break;
			default: FootSound = FootSoundDefault;
				FootSurface = FootSurfaceDefault;
				break;
			}
			const FRotator RotNormal = ImpactNormal.ToOrientationRotator();
		
			UGameplayStatics::PlaySoundAtLocation(W, FootSound, ImpactPoint,  RotNormal, 1,1,0, ImpactAttenuationSettings, nullptr, OwningCharacter);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(W, FootSurface, ImpactPoint,  RotNormal);
	}
}

uint8 USMAnimThirdPerson::SelectCardinalDirectionFromAngle(const float Angle, const float DeadZone, uint8 CurrentDirection,
                                                           const bool bUseCurrentDirection) const
{
	const float AbsAngle = FMath::Abs(Angle);
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = DeadZone;
	if (bUseCurrentDirection)
	{
		if (CurrentDirection == 0)
		{
			FwdDeadZone *= 2.0;
		} else if (CurrentDirection == 1)
		{
			BwdDeadZone *= 2.0;
		}
	}

	if (AbsAngle <= 45.0+FwdDeadZone)
	{
		return 0;//Forward
	}
	if (AbsAngle >= 135.0-BwdDeadZone)
	{
		return 0;//backward
	}
	if (AbsAngle > 0.0)
	{
		return 0;//Right
	}
	return 0; //Left
}

