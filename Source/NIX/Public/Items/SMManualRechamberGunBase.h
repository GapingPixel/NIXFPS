// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/SMGunBase.h"
#include "SMManualRechamberGunBase.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API ASMManualRechamberGunBase : public ASMGunBase
{
	GENERATED_BODY()

public:
	ASMManualRechamberGunBase();

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	
	/* Animations
	***********************************************************************************/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Animation")
	FEquippableAnimCluster ReloadEnterAnimations;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Animation")
	FEquippableAnimCluster ReChamberAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippable|Animation")
	FEquippableAnimCluster ReloadExitAnimations;
	
	/* Blueprint Exposed
	***********************************************************************************/
	protected:

	// The Ability to activate using it's GameplayTag.
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Gun")
	FGameplayTag ReChamberAbilityTag;

	// Should this rechamber gun start unchambered?
	UPROPERTY(EditDefaultsOnly, Category = "Equippable|Gun")
	bool bStartUnChambered = true;

	// Should this rechamber gun start unchambered when explicitly spawned in?
	UPROPERTY(BlueprintReadOnly, Category = "Equippable|Gun", meta=(ExposeOnSpawn=true))
	bool bSpawnUnChambered = bStartUnChambered;

public:

	// If this gun is a re-chamber gun (shotgun, winchester, etc) this sets the state if the gun needs to be re-chambered.
	UFUNCTION(BlueprintCallable, Category = "Equippable|Gun")
	void SetNeedsRechambering(bool bNewRechamberState) { bNeedsRechambering = bNewRechamberState; };

	// Returns whether this gun needs rechambering (if it's a rechambering gun).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equippable|Gun")
	bool NeedsRechambering() const { return bNeedsRechambering; }

	/* Other
	***********************************************************************************/

protected:

	// Checks if this equippable needs to rechamber.
	UFUNCTION()
	void CheckForReChamber();

	virtual void OnExplicitlySpawnedIn() override;
	
private:

	UPROPERTY(Replicated)
	bool bNeedsRechambering = false;
};
