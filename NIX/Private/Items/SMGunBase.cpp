// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SMGunBase.h"

#include "Components/SMHealthComponent.h"
#include "Components/SMPlayerAttributesComponent.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "Interfaces/SMFirstPersonInterface.h"
#include "Possessables/SMPlayerCharacter.h"

ASMGunBase::ASMGunBase()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ASMGunBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateSpread(DeltaSeconds);

#if WITH_EDITOR
	if (bDebugSpreadValues && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString("CurrentHeat: ") + FString::SanitizeFloat(CurrentHeat));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString("CurrentSpreadAngle: ") + FString::SanitizeFloat(CurrentSpreadAngle));
	}
#endif
	
}

void ASMGunBase::OnPickUp(USMEquippableInventoryComponent* inventory)
{
	Super::OnPickUp(inventory);

	// Start heat in the middle
	float MinHeatRange;
	float MaxHeatRange;
	ComputeHeatRange(/*out*/ MinHeatRange, /*out*/ MaxHeatRange);
	//CurrentHeat = (MinHeatRange + MaxHeatRange) * 0.5f;
}

void ASMGunBase::ComputeHeatRange(float& MinHeat, float& MaxHeat)
{
	float Min1;
	float Max1;
	HeatToHeatPerShotCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min1, /*out*/ Max1);

	float Min2;
	float Max2;
	HeatToCooldownPerSecondCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min2, /*out*/ Max2);

	float Min3;
	float Max3;
	HeatToSpreadCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min3, /*out*/ Max3);

	MinHeat = FMath::Min(FMath::Min(Min1, Min2), Min3);
	MaxHeat = FMath::Max(FMath::Max(Max1, Max2), Max3);
}

void ASMGunBase::UpdateSpread(float deltaSeconds)
{
	const float CooldownRate = HeatToCooldownPerSecondCurve.GetRichCurveConst()->Eval(CurrentHeat);
	CurrentHeat = ClampHeat(CurrentHeat - (CooldownRate * deltaSeconds));
	float CurrentStamina = 100;
	if (GetOwner()) {
		CurrentStamina = Cast<ASMPlayerCharacter>(GetOwner())->HealthComp->GetStamina();
	}
	float StaminaBasedSpreadMultiplier = 1.0f;
	if (CurrentStamina < 50.0f)
	{
		// Higher multiplier as stamina approaches 0
		constexpr float MinMultiplier = 1.0f;  // No additional spread at 50 stamina
		constexpr float MaxMultiplier = 2.0f;  // Max 2x spread when stamina is 0
		StaminaBasedSpreadMultiplier = FMath::Lerp(MaxMultiplier, MinMultiplier, CurrentStamina / 50.0f);
	}
	CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat) * StaminaBasedSpreadMultiplier;
	
	const float RecoilCooldownRate = RecoilHeatToCooldownPerSecondCurve.GetRichCurveConst()->Eval(CurrentRecoilHeat);
	CurrentRecoilHeat = CurrentRecoilHeat - (RecoilCooldownRate * deltaSeconds);

#if WITH_EDITOR
	Debug_CurrentHeat = CurrentHeat;
	Debug_CurrentSpreadAngle = CurrentSpreadAngle;
#endif
}

void ASMGunBase::AddSpread()
{
	const ISMFirstPersonInterface* Interface = GetOwnerFirstPersonInterface();
	check(Interface)
	float CurrentStamina = 100;
	if (GetOwner())
	{
		CurrentStamina = Cast<ASMPlayerCharacter>(GetOwner())->HealthComp->GetStamina();
	}
	const bool bIsAiming = Interface->GetSMAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Aiming")));
	
	const float HeatPerShot = HeatToHeatPerShotCurve.GetRichCurveConst()->Eval(CurrentHeat) * (bIsAiming ? AimSpreadMultiplier : 1.0f);
	float StaminaBasedSpreadMultiplier = 1.0f;
	if (CurrentStamina < 50.0f)
	{
		// The closer stamina is to 0, the higher the spread multiplier
		constexpr float MinMultiplier = 1.0f;    // No additional spread at 50 stamina
		constexpr float MaxMultiplier = 3.0f;    // Max 3x spread when stamina is 0
		StaminaBasedSpreadMultiplier = FMath::Lerp(MaxMultiplier, MinMultiplier, CurrentStamina / 50.0f);
	}
	CurrentHeat = ClampHeat(CurrentHeat + (HeatPerShot * StaminaBasedSpreadMultiplier));

	const float RecoilHeatPerShot = RecoilHeatToHeatPerShotCurve.GetRichCurveConst()->Eval(CurrentRecoilHeat);
	const float NewRecoilHeat = CurrentRecoilHeat + RecoilHeatPerShot;
	const float MaxRecoil = MaxRecoilHeat != 0.0f ? MaxRecoilHeat : 1000.f;
	CurrentRecoilHeat = FMath::Clamp(NewRecoilHeat, 0.0f, MaxRecoil);
}