#include "Actors/BarricadeClasses.h"
#include "Components/BoxComponent.h"
#include "Possessables/SMPlayerCharacter.h"

ASMBarricadeBase::ASMBarricadeBase()
{
	PrimaryActorTick.bCanEverTick = false;
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	RootComponent = DefaultSceneRoot;
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>("MainMesh");
	MainMesh->SetupAttachment(DefaultSceneRoot);
	MainMesh->bUseAttachParentBound = false;
	MainMesh->PrimaryComponentTick.bCanEverTick = false;
}

void ASMBarricadeBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ASMBarricadeBase::BeginPlay()
{
	Super::BeginPlay();
	//TurnOnHighlight(40);
}

void ASMBarricadeBase::Interact()
{
	
}

/*
void ASMBarricadeBase::DefineHighlightMeshes()
{
	//HighLightPrimitives.Empty();
	TArray<UActorComponent*> StaticMeshComponents = GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("Highlightable"));
	for (UActorComponent* Component : StaticMeshComponents)
	{
		if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Component))
		{
			HighLightPrimitives.Add(StaticMesh);
		}
	}
	HighLightPrimitives.Add(MainMesh);
}

void ASMBarricadeBase::TurnOnHighlight(const float Duration)
{
	const UWorld* World = GetWorld();
	World->GetTimerManager().ClearTimer(TimerHighlight);
	for (UPrimitiveComponent* Primitive :HighLightPrimitives)
	{
		Primitive->SetCustomDepthStencilValue(1);
		Primitive->SetRenderCustomDepth(true);
	}
	World->GetTimerManager().SetTimer(TimerHighlight, [this]()
	{
		for (UPrimitiveComponent* Primitive :HighLightPrimitives)
		{
			Primitive->SetRenderCustomDepth(false);
		}
	}, Duration, false);
}*/

// Sets default values
ASMBarricadeDoor::ASMBarricadeDoor()
{
	InteractArea = CreateDefaultSubobject<UBoxComponent>("InteractArea");
	InteractArea->SetupAttachment(RootComponent);
	InteractArea->PrimaryComponentTick.bCanEverTick = false;
}

void ASMBarricadeDoor::Interact()
{
	Super::Interact();
	DoorInteraction();
	
}

void ASMBarricadeDoor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	InteractArea->OnComponentBeginOverlap.AddDynamic(this, &ASMBarricadeDoor::InteractAreaBeginOverlap);
	InteractArea->OnComponentEndOverlap.AddDynamic(this, &ASMBarricadeDoor::InteractAreaEndOverlap);
}


// Called when the game starts or when spawned
void ASMBarricadeDoor::BeginPlay()
{
	Super::BeginPlay();
	ClosedRotation = MainMesh->GetRelativeRotation().Quaternion();
	OpenRotation = FQuat(FRotator(0.0f, OpenAngle, 0.0f)) * ClosedRotation; // Add rotation on the yaw axis
	DoorRotationTimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::UpdateDoorRotation);
	//DefineHighlightMeshes();
	//DoorInteraction();
}


void ASMBarricadeDoor::UpdateDoorRotation()
{
	CurrentTime += GetWorld()->GetDeltaSeconds(); // Increment time by the timer interval
	const float Alpha = FMath::Clamp(CurrentTime / OpenDuration, 0.0f, 1.0f);
	const FQuat TargetRotation = bIsOpen ? OpenRotation : ClosedRotation;
	// Interpolate smoothly
	const FQuat CurrentRotation = FQuat::Slerp(bIsOpen ? ClosedRotation : OpenRotation, TargetRotation, Alpha);
	MainMesh->SetRelativeRotation(CurrentRotation.Rotator());
	if (Alpha < 1.0f) GetWorldTimerManager().SetTimerForNextTick(DoorRotationTimerDelegate);
}

void ASMBarricadeDoor::DoorInteraction()
{
	CurrentTime = 0.0f;
	UpdateDoorRotation();
	bIsOpen = !bIsOpen;
}

void ASMBarricadeDoor::InteractAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 2, FColor::White, FString("Overlap"));
	if ((PlayerRef = Cast<ASMPlayerCharacter>(OtherActor)))
	{
		bCanBeInteracted = true;
		PlayerRef->BarricadeObjectsToEvaluate.Add(this);
		MainMesh->SetOverlayMaterial(MIOutlineHighlight);
	}
}

void ASMBarricadeDoor::InteractAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (PlayerRef == OtherActor)
	{
		bCanBeInteracted = false;
		PlayerRef->BarricadeObjectsToEvaluate.Remove(this);
		MainMesh->SetOverlayMaterial(nullptr);
	}
}

void ASMBarricadeObject::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}


