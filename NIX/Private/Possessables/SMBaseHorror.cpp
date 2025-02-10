// Fill out your copyright notice in the Description page of Project Settings.


#include "Possessables/SMBaseHorror.h"

#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "AI/SMStateTreeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SMHealthComponent.h"
#include "Components/SMPawnComponent.h"
#include "GameModes/SMTestGameMode.h"
#include "GAS/AttributeSets/SMCombatAttributeSet.h"
#include "GAS/AttributeSets/SMHealthAttributeSet.h"
#include "GAS/AttributeSets/SMSurvivorAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "NIX/NIX.h"

ASMBaseHorror::ASMBaseHorror(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
	ReactionRadius = CreateDefaultSubobject<USphereComponent>("ReactionRadius");
	ReactionRadius->SetupAttachment(GetCapsuleComponent());
	MeleeAttackRadius = CreateDefaultSubobject<USphereComponent>("MeleeAttackRadius");
	MeleeAttackRadius->SetupAttachment(GetCapsuleComponent());
	RangedAttackRadius = CreateDefaultSubobject<USphereComponent>("RangedAttackRadius");
	RangedAttackRadius->SetupAttachment(GetCapsuleComponent());
	ReactionRadius->SetIsReplicated(true);
	MeleeAttackRadius->SetIsReplicated(true);
	RangedAttackRadius->SetIsReplicated(true);
	HealthComp = CreateDefaultSubobject<USMHealthComponent>("HealthComp");
	SMStateTree = CreateDefaultSubobject<USMStateTreeComponent>("SMStateTree");
	SMStateTree->SetStateTree(STPatrol);
	
	PawnComponent = CreateDefaultSubobject<USMPawnComponent>(TEXT("PawnComponent"));
	CharacterAttributeSet = CreateDefaultSubobject<USMCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	HealthAttributeSet = CreateDefaultSubobject<USMHealthAttributeSet>(TEXT("HealthSet"));
	CombatAttributeSet = CreateDefaultSubobject<USMCombatAttributeSet>(TEXT("CombatSet"));

	AbilitySystemComponent = CreateDefaultSubobject<USMAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	//AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	
	NetUpdateFrequency = 70.f;

	GetCapsuleComponent()->SetCollisionObjectType(COLLISION_SMCHARACTERBASE);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_SMCHARACTERBASE, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(TRACECHANNEL_BULLET, ECR_Ignore);

	GetMesh()->SetCollisionResponseToChannel(TRACECHANNEL_BULLET, ECR_Block);
	FGenericTeamId(1);
	SetReplicates(true);
}

void ASMBaseHorror::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	PawnComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ASMBaseHorror::OnAbilitySystemComponentInitialized));
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	OnAbilitySystemComponentInitialized();
	ReactionRadius->OnComponentBeginOverlap.AddDynamic(this, &ASMBaseHorror::ReactSphereBeginOverlap);
	ReactionRadius->OnComponentEndOverlap.AddDynamic(this, &ASMBaseHorror::ReactSphereEndOverlap);
	MeleeAttackRadius->OnComponentBeginOverlap.AddDynamic(this, &ASMBaseHorror::MeleeSphereBeginOverlap);
	MeleeAttackRadius->OnComponentEndOverlap.AddDynamic(this, &ASMBaseHorror::MeleeSphereEndOverlap);
	HealthComp->OnDeath.AddDynamic(this, &ASMBaseHorror::PerformDeath);
	CombatAttributeSet->AdjustAttributesForDifficulty();
}

void ASMBaseHorror::BeginPlay()
{
	Super::BeginPlay();
	FTimerHandle HealthTimer;
	GetWorld()->GetTimerManager().SetTimer(HealthTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		const FString FloatAsString = FString::SanitizeFloat(HealthComp->GetHealth());

			// Display text on the screen
			GEngine->AddOnScreenDebugMessage(
				-1,                // Unique key (-1 makes it add a new message)
				0.4f,              // Duration (in seconds)
				FColor::Yellow,     // Color of the text
				FloatAsString      // The actual text to display
			);
		
	}), 0.5, true);
}

bool ASMBaseHorror::ShouldReact(AActor* Actor) const
{
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		if (Pawn->IsLocallyControlled() &&  Actor != this)
		{
			return true;
		}
	}
	return false;
}

void ASMBaseHorror::ActivateNewStateTree(UStateTree* NewStateTree) const
{
	SMStateTree->StopLogic("TransitioningStates");
	SMStateTree->SetStateTree(NewStateTree);
	SMStateTree->RestartLogic();
}

void ASMBaseHorror::TryToMelee()
{
	GetWorld()->GetTimerManager().SetTimer(MeleeAttackHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		//GetMesh()->GetAnimInstance()->Montage_Play(MeleeMontage, 1.0f); 
	}), 1.0f, true);
	//GetWorld()->GetTimerManager().SetTimer(MeleeAttackHandle, this, &ASMBaseHorror::Melee, 1.0f, true);
}

void ASMBaseHorror::ReactSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ShouldReact(OtherActor) && !bIsDead && bSeingPlayer)
	{
		if (SMStateTree->GetStateTree() != STMelee)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, FString("ReactInside"));
			ActivateNewStateTree(STReact);
		}
	}
}

void ASMBaseHorror::ReactSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ShouldReact(OtherActor) && !bIsDead )
	{
		if (AAIController* Contl = Cast<AAIController>(GetController()) )
		{
			Contl->SetFocus(nullptr);
			ActivateNewStateTree(STPatrol);
		}
	}
}

void ASMBaseHorror::MeleeSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString("Melee Overlap Outside"));
	if (ShouldReact(OtherActor) && !bIsDead && bSeingPlayer)
	{
			GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, FString("Melee Overlap Inside"));
			ActivateNewStateTree(STMelee);
	}
}

void ASMBaseHorror::MeleeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ShouldReact(OtherActor) && !bIsDead )
	{
		ActivateNewStateTree(STPatrol);
		const UWorld* W = GetWorld();
		if (UKismetSystemLibrary::K2_IsTimerActiveHandle(W, MeleeAttackHandle))
		{
			W->GetTimerManager().ClearTimer(MeleeAttackHandle);
		}
	}
}

void ASMBaseHorror::PerformDeath(AActor* OwningActor)
{
	//Destroy();
	if (HasAuthority())
	{
		Cast<ASMTestGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->CurrentTotalKills++; 
		SMStateTree->StopLogic("Is Now Dead");
		bIsDead = true;
	}
	FTimerHandle DestroyActorHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyActorHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		Destroy();
	}), 5, true);
	USkeletalMeshComponent* MeshIns = GetMesh();
	MeshIns->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshIns->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetSimulatePhysics(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASMBaseHorror::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASMBaseHorror, bIsDead);
}

/* GAS
***********************************************************************************/
void ASMBaseHorror::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (HasAuthority())
	{
		SetupGas();
	}
	
}

void ASMBaseHorror::OnAbilitySystemComponentInitialized()
{
	HealthComp->InitializeWithAbilitySystemComponent(AbilitySystemComponent);
}

void ASMBaseHorror::OnAbilitySystemComponentUnInitialized() const
{
	HealthComp->UnInitializeFromAbilitySystemComponent();
}

void ASMBaseHorror::SetupGas()
{
	if (HasAuthority())
	{
		AbilitySystemComponent->SetNumericAttributeBase(HealthComp->HealthSet->GetHealthAttribute(), 100);
		PawnComponent->InitializeAbilitySystemComponent(AbilitySystemComponent,this);
		SetupAttributes();
	}
	// @TODO: GET FUCKING RID OF THIS SHIT
	// Move this to one of the following places:
	// - AssetManager class’s start initial loading function
	// - UEngineSubsystem::Initialize()
	// - UEngine::Init()
	static int32 DoOnce = 0;
	if (DoOnce == 0)
	{
		DoOnce++;
		UAbilitySystemGlobals::Get().InitGlobalData();
	}
}

void ASMBaseHorror::SetupAttributes() const
{
	if (!HasAuthority()) // Ensure this runs only on the server
	{
		return;
	}
	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, FString("SetUpattributes"));
	const FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(StartingAttributes, 1, EffectContextHandle);
	if (NewHandle.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, FString("Applied Health"));
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
	}	
}

