// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AttributeSets/SMCombatAttributeSet.h"

#include "GameModes/SMTestGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void USMCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USMCombatAttributeSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
}

void USMCombatAttributeSet::AdjustAttributesForDifficulty()
{
	const ASMTestGameMode* GameMode = Cast<ASMTestGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		float DifficultyMultiplier = 1.0;
		if (GameMode->MatchDifficulty == TEXT("Easy"))
		{
			DifficultyMultiplier = 0.8;
		}
		else if (GameMode->MatchDifficulty == TEXT("Normal"))
		{
			DifficultyMultiplier = 1.0;
		}
		else if (GameMode->MatchDifficulty == TEXT("Hard"))
		{
			DifficultyMultiplier = 1.2;
		}
		else if (GameMode->MatchDifficulty == TEXT("Expert"))
		{
			DifficultyMultiplier = 1.4;
		}
		// Adjust BaseDamage based on Difficulty 
		BaseDamage.SetCurrentValue(BaseDamage.GetCurrentValue() * DifficultyMultiplier);
		BaseDamage.SetBaseValue(BaseDamage.GetBaseValue() * DifficultyMultiplier);
	}
}

void USMCombatAttributeSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMCombatAttributeSet, BaseDamage, OldValue);
}
