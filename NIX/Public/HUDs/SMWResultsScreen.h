// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMWResultsScreen.generated.h"

class ASMPlayerState;
/**
 * 
 */
UCLASS(meta=(DisableNativeTick))
class NIX_API USMWResultsScreen : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
    class UTextBlock* Player1;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player2;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player3;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player4;

    // Kills Text Blocks
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player1Kills;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player2Kills;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player3Kills;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player4Kills;

    // Deaths Text Blocks
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player1Deaths;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player2Deaths;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player3Deaths;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player4Deaths;

    // Assists Text Blocks
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player1Assists;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player2Assists;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player3Assists;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player4Assists;

    // Damage Text Blocks
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player1Damage;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player2Damage;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player3Damage;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player4Damage;

    // Rescues Text Blocks
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player1Rescues;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player2Rescues;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player3Rescues;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player4Rescues;

    // Friendly Fire Text Blocks
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player1FriendlyFire;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player2FriendlyFire;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player3FriendlyFire;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Player4FriendlyFire;

    // Total Time Text Block
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TotalTime;
public:
	void SetPlayerStats(const ASMPlayerState* Player1State, const ASMPlayerState* Player2State,
									   const ASMPlayerState* Player3State, const ASMPlayerState* Player4State, const FString& MatchTime);
	
};
