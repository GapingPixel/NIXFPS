// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/SMEquippableAbilityBase.h"
#include "SMEquippableAbility.generated.h"


/**
 * 
 */
UCLASS()
class NIX_API USMEquippableAbility : public USMEquippableAbilityBase
{
	GENERATED_BODY()

public:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

	struct FRangedEquippableFiringInput
	{
		// Start of the trace
		FVector StartTrace;

		// End of the trace if aim were perfect
		FVector EndAim;

		// The direction of the trace if aim were perfect
		FVector AimDir;

		// The equippable instance / source of equippable data
		ASMGunBase* EquippableData = nullptr;

		// Can we play bullet FX for hits during this trace
		bool bCanPlayBulletFX = false;

		FRangedEquippableFiringInput()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};

	UFUNCTION(BlueprintCallable)
	void StartRangedTargeting();
	
	void PerformLocalTargeting(TArray<FHitResult>& Hits);
	void TraceBulletsInCartridge(const FRangedEquippableFiringInput& InputData, TArray<FHitResult>& OutHits);
	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits);
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const;
	
	// Determine the trace channel to use for the weapon trace(s)
	virtual ECollisionChannel DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated) const;
	
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);
	
	FTransform GetTargetingTransform(APawn* SourcePawn) const;
	FVector GetEquippableTargetingSourceLocation() const;
	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);
	
	FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad, const float Exponent)
	{
		if (ConeHalfAngleRad > 0.f)
		{
			const float ConeHalfAngleDegrees = FMath::RadiansToDegrees(ConeHalfAngleRad);

			// consider the cone a concatenation of two rotations. one "away" from the center line, and another "around" the circle
			// apply the exponent to the away-from-center rotation. a larger exponent will cluster points more tightly around the center
			const float FromCenter = FMath::Pow(FMath::FRand(), Exponent);
			const float AngleFromCenter = FromCenter * ConeHalfAngleDegrees;
			const float AngleAround = FMath::FRand() * 360.0f;

			FRotator Rot = Dir.Rotation();
			FQuat DirQuat(Rot);
			FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));
			FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));
			FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
			FinalDirectionQuat.Normalize();

			return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
		}
		else
		{
			return Dir.GetSafeNormal();
		}
	};

	// Called when target data is ready
	UFUNCTION(BlueprintImplementableEvent)
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

	public:

	virtual void OnRangedWeaponTargetDataReadyCPP(const FGameplayAbilityTargetDataHandle& TargetData);
private:
	
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
};
