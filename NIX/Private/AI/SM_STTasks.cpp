// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SM_STTasks.h"
#include "AIController.h"
#include "AI/SMSplineActor.h"
#include "Blueprint/AIAsyncTaskBlueprintProxy.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/SplineComponent.h"
#include "Possessables/SMBaseHorror.h"

void USM_STTDestroy::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::ExitState(Context, Transition);
	Actor->Destroy();
}

EStateTreeRunStatus USM_STTMoveAlongSpline::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	const USplineComponent* Spline = SplineActor->SplineComponent;
	Actor->SetActorLocation(Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World));

	if (Distance < Spline->GetSplineLength())
	{
		Distance += MovementSpeed;
	} else
	{
		Distance = 0;
		FinishTask(true);
	}
	return Super::Tick(Context, DeltaTime);
}

EStateTreeRunStatus USM_STTMoveToPatrolPoint::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	if (bChooseRandomPoint)
	{
		SetRandomPoint();
	} else
	{
		GetNextPointFromCharacter();
	}
	
	PerformMove();
	return Super::EnterState(Context, Transition);
}

FORCEINLINE void USM_STTMoveToPatrolPoint::SetRandomPoint()
{
	NextPatrolPoint = FMath::RandRange(0,SplineActor->SplineComponent->GetNumberOfSplinePoints()-1);
}

FORCEINLINE void USM_STTMoveToPatrolPoint::GetNextPointFromCharacter()
{
	ASMBaseHorror* Character = Cast<ASMBaseHorror>(Actor);
	NextPatrolPoint = Character->CurrentPoint++;
	if ( NextPatrolPoint >= SplineActor->SplineComponent->GetNumberOfSplinePoints())
	{
		Character->CurrentPoint = 0;
		NextPatrolPoint = 0;
	}
}

FORCEINLINE void USM_STTMoveToPatrolPoint::PerformMove()
{
	AAIController* Controller = Cast<AAIController>(Cast<ACharacter>(Actor)->GetController());
	if (Controller)
	{
		UAIBlueprintHelperLibrary::CreateMoveToProxyObject(GetWorld(), Cast<APawn>(Actor), SplineActor->SplineComponent->GetLocationAtSplinePoint(NextPatrolPoint, ESplineCoordinateSpace::World), nullptr, 5);
		Controller->ReceiveMoveCompleted.AddDynamic(this, &USM_STTMoveToPatrolPoint::OnMoveFinished);
	}
}

void USM_STTMoveToPatrolPoint::OnMoveFinished(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	FinishTask();
}

EStateTreeRunStatus USM_STTMoveToLocation::EnterState(FStateTreeExecutionContext& Context,
                                                      const FStateTreeTransitionResult& Transition)
{
	AAIController* Controller = Cast<AAIController>(Cast<APawn>(Actor)->GetController());
	if (Controller)
	{
		UAIBlueprintHelperLibrary::CreateMoveToProxyObject(GetWorld(), Cast<APawn>(Actor), FVector::ZeroVector, Target, 100);
		Controller->ReceiveMoveCompleted.AddDynamic(this, &USM_STTMoveToLocation::OnMoveFinished);
	}
	return Super::EnterState(Context, Transition);
}

void USM_STTMoveToLocation::OnMoveFinished(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	FinishTask();
}

EStateTreeRunStatus USM_STTFocusTarget::EnterState(FStateTreeExecutionContext& Context,
                                                   const FStateTreeTransitionResult& Transition)
{
	AAIController* Controller = Cast<AAIController>(Cast<APawn>(Actor)->GetController());
	if (Controller)
	{
		Controller->StopMovement();
		Controller->SetFocus(Target);
		FinishTask(true);
	}
	return Super::EnterState(Context, Transition);
}

EStateTreeRunStatus USM_STTExecuteMeleeAttack::EnterState(FStateTreeExecutionContext& Context,
                                                          const FStateTreeTransitionResult& Transition)
{
	Cast<ASMBaseHorror>(Actor)->TryToMelee();
	FinishTask(true);
	return EStateTreeRunStatus::Succeeded;
}
