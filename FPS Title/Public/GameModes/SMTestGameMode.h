// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SMTestGameMode.generated.h"

class USMWResultsScreen;
class ASMPlayerCharacter;
class ASMPlayerController;
/**
 * 
 */

UCLASS()
class ASMTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASMTestGameMode();

	float StartTime;
	
	float EndTime;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	TSubclassOf<ASMPlayerCharacter> PlayerBP;
	
	FString MatchDifficulty;

	int8 KillGoal;
	int8 CurrentTotalKills;

	void IncrementKillCount();
	
	float GameStartCountDown;

	void LevelComplete();
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float RespawnTime;
	
	void QueueRespawnPlayer(AController* Controller, FVector SpawnLocation, FRotator SpawnRotation) const;

	void BoxObjectiveCompleted();

	bool bAdultContent;

protected:
	virtual void BeginPlay() override;
	void StartLevel();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FString GetFormattedMatchTime() const;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	
};
