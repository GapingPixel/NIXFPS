#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BarricadeClasses.generated.h"

class ASMPlayerCharacter;
class UBoxComponent;

UCLASS()
class NIX_API ASMBarricadeBase : public AActor
{
	GENERATED_BODY()
	
public:
	ASMBarricadeBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> MainMesh;

	UPROPERTY()
	TObjectPtr<ASMPlayerCharacter> PlayerRef;

	uint8 bCanBeInteracted:1;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> MIOutlineHighlight;

	float HP = 10;
	float MaxHP = HP;
protected:
	/*FTimerHandle TimerHighlight;
	UPROPERTY()
	TArray<TObjectPtr<UPrimitiveComponent>> HighLightPrimitives;
	void DefineHighlightMeshes();
	UFUNCTION(BlueprintCallable)
	void TurnOnHighlight(const float Duration);	*/
	virtual void PostInitializeComponents() override;
	
	virtual void BeginPlay() override;
public:
	virtual void Interact();
	
};


UCLASS()
class NIX_API ASMBarricadeDoor : public ASMBarricadeBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASMBarricadeDoor();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> InteractArea;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Slot2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpenAngle = 160.0f;  // Angle to open the door

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpenDuration = 0.2f;

	void DoorInteraction();

	uint8 bIsOpen:1;

	virtual void Interact() override;
protected:
	// Called when the game starts or when spawned
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	
	void UpdateDoorRotation();

	FTimerDelegate DoorRotationTimerDelegate;
	float CurrentTime;

	
private:
	UFUNCTION()
	void InteractAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void InteractAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	FQuat ClosedRotation;
	FQuat OpenRotation;

};

UCLASS()
class NIX_API ASMBarricadeObject : public ASMBarricadeBase
{
	GENERATED_BODY()

protected:
	virtual void PostInitializeComponents() override;
private:
/*
	UFUNCTION()
	void InteractAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void InteractAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);*/
};
