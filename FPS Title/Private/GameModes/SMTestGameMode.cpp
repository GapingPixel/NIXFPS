// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/SMTestGameMode.h"


#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Online/MultiplayerSessionsSubsystem.h"
#include "Player/SMPlayerController.h"
#include "Possessables/SMPlayerCharacter.h"


ASMTestGameMode::ASMTestGameMode()
{
	RespawnTime = 15.0;
	GameStartCountDown = 4.0;
	KillGoal = 1;
	MatchDifficulty = TEXT("Normal");
}

void ASMTestGameMode::BeginPlay()
{
	Super::BeginPlay();
	const UWorld* W = GetWorld();
	FString CurrentLevelName = GetWorld()->GetMapName();
	// Unreal adds a prefix like "UEDPIE_" in Play-in-Editor (PIE) mode
	// so we can clean that up for comparison.
	CurrentLevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	if (CurrentLevelName == "GameStartupMap")
	{
		return;
	}
	
	FTimerHandle TimerSleep;
	GetWorld()->GetTimerManager().SetTimer(TimerSleep, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		StartLevel();
		GEngine->AddOnScreenDebugMessage(-1, GameStartCountDown, FColor::Yellow, TEXT("GAME START !"));
	}), GameStartCountDown, false);

	if (HasAuthority())
	{
		for (FConstPlayerControllerIterator It = W->GetPlayerControllerIterator(); It; ++It)
		{
			const ASMPlayerController* PlayerController = Cast<ASMPlayerController>(It->Get());
			if (PlayerController)
			{
				Cast<ASMPlayerCharacter>(PlayerController->GetPawn())->bIsDowned = false;
			}
		}
	}
}

void ASMTestGameMode::StartLevel()
{
	StartTime = GetWorld()->GetTimeSeconds();
	
	const UWorld* World = GetWorld();
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const ASMPlayerController* PlayerController = Cast<ASMPlayerController>(It->Get());
		if (PlayerController)
		{
			Cast<ASMPlayerCharacter>(PlayerController->GetPawn())->bIsDowned = false;
			
			if (MatchDifficulty == TEXT("Easy"))
			{
				Cast<ASMPlayerCharacter>(PlayerController->GetCharacter())->MaxPlayerWounds = 5;
			}
			else if (MatchDifficulty == TEXT("Normal"))
			{
				Cast<ASMPlayerCharacter>(PlayerController->GetCharacter())->MaxPlayerWounds = 3;
			}
			else if (MatchDifficulty == TEXT("Hard"))
			{
				Cast<ASMPlayerCharacter>(PlayerController->GetCharacter())->MaxPlayerWounds = 2;
			}
			else if (MatchDifficulty == TEXT("Expert"))
			{
				Cast<ASMPlayerCharacter>(PlayerController->GetCharacter())->MaxPlayerWounds = 1;
			}
			//EnableInput(PlayerController);
			//PlayerController->SetInputMode(FInputModeGameOnly());
			//PlayerController->SetIgnoreLookInput(false);
			/*ASMPlayerController* MyController = Cast<ASMPlayerController>(PlayerController);
			if (MyController)
			{
				MyController->SetupInputComponent();
			}*/
			//UE_LOG(LogTemp, Warning, TEXT("Enabled input for player controller: %s"), *PlayerController->GetName());
		}
	}
}

void ASMTestGameMode::QueueRespawnPlayer(AController* Controller, FVector SpawnLocation, FRotator SpawnRotation) const
{
	FTimerHandle TimerRespawn;
	GetWorld()->GetTimerManager().SetTimer(TimerRespawn, FTimerDelegate::CreateWeakLambda(this, [this, Controller, SpawnLocation, SpawnRotation]()
	{
		const FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation);
		ASMPlayerCharacter* NewPlayer = GetWorld()->SpawnActor<ASMPlayerCharacter>(PlayerBP, SpawnTransform);
		Controller->Possess(NewPlayer);
		FTimerHandle TimerNextTick;
		GetWorld()->GetTimerManager().SetTimer(TimerNextTick, FTimerDelegate::CreateWeakLambda(this, [this, Controller]()
		{
			Cast<ASMPlayerController>(Controller)->Client_UpdateHUDReferences();
		}), GetWorld()->DeltaTimeSeconds, false);
	}), RespawnTime, false);
}


void ASMTestGameMode::IncrementKillCount()
{
	CurrentTotalKills++;
	if (CurrentTotalKills >= KillGoal)
	{
		CurrentTotalKills = 0;
		LevelComplete();
	}
}

void ASMTestGameMode::BoxObjectiveCompleted()
{
	LevelComplete();
	//Do Something 
}

void ASMTestGameMode::LevelComplete() 
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("LEVEL COMPLETE"));
	EndTime = GetWorld()->GetTimeSeconds() - StartTime;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ASMPlayerController* PlayerController = Cast<ASMPlayerController>(Iterator->Get());
		if (PlayerController)
		{
			// Call the client RPC to update the widget for this client
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, PlayerController]()
			{
				PlayerController->Client_UpdateWidgetWithPlayerStats(GetFormattedMatchTime());
			}), 1, false);
		}
	}
}

 FString ASMTestGameMode::GetFormattedMatchTime() const
{
	const uint32 Minutes = FMath::FloorToInt(EndTime / 60);
	const uint32 Seconds = FMath::FloorToInt(EndTime) % 60;
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}

void ASMTestGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);

		if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;

				FString MatchType = Subsystem->DesiredMatchType;
				World->ServerTravel(FString("/Game/Maps/Lobby?listen"));
				/*if (MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
				}
				else if (MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/Maps/Teams?listen"));
				}
				else if (MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/CaptureTheFlag?listen"));
				}*/
			}
		}
	}
}

void ASMTestGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}



