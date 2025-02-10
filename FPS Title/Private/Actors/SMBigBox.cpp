// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/SMBigBox.h"

#include "Actors/SMInteractableActor.h"
#include "Components/BoxComponent.h"
#include "GameModes/SMTestGameMode.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASMBigBox::ASMBigBox()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	//RootComponent->SetComponentTickEnabled(false);
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	//SetRootComponent(StaticMeshComponent);
	StaticMeshComponent->SetupAttachment(RootComponent);
	//StaticMeshComponent->SetComponentTickEnabled(false);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);
	//BoxComponent->SetComponentTickEnabled(false);
	LittleBoxTarget = 4;
}

void ASMBigBox::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ASMBigBox::CheckAmmountOfLittleBoxes);
}

void ASMBigBox::CheckAmmountOfLittleBoxes(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if (OtherActor->IsA<ASMInteractableActor>())
	{
		LittleBoxCount++;
		//OtherActor->Destroy();
		GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Red,"Triggers");
		if (LittleBoxCount >= LittleBoxTarget)
		{
			Cast<ASMTestGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->BoxObjectiveCompleted();
			GEngine->AddOnScreenDebugMessage(-1,0.4f,FColor::Red,"BoxObjectiveCompleted");
		}
	}
}


