// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "SMGC_Gunfire.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API USMGC_Gunfire : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> MuzzleFlash;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> MuzzleSmoke;

	// Impact Particle Systems
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Impact List", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> ImpactDefault;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Impact List", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> ImpactAsphalt;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Impact List", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> ImpactConcrete;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Impact List", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> ImpactDirt;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Impact List", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> ImpactMetal;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Impact List", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> ImpactWater;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Impact List", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> ImpactWood;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Impact List", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> ImpactGlass;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Impact List", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> ImpactFlesh;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Impact List", meta=(MultiLine="true"))
	TObjectPtr<UParticleSystem> ImpactGrass;

	// Impact Sounds
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Sound List", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> SoundDefault;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Sound List", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> SoundAsphalt;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Sound List", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> SoundConcrete;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Sound List", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> SoundDirt;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Sound List", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> SoundMetal;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Sound List", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> SoundWater;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Sound List", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> SoundWood;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Sound List", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> SoundGlass;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Sound List", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> SoundFlesh;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Sound List", meta=(MultiLine="true"))
	TObjectPtr<USoundBase> SoundGrass;
	
	// Impact Decals
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Decal List", meta=(MultiLine="true"))
	TObjectPtr<UMaterialInterface> DecalDefault;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Decal List", meta=(MultiLine="true"))
	TObjectPtr<UMaterialInterface> DecalAsphalt;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Decal List", meta=(MultiLine="true"))
	TObjectPtr<UMaterialInterface> DecalConcrete;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Decal List", meta=(MultiLine="true"))
	TObjectPtr<UMaterialInterface> DecalDirt;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Decal List", meta=(MultiLine="true"))
	TObjectPtr<UMaterialInterface> DecalMetal;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Decal List", meta=(MultiLine="true"))
	TObjectPtr<UMaterialInterface> DecalWater;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Decal List", meta=(MultiLine="true"))
	TObjectPtr<UMaterialInterface> DecalWood;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Decal List", meta=(MultiLine="true"))
	TObjectPtr<UMaterialInterface> DecalGlass;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Decal List", meta=(MultiLine="true"))
	TObjectPtr<UMaterialInterface> DecalFlesh;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default| Decal List", meta=(MultiLine="true"))
	TObjectPtr<UMaterialInterface> DecalGrass;
	
	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;
	
};
