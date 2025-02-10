// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "SM_STEvaluators.generated.h"

class ASMSplineActor;
/**
 * 
 */
UCLASS()
class NIX_API USM_STEGetSpline : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output"/*, meta = (DisplayName = "Spline Actor", Categories = "StateTree.Output")*/)
	TObjectPtr<ASMSplineActor> SplineActor;

	virtual void TreeStart(FStateTreeExecutionContext& Context) override;
};

UCLASS()
class NIX_API USM_STEGetPatrolRoute : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output"/*, meta = (DisplayName = "Spline Actor", Categories = "StateTree.Output")*/)
	TObjectPtr<ASMSplineActor> SplineActor;

	virtual void TreeStart(FStateTreeExecutionContext& Context) override;
};

UCLASS()
class NIX_API USM_STEFindClosestTarget : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output")
	TObjectPtr<ACharacter> TargetActor;

	TArray<TObjectPtr<ACharacter>> TargetArray;

	float MinReactionDistance = 500;
	
	virtual void TreeStart(FStateTreeExecutionContext& Context) override;

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};


