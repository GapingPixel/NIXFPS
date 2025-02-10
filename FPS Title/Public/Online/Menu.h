// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"
class UCheckBox;
class UButton;
class UMultiplayerSessionsSubsystem;
class ASMTestGameMode;
/**
 * 
 */
UCLASS(meta=(DisableNativeTick))
class UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("Normal")), FString LobbyPath = FString(TEXT("/Game/Maps/Lobby")));

protected:

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	//
	// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful) const;
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result) const;
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:

	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* EasyCheckbox;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* NormalCheckbox;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* HardCheckbox;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* ExpertCheckbox;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* AdultContentCheckbox;

	UFUNCTION()
	void EasyCheckBoxStateChanged(bool bIsChecked);

	UFUNCTION()
	void NormalCheckBoxStateChanged(bool bIsChecked);
	UFUNCTION()
	void HardCheckBoxStateChanged(bool bIsChecked);

	UFUNCTION()
	void ExpertCheckBoxStateChanged(bool bIsChecked);

	UFUNCTION()
	void AdultContentCheckBoxStateChanged(bool bIsChecked);
	
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	UFUNCTION()
	void ExitButtonClicked();

	void MenuTearDown();

	// The subsystem designed to handle all online session functionality
	UPROPERTY()
	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections{4};

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchDifficulty{TEXT("Normal")};

	FString PathToLobby{TEXT("")};
};
