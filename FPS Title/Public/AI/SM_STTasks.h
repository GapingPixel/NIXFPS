// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "SM_STTasks.generated.h"

class ASMSplineActor;
/**
 * 
 */
UCLASS()
class NIX_API USM_STTDestroy : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TObjectPtr<AActor> Actor;
	
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
};

UCLASS()
class NIX_API USM_STTMoveAlongSpline : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	float MovementSpeed = 5.0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ASMSplineActor> SplineActor;

private:
	float Distance;
	
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};


UCLASS()
class NIX_API USM_STTMoveToPatrolPoint : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ASMSplineActor> SplineActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bChooseRandomPoint = true;

	int8 NextPatrolPoint = -1; 
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	void SetRandomPoint();
	void GetNextPointFromCharacter();
	void PerformMove();
	UFUNCTION()
	void OnMoveFinished(FAIRequestID RequestID, EPathFollowingResult::Type Result);
};

UCLASS()
class NIX_API USM_STTMoveToLocation : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> Target;
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	UFUNCTION()
	void OnMoveFinished(FAIRequestID RequestID, EPathFollowingResult::Type Result);
};

UCLASS()
class NIX_API USM_STTFocusTarget : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> Target;
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
};


UCLASS()
class NIX_API USM_STTExecuteMeleeAttack : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
};

