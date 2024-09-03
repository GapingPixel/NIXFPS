// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/SMAnimHandsFP.h"
#include "Components/SMCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Possessables/SMPlayerCharacter.h"

void FASAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);
	Owner = InAnimInstance->TryGetPawnOwner();
	if (!Owner) return;

	Character = Cast<ASMPlayerCharacter>(Owner);
	MovementComponent = Cast<USMCharacterMovementComponent>(Owner->GetMovementComponent());
}

void FASAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
}

void FASAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}

void USMAnimHandsFP::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!Proxy.Owner) return;
	
	const FVector Velocity = Proxy.Character->GetVelocity();
	//float Speed = Velocity.Length();
	const FVector UnrotatedVector = Proxy.Character->GetActorRotation().UnrotateVector(Velocity);
	LocalForwardVelocity = UnrotatedVector.X;
	LocalStrafeVelocity = UnrotatedVector.Y;
	LocalVerticalVelocity = UnrotatedVector.Z;

	LocalForwardVelocityPercentage = LocalForwardVelocity != 0.0 ? LocalForwardVelocity/Proxy.Character->GetMovementComponent()->GetMaxSpeed() : 0;
	LocalStrafeVelocityPercentage = LocalStrafeVelocity != 0.0 ? LocalStrafeVelocity/Proxy.Character->GetMovementComponent()->GetMaxSpeed() : 0;
	bIsFalling = Proxy.MovementComponent->IsFalling();
	LocalVerticalVelocityPercentage = LocalVerticalVelocity != 0.0 ? LocalVerticalVelocity/Proxy.Character->GetMovementComponent()->GetMaxSpeed() : 0;
	bIsRunning = Proxy.MovementComponent->bIsSprinting; 
	AimRotation = Proxy.Character->GetBaseAimRotation();

	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, AimRotationLastFrame);
	PitchChange = FMath::FInterpTo(PitchChangeLastFrame,  FMath::Clamp(DeltaRotator.Pitch, -10.0f, 10.0f), DeltaSeconds, 15.0f); 
	PitchChangeLastFrame = PitchChange;
	YawChange = FMath::FInterpTo(PitchChangeLastFrame,  FMath::Clamp(DeltaRotator.Yaw, -10.0f, 10.0f)*-1.0, DeltaSeconds, 15.0f); 
	YawChangeLastFrame = YawChange;
	AimRotationLastFrame = AimRotation;
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

void USMAnimHandsFP::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	Time = Time + DeltaSeconds;
	CharacterVelocity = GetOwningActor()->GetVelocity().Length();
}

float USMAnimHandsFP::GetAlpha() const
{
	return FMath::Max(0.1,FMath::Clamp(sin(Time*0.75),0, 0.75)); 
}
