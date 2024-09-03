// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/SMEquippableBase.h"
#include "SMGunBase.generated.h"

/**
 * 
 */
UCLASS()
class ASMGunBase : public ASMEquippableBase
{
	GENERATED_BODY()

public:
	ASMGunBase();
	
	virtual void Tick(float DeltaSeconds) override;

	// ~Start of ASMItemBase Interface 
	virtual void OnPickUp(USMEquippableInventoryComponent* inventory) override;
	// ~End of ASMItemBase Interface 

public:

	/* Empty Reloads
	***********************************************************************************/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Animation")
	FEquippableAnimCluster EmptyReloads;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Animation")
	FEquippableAnimCluster LastFire;
	
	/* Gun Values
	***********************************************************************************/

protected:

	// Spread multiplier when aiming. Lower values means less spread. 0 means no spread when aiming.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Gun Spread", meta=(ClampMin=0.0f, ClampMax=1.0f))
	float AimSpreadMultiplier = 0.25f;
	
	float CurrentHeat = 0.0f;
	float CurrentRecoilHeat = 0.0f;
    
	float CurrentSpreadAngle = 0.0f;

	/* Curves
	***********************************************************************************/

	// A curve that maps the heat to the spread angle
	// The X range of this curve typically sets the min/max heat range of the weapon
	// The Y range of this curve is used to define the min and maximum spread angle
	UPROPERTY(EditAnywhere, Category = "Equippable|Gun Spread")
	FRuntimeFloatCurve HeatToSpreadCurve;
	
	// A curve that maps the current heat to the heat cooldown rate per second
	// This is typically a flat curve with a single data point indicating how fast the heat
	// wears off, but can be other shapes to do things like punish overheating by slowing down
	// recovery at high heat.
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Gun Spread")
	FRuntimeFloatCurve HeatToCooldownPerSecondCurve;

	// A curve that maps the current heat to the amount a single shot will further 'heat up'
	// This is typically a flat curve with a single data point indicating how much heat a shot adds,
	// but can be other shapes to do things like punish overheating by adding progressively more heat.
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Gun Spread")
	FRuntimeFloatCurve HeatToHeatPerShotCurve;

	
	// A curve that maps the current heat to the heat cooldown rate per second
	// This is typically a flat curve with a single data point indicating how fast the heat
	// wears off, but can be other shapes to do things like punish overheating by slowing down
	// recovery at high heat.
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Recoil")
	FRuntimeFloatCurve RecoilHeatToCooldownPerSecondCurve;

	// A curve that maps the current heat to the amount a single shot will further 'heat up'
	// This is typically a flat curve with a single data point indicating how much heat a shot adds,
	// but can be other shapes to do things like punish overheating by adding progressively more heat.
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Recoil")
	FRuntimeFloatCurve RecoilHeatToHeatPerShotCurve;
	
	/* Spread and Heat
	***********************************************************************************/

protected:
	
	void ComputeHeatRange(float& MinHeat, float& MaxHeat);

	void UpdateSpread(float deltaSeconds);

	virtual float GetRecoilHeatMultiplier() override { return GetCurrentRecoilHeat(); };

public:

	FORCEINLINE float GetCurrentRecoilHeat() const { return CurrentRecoilHeat; }
	
	/* Other (uncategorized)
	***********************************************************************************/

protected:
	
#if WITH_EDITORONLY_DATA

	UPROPERTY(EditDefaultsOnly, Category = "Debug Equippable")
	bool bDebugSpreadValues = false;

	// @TODO: wht are these values not updating real time in editor???
	
	UPROPERTY(VisibleAnywhere, Category = "Debug Equippable")
	float Debug_CurrentHeat = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Debug Equippable")
	float Debug_CurrentSpreadAngle = 0.0f;

#endif
	
public:

	void AddSpread();

	int32 GetBulletsPerCartridge() const { return BulletsPerCartridge; }
	
	/** Returns the current spread angle (in degrees, diametrical) */
	float GetCalculatedSpreadAngle() const
	{
		return CurrentSpreadAngle;
	}
	
	float GetMaxDamageRange() const
	{
		// @TODO: expose
		return 99999.f;
	}

private:

	inline float ClampHeat(float NewHeat)
	{
		float MinHeat;
		float MaxHeat;
		ComputeHeatRange(/*out*/ MinHeat, /*out*/ MaxHeat);

		return FMath::Clamp(NewHeat, MinHeat, MaxHeat);
	}
};
