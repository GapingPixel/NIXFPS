// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMHealthDrainArea.generated.h"

class UBoxComponent;
class UGameplayEffect;

UCLASS()
class NIX_API ASMHealthDrainArea : public AActor
{
	GENERATED_BODY()
	
public:
	ASMHealthDrainArea();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> DamageGE;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> MaxStaminaDiminishGE;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> MaxStaminaResetGE;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
	TSubclassOf<UGameplayEffect> StaminaGE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float DamagePerSecond = 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float MaxStaminaDepletionPerSecond = 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float MaxStaminaMinValue = 25;
protected:
	
	// The area trigger (you can use USphereComponent, UBoxComponent, etc.)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	// Timer handle for the health drain
	FTimerHandle HealthDrainTimerHandle;
	FTimerHandle MaxStaminaDrainTimerHandle;
	
	// Called when player enters the area
	UFUNCTION()
	void OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when player leaves the area
	UFUNCTION()
	void OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// Reference to the player, assuming single player.
	UPROPERTY()
	class ASMPlayerCharacter* AffectedPlayer;
};
