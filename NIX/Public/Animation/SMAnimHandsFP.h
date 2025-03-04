// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "SMAnimHandsFP.generated.h"

class USMCharacterMovementComponent;
class ASMPlayerCharacter;

struct FAnimNodeReference;
struct FAnimUpdateContext;
/**
 * 
 */
USTRUCT()
struct FASAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

protected:
	virtual void InitializeObjects(UAnimInstance* InAnimInstance) override;
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	virtual void Update(float DeltaSeconds) override;

public:
	UPROPERTY(Transient)
	TWeakObjectPtr<ASMPlayerCharacter> Character;
	UPROPERTY(Transient)
	TWeakObjectPtr<USMCharacterMovementComponent> MovementComponent;
};

UCLASS()
class NIX_API USMAnimHandsFP : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	//virtual void NativeUpdateAnimation(const float DeltaSeconds) override;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe, ThreadSafe))
	float GetAlpha() const;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe, ThreadSafe))
	FRotator PitchYawMakeRotator() const;

	UPROPERTY(BlueprintReadWrite)
	float Alpha;
	UPROPERTY(BlueprintReadWrite)
	FRotator RotatedPitchYaw;

	UPROPERTY(BlueprintReadWrite)
	bool bBlendPoseActiveValue;
	
	UPROPERTY(BlueprintReadWrite)
	float FrameSkip;
	UPROPERTY(BlueprintReadWrite)
	bool bShouldPlayFireLoop;
	UPROPERTY(BlueprintReadWrite)
	float Time;
	
	UPROPERTY(BlueprintReadWrite)
	float LocalForwardVelocityPercentage;
	UPROPERTY(BlueprintReadWrite)
	float LocalStrafeVelocityPercentage;
	UPROPERTY(BlueprintReadWrite)
	bool bIsFalling;
	UPROPERTY(BlueprintReadWrite)
	float LocalVerticalVelocityPercentage;
	UPROPERTY(BlueprintReadWrite)
	bool bIsRunning;
	
	UPROPERTY(BlueprintReadWrite)
	float PitchChange;
	UPROPERTY(BlueprintReadWrite)
	float YawChange;
	
	UPROPERTY(BlueprintReadWrite)
	bool bShouldADS;
	UPROPERTY(BlueprintReadWrite)
	bool bIsADS;

	//CanEnterTransitions
	UPROPERTY(BlueprintReadWrite)
	bool bNothingToRunStartCanEnter;
	UPROPERTY(BlueprintReadWrite)
	bool bADSIdleToADSMovingCanEnter;
	UPROPERTY(BlueprintReadWrite)
	bool bADSMovingToADSIdleCanEnter;
	UPROPERTY(BlueprintReadWrite)
	bool bNothingToFireCanEnter;
	UPROPERTY(BlueprintReadWrite)
	bool bIdleToWalkingCanEnter;
	UPROPERTY(BlueprintReadWrite)
	bool bWalkingToWalkingEndCanEnter;
	UPROPERTY(BlueprintReadWrite)
	bool bWalkingEndToWalkingCanEnter;
	UPROPERTY(BlueprintReadWrite)
	bool bRunStartRunEndCanEnter;
	UPROPERTY(BlueprintReadWrite)
	bool bRunEndToNothingCanEnter;

	UFUNCTION(BlueprintCallable, meta=(ThreadSafe="true", BlueprintThreadSafe))
	void OnADS(const  FAnimUpdateContext& Context, const  FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta=(ThreadSafe="true", BlueprintThreadSafe))
	void OnUnADS(const  FAnimUpdateContext& Context, const  FAnimNodeReference& Node);
	
private:
	float LocalForwardVelocity;
	float LocalStrafeVelocity;
	float LocalVerticalVelocity;

	FRotator AimRotationLastFrame;
	FRotator AimRotation;

	float PitchChangeLastFrame;
	float YawChangeLastFrame;
	
protected:
	UPROPERTY(Transient)
	FASAnimInstanceProxy Proxy;
	virtual FASAnimInstanceProxy* CreateAnimInstanceProxy() override {return &Proxy;}
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override {};
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
};
