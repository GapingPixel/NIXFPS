// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SMPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameStateBase.h"
#include "HUDs/SMWPlayerHUD.h"
#include "HUDs/SMWResultsScreen.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SMPlayerState.h"
#include "Possessables/SMBaseHorror.h"
#include "Possessables/SMPlayerCharacter.h"

class USMWResultsScreen;

void ASMPlayerController::Client_UpdateHUDReferences_Implementation()
{
	PlayerHUDInstance->SetUpReferences(Cast<ASMPlayerCharacter>(GetPawn()));
}

void ASMPlayerController::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<ASMPlayerCharacter>(GetPawn());
	//PlayerAttributes = PlayerCharacter->HealthComp;
	//AbilitySystemComponent = PlayerCharacter->GetSMAbilitySystemComponent();
	if (IsLocalController())
	{
		PlayerHUDInstance = CreateWidget<USMWPlayerHUD>(GetWorld(), PlayerHUDReference);
		PlayerHUDInstance->AddToViewport();
		//DisableInput(this);
		//EnableInput(this);
	}
	/*if (HasAuthority())
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (IsAnyHorrorObjectInLineOfSight())
			{
				const FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
				const FGameplayEffectSpecHandle SanitySpecHandle = AbilitySystemComponent->MakeOutgoingSpec(SanityGE, 1, EffectContext);
				SanitySpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.DamageToApply")), SeeHorrorSanityCost);
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SanitySpecHandle.Data.Get());
			}
		}), 0.3, true);
	}*/
	
	/*if (GEngine && GEngine->GameViewport)
	{
		
		SAssignNew(DialogWidget, SDialogWidget);
		GEngine->GameViewport->AddViewportWidgetContent(
			SNew(SWeakWidget).PossiblyNullContent(DialogWidget.ToSharedRef())
		);
	}*/
	
}

bool ASMPlayerController::IsAnyHorrorObjectInLineOfSight() const
{
	TArray<AActor*> HorrorActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASMBaseHorror::StaticClass(), HorrorActors);
	for (const AActor* Actor : HorrorActors)
	{
		if (Actor)
		{
			if (LineOfSightTo(Actor))
			{
				return true;
			}
		}
	}
	return false;
}

void ASMPlayerController::ShowResultsScreen()
{
	if (IsLocalController())
	{
		PlayerHUDInstance->RemoveFromParent();
		ResultsScreenInstance = CreateWidget<USMWResultsScreen>(GetWorld(), ResultsScreenReference);
		ResultsScreenInstance->AddToViewport();
	}
}

void ASMPlayerController::Client_UpdateWidgetWithPlayerStats_Implementation(const FString& MatchTime)
{
	if (IsLocalController())
	{
		PlayerHUDInstance->RemoveFromParent();
		ResultsScreenInstance = CreateWidget<USMWResultsScreen>(GetWorld(), ResultsScreenReference);
		ResultsScreenInstance->AddToViewport();
		TArray<APlayerState*> PlayerStates = GetWorld()->GetGameState()->PlayerArray;

		ASMPlayerState* Player1State = nullptr;
		ASMPlayerState* Player2State = nullptr;
		ASMPlayerState* Player3State = nullptr;
		ASMPlayerState* Player4State = nullptr;

		// Iterate over the player states and assign them to the appropriate variables
		for (int32 i = 0; i < PlayerStates.Num(); ++i)
		{
			ASMPlayerState* PS = Cast<ASMPlayerState>(PlayerStates[i]);
			if (PS)
			{
				switch (i)
				{
				case 0:
					Player1State = PS;
					break;
				case 1:
					Player2State = PS;
					break;
				case 2:
					Player3State = PS;
					break;
				case 3:
					Player4State = PS;
					break;
				default:
					break;
				}
			}
		}
		ResultsScreenInstance->SetPlayerStats(Player1State, Player2State, Player3State, Player4State, MatchTime);
	}
}

void ASMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent)
	{
		InputComponent->BindAxis("LookUp", this, &ASMPlayerController::LookUp);
		InputComponent->BindAxis("Turn", this, &ASMPlayerController::Turn);
	}
}

void ASMPlayerController::ReactivateInput_Implementation()
{
	if (IsLocalController())
	{
		EnableInput(this);
	}
}

void ASMPlayerController::ApplyRecoil(const FVector2D& RecoilAmount, const float RecoilSpeed, const float RecoilResetSpeed)
{
	if (IsLocalPlayerController())
	{
		RecoilBumpAmount += RecoilAmount;
		RecoilResetAmount += -RecoilAmount;

		CurrentRecoilSpeed = RecoilSpeed;
		CurrentRecoilResetSpeed = RecoilResetSpeed;

		LastRecoilTime = GetWorld()->GetTimeSeconds();
	}
}

void ASMPlayerController::LookUp(const float Value)
{
	if (!FMath::IsNearlyZero(RecoilResetAmount.Y, 0.01f))
	{
		if (RecoilResetAmount.Y > 0.f && Value > 0.f)
		{
			RecoilResetAmount.Y = FMath::Max(0.f, RecoilResetAmount.Y);
		}
		else if (RecoilResetAmount.Y < 0.f && Value < 0.f)
		{
			RecoilResetAmount.Y = FMath::Min(0.f, RecoilResetAmount.Y);
		}
	}
	// Apply the recoil over several frames
	if (!FMath::IsNearlyZero(RecoilBumpAmount.Y, 0.01f))
	{
		const FVector2D LastCurrentRecoil = RecoilBumpAmount;
		RecoilBumpAmount.Y = FMath::FInterpTo(RecoilBumpAmount.Y, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilSpeed);
		AddPitchInput(LastCurrentRecoil.Y - RecoilBumpAmount.Y);
	}
	// Slowly reset back to center after recoil is processed
	const FVector2D LastRecoilResetAmount = RecoilResetAmount;
	RecoilResetAmount.Y = FMath::FInterpTo(RecoilResetAmount.Y, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilResetSpeed);
	AddPitchInput(LastRecoilResetAmount.Y - RecoilResetAmount.Y);
	AddPitchInput(Value);
}

void ASMPlayerController::Turn(const float Value)
{
	// If the player has moved their camera to compensate for recoil we need this to cancel out the recoil reset effect
	if (!FMath::IsNearlyZero(RecoilResetAmount.X, 0.01f))
	{
		if (RecoilResetAmount.X > 0.f && Value > 0.f)
		{
			RecoilResetAmount.X = FMath::Max(0.f, RecoilResetAmount.X);
		}
		else if (RecoilResetAmount.X < 0.f && Value < 0.f)
		{
			RecoilResetAmount.X = FMath::Min(0.f, RecoilResetAmount.X);
		}
	}
	// Apply the recoil over several frames
	if (!FMath::IsNearlyZero(RecoilBumpAmount.X, 0.1f))
	{
		const FVector2D LastCurrentRecoil = RecoilBumpAmount;
		RecoilBumpAmount.X = FMath::FInterpTo(RecoilBumpAmount.X, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilSpeed);
		AddYawInput(LastCurrentRecoil.X - RecoilBumpAmount.X);
	}
	// Slowly reset back to center after recoil is processed
	const FVector2D LastRecoilResetAmount = RecoilResetAmount;
	RecoilResetAmount.X = FMath::FInterpTo(RecoilResetAmount.X, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilResetSpeed);
	AddYawInput(LastRecoilResetAmount.X - RecoilResetAmount.X);
	AddYawInput(Value);
}

void ASMPlayerController::Look(const FInputActionValue& Value)
{
	FVector2d Input = Value.Get<FVector2D>();
	Input.Y *= -1; 
	//Input = FVector2d(Input.X, Input.Y*-1);
	if (!FMath::IsNearlyZero(RecoilResetAmount.Y, 0.01f))
	{
		if (RecoilResetAmount.Y > 0.f && Input.Y < 0.f)
		{
			RecoilResetAmount.Y = FMath::Max(0.f, RecoilResetAmount.Y);
		}
		else if (RecoilResetAmount.Y < 0.f && Input.Y < 0.f)
		{
			RecoilResetAmount.Y = FMath::Min(0.f, RecoilResetAmount.Y);
		}
	}
	if (!FMath::IsNearlyZero(RecoilResetAmount.X, 0.01f))
	{
		if (RecoilResetAmount.X > 0.f && Input.X > 0.f)
		{
			RecoilResetAmount.X = FMath::Max(0.f, RecoilResetAmount.X);
		}
		else if (RecoilResetAmount.X < 0.f && Input.X < 0.f)
		{
			RecoilResetAmount.X = FMath::Min(0.f, RecoilResetAmount.X);
		}
		
	}
	if (Input.X != 0.0f)
	{
		AddYawInput(Input.X);
	}
	if (Input.Y != 0.0f)
	{
		AddPitchInput(Input.Y);
	}
}



