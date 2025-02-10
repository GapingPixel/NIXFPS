// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDs/SMWResultsScreen.h"

#include "Components/TextBlock.h"
#include "Player/SMPlayerState.h"

void USMWResultsScreen::SetPlayerStats(const ASMPlayerState* Player1State, const ASMPlayerState* Player2State,
                                       const ASMPlayerState* Player3State, const ASMPlayerState* Player4State, const FString& MatchTime)
{
    
    // Player 1
    if (Player1State)
    {
        Player1->SetText(FText::FromString("Player 1"));
        Player1Kills->SetText(FText::AsNumber(Player1State->KillCount));
        Player1Deaths->SetText(FText::AsNumber(Player1State->DeathCount));
        Player1Assists->SetText(FText::AsNumber(Player1State->FireAssist));
        Player1Damage->SetText(FText::AsNumber(Player1State->DamageDealt));
        Player1Rescues->SetText(FText::AsNumber(Player1State->RescueCount));
        Player1FriendlyFire->SetText(FText::AsNumber(Player1State->FriendlyFire));
    }
    else
    {
        Player1->SetText(FText::FromString(""));
        Player1Kills->SetText(FText::FromString(""));
        Player1Deaths->SetText(FText::FromString(""));
        Player1Assists->SetText(FText::FromString(""));
        Player1Damage->SetText(FText::FromString(""));
        Player1Rescues->SetText(FText::FromString(""));
        Player1FriendlyFire->SetText(FText::FromString(""));
    }

    // Player 2
    if (Player2State)
    {
        Player2->SetText(FText::FromString("Player 2"));
        Player2Kills->SetText(FText::AsNumber(Player2State->KillCount));
        Player2Deaths->SetText(FText::AsNumber(Player2State->DeathCount));
        Player2Assists->SetText(FText::AsNumber(Player2State->FireAssist));
        Player2Damage->SetText(FText::AsNumber(Player2State->DamageDealt));
        Player2Rescues->SetText(FText::AsNumber(Player2State->RescueCount));
        Player2FriendlyFire->SetText(FText::AsNumber(Player2State->FriendlyFire));
    }
    else
    {
        Player2->SetText(FText::FromString(""));
        Player2Kills->SetText(FText::FromString(""));
        Player2Deaths->SetText(FText::FromString(""));
        Player2Assists->SetText(FText::FromString(""));
        Player2Damage->SetText(FText::FromString(""));
        Player2Rescues->SetText(FText::FromString(""));
        Player2FriendlyFire->SetText(FText::FromString(""));
    }

    // Player 3
    if (Player3State)
    {
        Player3->SetText(FText::FromString("Player 3"));
        Player3Kills->SetText(FText::AsNumber(Player3State->KillCount));
        Player3Deaths->SetText(FText::AsNumber(Player3State->DeathCount));
        Player3Assists->SetText(FText::AsNumber(Player3State->FireAssist));
        Player3Damage->SetText(FText::AsNumber(Player3State->DamageDealt));
        Player3Rescues->SetText(FText::AsNumber(Player3State->RescueCount));
        Player3FriendlyFire->SetText(FText::AsNumber(Player3State->FriendlyFire));
    }
    else
    {
        Player3->SetText(FText::FromString(""));
        Player3Kills->SetText(FText::FromString(""));
        Player3Deaths->SetText(FText::FromString(""));
        Player3Assists->SetText(FText::FromString(""));
        Player3Damage->SetText(FText::FromString(""));
        Player3Rescues->SetText(FText::FromString(""));
        Player3FriendlyFire->SetText(FText::FromString(""));
    }

    // Player 4
    if (Player4State)
    {
        Player4->SetText(FText::FromString("Player 4"));
        Player4Kills->SetText(FText::AsNumber(Player4State->KillCount));
        Player4Deaths->SetText(FText::AsNumber(Player4State->DeathCount));
        Player4Assists->SetText(FText::AsNumber(Player4State->FireAssist));
        Player4Damage->SetText(FText::AsNumber(Player4State->DamageDealt));
        Player4Rescues->SetText(FText::AsNumber(Player4State->RescueCount));
        Player4FriendlyFire->SetText(FText::AsNumber(Player4State->FriendlyFire));
    }
    else
    {
        Player4->SetText(FText::FromString(""));
        Player4Kills->SetText(FText::FromString(""));
        Player4Deaths->SetText(FText::FromString(""));
        Player4Assists->SetText(FText::FromString(""));
        Player4Damage->SetText(FText::FromString(""));
        Player4Rescues->SetText(FText::FromString(""));
        Player4FriendlyFire->SetText(FText::FromString(""));
    }
    TotalTime->SetText(FText::FromString(MatchTime));
    //TotalTime->SetText(FText::AsNumber(Cast<ASMTestGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->GameTimer));
}
