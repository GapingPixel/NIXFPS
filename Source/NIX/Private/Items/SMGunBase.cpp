// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SMGunBase.h"

#include "GAS/SMAbilitySystemComponent.h"
#include "Interfaces/SMFirstPersonInterface.h"

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
	CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);

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
	
	const bool bIsAiming = Interface->GetSMAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Aiming")));
	
	const float HeatPerShot = HeatToHeatPerShotCurve.GetRichCurveConst()->Eval(CurrentHeat) * (bIsAiming ? AimSpreadMultiplier : 1.0f);
	CurrentHeat = ClampHeat(CurrentHeat + HeatPerShot);

	const float RecoilHeatPerShot = RecoilHeatToHeatPerShotCurve.GetRichCurveConst()->Eval(CurrentRecoilHeat);
	const float NewRecoilHeat = CurrentRecoilHeat + RecoilHeatPerShot;
	const float MaxRecoil = MaxRecoilHeat != 0.0f ? MaxRecoilHeat : 1000.f;
	CurrentRecoilHeat = FMath::Clamp(NewRecoilHeat, 0.0f, MaxRecoil);
}