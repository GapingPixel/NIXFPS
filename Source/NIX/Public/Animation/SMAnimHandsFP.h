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
	TObjectPtr<APawn> Owner;
	UPROPERTY(Transient)
	TObjectPtr<ASMPlayerCharacter> Character;
	UPROPERTY(Transient)
	TObjectPtr<USMCharacterMovementComponent> MovementComponent;
};

UCLASS()
class NIX_API USMAnimHandsFP : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(const float DeltaSeconds) override;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe, ThreadSafe))
	float GetAlpha() const;

	UPROPERTY(BlueprintReadWrite)
	float CharacterVelocity;
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
