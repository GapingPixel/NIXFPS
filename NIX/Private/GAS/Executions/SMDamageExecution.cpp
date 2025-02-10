// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Executions/SMDamageExecution.h"

#include "AbilitySystemComponent.h"
#include "GAS/AttributeSets/SMCombatAttributeSet.h"
#include "GAS/AttributeSets/SMHealthAttributeSet.h"

struct FDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition HealthDef;
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;

	FDamageStatics()
	{
		HealthDef = FGameplayEffectAttributeCaptureDefinition(USMHealthAttributeSet::GetHealthAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
		BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(USMCombatAttributeSet::GetBaseDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}


USMDamageExecution::USMDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().HealthDef);
	RelevantAttributesToCapture.Add(DamageStatics().BaseDamageDef);

#if WITH_EDITORONLY_DATA
	InvalidScopedModifierAttributes.Add(DamageStatics().HealthDef);
#endif // #if WITH_EDITORONLY_DATA
}

void USMDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE

	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->GetAvatarActor() : nullptr;
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float CurrentHealth = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().HealthDef, EvaluationParameters, CurrentHealth);

	float BaseDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDamageDef, EvaluationParameters, BaseDamage);

	// This clamp prevents us from doing more damage than there is health available.
	const float DamageDone = FMath::Clamp(BaseDamage, 0.0f, CurrentHealth);

	// @TODO: add distance falloff, different damage for body parts, etc (look at lyra example and gas shooter to see how to do this)
	
	if (DamageDone > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(USMHealthAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -DamageDone));
	}
	
#endif
}
