// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CueNotifies/SMGC_Gunfire.h"

#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NIX/GlobalFunctions.h"

void USMGC_Gunfire::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType,
                                      const FGameplayCueParameters& Parameters)
{
	Super::HandleGameplayCue(MyTarget, EventType, Parameters);
	
	if (IsValid(MyTarget))
	{
		K2_HandleGameplayCue(MyTarget, EventType, Parameters);

		switch (EventType)
		{
		case EGameplayCueEvent::OnActive:
			OnActive(MyTarget, Parameters);
			break;

		case EGameplayCueEvent::WhileActive:
			WhileActive(MyTarget, Parameters);
			break;

		case EGameplayCueEvent::Executed:
			OnExecute(MyTarget, Parameters);
			if (IsValid(GlobalFunctions::GetCurrentEquippable(Parameters.Instigator.Get())))
			{
				const ASMEquippableBase* Equippable = GlobalFunctions::GetCurrentEquippable(Parameters.Instigator.Get());
				const auto Role = Parameters.Instigator.Get()->GetLocalRole();
				const bool bIsLocallyControlled = Role == ENetRole::ROLE_AutonomousProxy || ENetRole::ROLE_Authority;
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, FVector::Zero() );
				const auto Mesh = bIsLocallyControlled ? Equippable->GetEquippableMesh1P() : Equippable->GetEquippableMesh3P();
				UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Mesh,
					FName("Muzzle"),FVector::Zero(), FRotator(0,90,0));

				UGameplayStatics::SpawnEmitterAttached(MuzzleSmoke,  Mesh ,
					FName("Muzzle"), FVector::Zero(), FRotator(0,90,0));

				if (IsValid(Parameters.PhysicalMaterial.Get()))
				{
					UParticleSystem* SelectedImpactEffect;
					USoundBase* MatSound;
					UMaterialInterface* MatI;
					switch (Parameters.PhysicalMaterial->SurfaceType) //For Emitter and Sound 
					{
						case SurfaceType_Default: // Default
							SelectedImpactEffect = ImpactDefault;
							MatSound = SoundDefault;
							MatI = DecalDefault;
							break;

						case SurfaceType1: // Asphalt
							SelectedImpactEffect = ImpactAsphalt;
							MatSound = SoundAsphalt;
							MatI = DecalAsphalt;
							break;
	        
						case SurfaceType2: // Concrete
							SelectedImpactEffect = ImpactConcrete;
							MatSound = SoundConcrete;
							MatI = DecalConcrete;
							break;
	        
						case SurfaceType3: // Dirt
							SelectedImpactEffect = ImpactDirt;
							MatSound = SoundDirt;
							MatI = DecalDirt;
							break;
	        
						case SurfaceType4: // Metal
							SelectedImpactEffect = ImpactMetal;
							MatSound = SoundMetal;
							MatI = DecalMetal;
							break;
	        
						case SurfaceType5: // Water
							SelectedImpactEffect = ImpactWater;
							MatSound = SoundWater;
							MatI = DecalWater;
							break;
	        
						case SurfaceType6: // Wood
							SelectedImpactEffect = ImpactWood;
							MatSound = SoundWood;
							MatI = DecalWood;
							break;
	        
						case SurfaceType7: // Glass
							SelectedImpactEffect = ImpactGlass;
							MatSound = SoundGlass;
							MatI = DecalGlass;
							break;
	        
						case SurfaceType8: // Flesh
							SelectedImpactEffect = ImpactFlesh;
							MatSound = SoundFlesh;
							MatI = DecalFlesh;
							break;
	        
						case SurfaceType9: // Grass
							SelectedImpactEffect = ImpactGrass;
							MatSound = SoundGrass;
							MatI = DecalGrass;
							break;
					default: SelectedImpactEffect = ImpactDefault;
						MatSound = SoundDefault;
						MatI = DecalDefault;
						break;
					}
					const UWorld* W = GetWorld();
					const auto ToRotate = Parameters.Normal.ToOrientationRotator();
					UGameplayStatics::SpawnEmitterAtLocation(W, SelectedImpactEffect, Parameters.Location, FRotator(ToRotate.Pitch-30,ToRotate.Yaw,ToRotate.Roll));
					
					UGameplayStatics::SpawnSoundAtLocation(W, MatSound, Parameters.Location);
					const float DecalSize = FMath::FRandRange(1.0,4.0);
					const auto Decal = UGameplayStatics::SpawnDecalAtLocation(W, MatI, FVector(DecalSize, DecalSize, DecalSize), Parameters.Location, FRotator(ToRotate.Pitch,  ToRotate.Yaw, FMath::FRand()*360), 600);
					if (IsValid(Decal))
					{
						Decal->SetFadeScreenSize(0);
					}
				} 
			} 
			break;

		case EGameplayCueEvent::Removed:
			OnRemove(MyTarget, Parameters);
			break;
		};
	}
}
