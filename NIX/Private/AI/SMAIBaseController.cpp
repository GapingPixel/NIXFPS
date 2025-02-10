// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SMAIBaseController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Possessables/SMBaseHorror.h"
#include "Possessables/SMPlayerCharacter.h"

ASMAIBaseController::ASMAIBaseController()
{
	//AIPerceptionComponent = ;
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent")));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.0f;  // Adjust this based on the desired vision range
	SightConfig->LoseSightRadius = SightConfig->SightRadius + 700.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	// Configure the hearing sense
	HearingConfig->HearingRange = 2000.0f; // Set range to hear sounds
	HearingConfig->DetectionByAffiliation.bDetectEnemies = false;  // Adjust as necessary
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	
	// Register the hearing sense
	//AIPerceptionComponent->ConfigureSense(*HearingConfig);
	//AAIController::GetPerceptionComponent()->SetDominantSense(UAISense_Hearing::StaticClass());
	AAIController::GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &ASMAIBaseController::OnPerceptionUpdated);
	AAIController::GetPerceptionComponent()->ConfigureSense(*HearingConfig);
	AAIController::GetPerceptionComponent()->ConfigureSense(*SightConfig);
	AAIController::GetPerceptionComponent()->Activate();
	// Bind the perception update event
	
	
}

void ASMAIBaseController::BeginPlay()
{
	Super::BeginPlay();
	
	//AIPerceptionComponent->Activate();
	//AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ASMAIBaseController::OnPerceptionUpdated);
}

void ASMAIBaseController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	// Loop through all actors that triggered a perception update
	
	for (AActor* Actor : UpdatedActors)
	{
		if (!Cast<ASMPlayerCharacter>(Actor))
		{
			break;
		}
		FActorPerceptionBlueprintInfo PerceptionInfo;
		PerceptionComponent->GetActorsPerception(Actor, PerceptionInfo);

		// Check each sensed information for the actor
		for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
		{
			if (ASMBaseHorror* Horror = Cast<ASMBaseHorror>(GetPawn()))
			{
				if (Stimulus.IsActive())
				{
					if (Stimulus.Type == SightConfig->GetSenseID())
					{
						GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, TEXT("Detected by Sight!"));
						Horror->bSeingPlayer = true; 
					}
					else if (Stimulus.Type == HearingConfig->GetSenseID())
					{
						GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Detected by Hearing!"));
					
						
						GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("Actor Detected: %s"), *Actor->GetName()));
						// Activate a state or move based on detection (sight/hearing)
						Horror->ActivateNewStateTree(Horror->STMelee);
						MoveToActor(Actor);
						
					}
				} else
				{
					if (Stimulus.Type == SightConfig->GetSenseID())
					{
						Horror->bSeingPlayer = false;
						GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Not seing player"));
					}
				}
			}
		}
	}
}
