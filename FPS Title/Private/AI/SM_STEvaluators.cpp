// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SM_STEvaluators.h"

#include "EngineUtils.h"
#include "AI/SMSplineActor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Possessables/SMBaseHorror.h"

void USM_STEGetSpline::TreeStart(FStateTreeExecutionContext& Context)
{
	Super::TreeStart(Context);

	TArray<AActor*> SplineActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASMSplineActor::StaticClass(), SplineActors);
	float Distance;
	SplineActor = Cast<ASMSplineActor>(UGameplayStatics::FindNearestActor(Actor->GetActorLocation(),  SplineActors, Distance)) ;
	
}

void USM_STEGetPatrolRoute::TreeStart(FStateTreeExecutionContext& Context)
{
	Super::TreeStart(Context);
	TArray<AActor*> SplineActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASMSplineActor::StaticClass(), SplineActors);
	float Distance;
	SplineActor = Cast<ASMSplineActor>(UGameplayStatics::FindNearestActor(Actor->GetActorLocation(),  SplineActors, Distance));
}

void USM_STEFindClosestTarget::TreeStart(FStateTreeExecutionContext& Context)
{
	Super::TreeStart(Context);
	if (const UWorld* World = GEngine->GetWorldFromContextObject(GetWorld(), EGetWorldErrorMode::LogAndReturnNull))
	{
		for (TActorIterator<ACharacter> It(World, ACharacter::StaticClass()); It; ++It)
		{
			ACharacter* Act = *It;
			TargetArray.Add(Act);
		}
	}
	MinReactionDistance = Cast<ASMBaseHorror>(Actor)->ReactionRadius->GetScaledSphereRadius();
}

void USM_STEFindClosestTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);
	for (ACharacter* Character : TargetArray)
	{
		if (Character != Actor && Character->IsLocallyControlled())
		{
			/*if (Actor->GetDistanceTo(Character) <= MinReactionDistance)
			{
				
			}*/
			TargetActor = Character;
		}
	}
}
