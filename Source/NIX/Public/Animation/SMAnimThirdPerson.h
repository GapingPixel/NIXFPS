// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Kismet/KismetMathLibrary.h"
#include "SMAnimThirdPerson.generated.h"

class UNiagaraSystem;
class USMCharacterMovementComponent;
class ASMPlayerCharacter;
/**
 * 
 */
struct FAnimNodeReference;
struct FAnimUpdateContext;
/**
 * 
 */
USTRUCT()
struct FTPAnimInstanceProxy : public FAnimInstanceProxy
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
class NIX_API USMAnimThirdPerson : public UAnimInstance
{
	GENERATED_BODY()

public:
	
	
    FVector WorldVelocity;
	float Speed;
	FVector2D WorldVelocity2D;

    // Character State Data
    UPROPERTY(BlueprintReadWrite, Category="Character State Data")
    bool bIsMoving;

    UPROPERTY(BlueprintReadWrite, Category="Character State Data")
    float StrafeVelocity;

    UPROPERTY(BlueprintReadWrite, Category="Character State Data")
    float ForwardVelocity;

    // Rotation Data
	FRotator WorldRotation;

    UPROPERTY(BlueprintReadWrite, Category="Rotation Data")
    float AimPitch;

    UPROPERTY(BlueprintReadWrite, Category="Rotation Data")
    float AimYaw;

    UPROPERTY(BlueprintReadWrite, Category="Rotation Data")
    float RootYawOffset;

	float YawDeltaSinceLastUpdate;

	float YawDeltaSpeed;

    // Location Data
	FVector WorldLocation;

    // Turn In Place Data
	float PreviousTurnYawCurveValue;
	float TurnYawCurveValue;
	FFloatSpringState RootYawOffsetSpringState;

    UPROPERTY()
    TObjectPtr<ASMPlayerCharacter> OwningCharacter; // Replace AYourCharacterClass with your character class

    UPROPERTY(BlueprintReadWrite, Category="Turn In Place Data")
    bool bShouldBlendLegs;

    UPROPERTY(BlueprintReadWrite, Category="Turn In Place Data")
    bool bTurnLegsBack;

    UPROPERTY(BlueprintReadWrite, Category="Turn In Place Data")
    bool bIsSprinting;


	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(const float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category="Helper Functions", meta=(Keywords="Cardinal Direction, Angle", CompactNodeTitle="Select Cardinal Direction", ThreadSafe, BlueprintThreadSafe))
	uint8 SelectCardinalDirectionFromAngle(float Angle, float DeadZone, uint8 CurrentDirection, bool bUseCurrentDirection) const;
	
	UFUNCTION(meta=(ThreadSafe, BlueprintThreadSafe))
	void UpdateRotationData(float Delta = 0);

	UFUNCTION(meta=(ThreadSafe, BlueprintThreadSafe))
	void UpdateVelocityData();

	UFUNCTION(meta=(ThreadSafe, BlueprintThreadSafe))
	void UpdateRootYawOffset(float Delta = 0);

	UFUNCTION()
	void SetRootYawOffset(float InRootYawOffset = 0);

	UFUNCTION(meta=(ThreadSafe, BlueprintThreadSafe))
	void ProcessTurnYawCurve();

	UFUNCTION(BlueprintCallable, meta=(ThreadSafe="true", BlueprintThreadSafe))
	void UpdateIdleStateCPP(const  FAnimUpdateContext Context, const  FAnimNodeReference Node);
	
	UFUNCTION(BlueprintPure, meta=(ThreadSafe, BlueprintThreadSafe))
	UAnimInstance* GetMainAnimBPThreadSafe() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ThreadSafe, BlueprintThreadSafe))
	USMAnimThirdPerson* GetSMCharacter();  // Replace `AYourCharacterClass` with your specific character class

	UFUNCTION(BlueprintCallable,  BlueprintPure, meta=(ThreadSafe, BlueprintThreadSafe))
	USMCharacterMovementComponent* GetSMMovementComponent() const;

	void PlayFootstepEffect(bool bIsLeftFoot = false) const;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets")
	TObjectPtr<USoundAttenuation> ImpactAttenuationSettings;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Sounds")
	TObjectPtr<USoundBase> FootSoundDefault;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Sounds")
	TObjectPtr<USoundBase> FootSoundConcrete;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Sounds")
	TObjectPtr<USoundBase> FootSoundDirt;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Sounds")
	TObjectPtr<USoundBase> FootSoundMetal;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Sounds")
	TObjectPtr<USoundBase> FootSoundWood;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Sounds")
	TObjectPtr<USoundBase> FootSoundGlass;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Sounds")
	TObjectPtr<USoundBase> FootSoundFlesh;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Sounds")
	TObjectPtr<USoundBase> FootSoundGrass;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Sounds")
	TObjectPtr<UNiagaraSystem> FootSurfaceDefault;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Effects")
	TObjectPtr<UNiagaraSystem> FootSurfaceAsphalt;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Effects")
	TObjectPtr<UNiagaraSystem> FootSurfaceConcrete;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Effects")
	TObjectPtr<UNiagaraSystem> FootSurfaceDirt;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Effects")
	TObjectPtr<UNiagaraSystem> FootSurfaceMetal;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Effects")
	TObjectPtr<UNiagaraSystem> FootSurfaceWater;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Effects")
	TObjectPtr<UNiagaraSystem> FootSurfaceWood;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Effects")
	TObjectPtr<UNiagaraSystem> FootSurfaceGlass;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Effects")
	TObjectPtr<UNiagaraSystem> FootSurfaceFlesh;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Assets|Effects")
	TObjectPtr<UNiagaraSystem> FootSurfaceGrass;
	
protected:
	UPROPERTY(Transient)
	FTPAnimInstanceProxy Proxy;
	virtual FTPAnimInstanceProxy* CreateAnimInstanceProxy() override {return &Proxy;}
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override {};
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
};
