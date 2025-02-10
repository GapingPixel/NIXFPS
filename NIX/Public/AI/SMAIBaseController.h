// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SMAIBaseController.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
/**
 * 
 */
UCLASS()
class NIX_API ASMAIBaseController : public AAIController
{
	GENERATED_BODY()

	ASMAIBaseController();
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	virtual void BeginPlay() override;
	// Handle perception updates
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
};
