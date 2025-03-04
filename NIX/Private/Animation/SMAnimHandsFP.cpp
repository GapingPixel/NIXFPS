// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/SMAnimHandsFP.h"
#include "Components/SMCharacterMovementComponent.h"
#include "Components/SMHealthComponent.h"
#include "Components/SMPlayerAttributesComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Possessables/SMPlayerCharacter.h"

void FASAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);
	Character = Cast<ASMPlayerCharacter>(InAnimInstance->TryGetPawnOwner());
	if (!Character.Get()) return;

	MovementComponent = Cast<USMCharacterMovementComponent>(Character->GetMovementComponent());
}

void FASAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
}

void FASAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}

void USMAnimHandsFP::OnADS(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	bIsADS = true;
}

void USMAnimHandsFP::OnUnADS(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	bIsADS = false;
}

void USMAnimHandsFP::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
	if (!Proxy.Character.Get()) return;
	const FVector Velocity = Proxy.Character->GetVelocity();
	const float Speed = Velocity.Length();
	const FVector UnrotatedVector = Proxy.Character->GetActorRotation().UnrotateVector(Velocity);
	LocalForwardVelocity = UnrotatedVector.X;
	LocalStrafeVelocity = UnrotatedVector.Y;
	LocalVerticalVelocity = UnrotatedVector.Z;
	Time = Time + DeltaSeconds;
	LocalForwardVelocityPercentage = LocalForwardVelocity != 0.0 ? LocalForwardVelocity/Proxy.Character->GetMovementComponent()->GetMaxSpeed() : 0;
	LocalStrafeVelocityPercentage = LocalStrafeVelocity != 0.0 ? LocalStrafeVelocity/Proxy.Character->GetMovementComponent()->GetMaxSpeed() : 0;
	bIsFalling = Proxy.MovementComponent->IsFalling();
	LocalVerticalVelocityPercentage = LocalVerticalVelocity != 0.0 ? LocalVerticalVelocity/Proxy.Character->GetMovementComponent()->GetMaxSpeed() : 0;
	bIsRunning = Proxy.MovementComponent->bIsSprinting; 
	AimRotation = Proxy.Character->GetBaseAimRotation();
	
	const float CurrentStamina = Proxy.Character->HealthComp->GetStamina();
	// Weapon Wobble
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, AimRotationLastFrame);
	PitchChange = FMath::FInterpTo(PitchChangeLastFrame, FMath::Clamp(DeltaRotator.Pitch, -10.0f, 10.0f), DeltaSeconds, 15.0f); 
	PitchChangeLastFrame = PitchChange;
	YawChange = FMath::FInterpTo(PitchChangeLastFrame, FMath::Clamp(DeltaRotator.Yaw, -10.0f, 10.0f) * -1.0f, DeltaSeconds, 15.0f); 
	YawChangeLastFrame = YawChange;
	AimRotationLastFrame = AimRotation;
	if (CurrentStamina < 50.0f)
	{
		// Define wobble intensity based on stamina
		constexpr float MinWobbleIntensity = 0.3f;   // Minimum wobble intensity
		constexpr float MaxWobbleIntensity = 4.0f;   // Maximum wobble intensity at 0 stamina

		// Scale wobble intensity based on stamina (as stamina decreases, wobble increases)
		const float WobbleIntensity = FMath::Lerp(MaxWobbleIntensity, MinWobbleIntensity, CurrentStamina / 50.0f);

		// Define wobble speed (how fast the wobble oscillates)
		constexpr float WobbleSpeed = 1.5f;

		// Use sinusoidal oscillation to create the wobble effect for both pitch and yaw
		const float WobblePitch = FMath::Sin(Time * WobbleSpeed) * WobbleIntensity;
		const float WobbleYaw = FMath::Cos(Time * WobbleSpeed) * WobbleIntensity;

		// Apply wobble to the pitch and yaw changes
		PitchChange += WobblePitch;
		YawChange += WobbleYaw;
	}
	RotatedPitchYaw = PitchYawMakeRotator();
	bBlendPoseActiveValue =  bShouldPlayFireLoop || bShouldADS;
	Alpha = GetAlpha();
	//Animgraph Can Enter Tranistions
	bIdleToWalkingCanEnter = Speed >=1;
	bNothingToRunStartCanEnter = bIsRunning && !bIsFalling;
	bADSIdleToADSMovingCanEnter = Speed >= 1;
	bRunStartRunEndCanEnter = !bIsRunning && bIsFalling;
	bWalkingEndToWalkingCanEnter = Speed >= 1;
	bWalkingToWalkingEndCanEnter = Speed < 75.0;
	bNothingToFireCanEnter = bShouldPlayFireLoop && !bShouldADS;
	bADSMovingToADSIdleCanEnter = Speed <1;
	bRunEndToNothingCanEnter = !bIsRunning || bIsFalling;
	
}

void USMAnimHandsFP::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	FTimerHandle TimerSleep;
	GetWorld()->GetTimerManager().SetTimer(TimerSleep, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		FrameSkip = 1;
	}), 0.1, false);
}
/*
void USMAnimHandsFP::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	Time = Time + DeltaSeconds;
	CharacterVelocity = GetOwningActor()->GetVelocity().Length();
}*/

FORCEINLINE float USMAnimHandsFP::GetAlpha() const
{
	return FMath::Max(0.1,FMath::Clamp(sin(Time*0.75),0, 0.75)); 
}

FORCEINLINE FRotator USMAnimHandsFP::PitchYawMakeRotator() const
{
	return FRotator(0,YawChange,PitchChange);
}
