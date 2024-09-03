#pragma once
#include "CoreMinimal.h"
#include "Items/SMGunBase.h"
#include "Items/SMManualRechamberGunBase.h"
#include "NIX/Public/Components/SMEquippableInventoryComponent.h"

UENUM(BlueprintType)
enum class EEMeshType : uint8
{
	Arms1P, 
	Equippable1P,
	Arms3P,
	Equippable3P,
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	SemiAuto, 
	FullAuto 
};

namespace GlobalFunctions
{
	FORCEINLINE static ASMEquippableBase* GetCurrentEquippable(const AActor* Actor)
	{
		const TSubclassOf<USMEquippableInventoryComponent> EquippableClass = USMEquippableInventoryComponent::StaticClass();
		return Cast<USMEquippableInventoryComponent>(Actor->GetComponentByClass(EquippableClass))->CurrentEquippable;
	}

	FORCEINLINE static ASMGunBase* GetCurrentEquippableIfGun(const AActor* Actor, UWorld* WorldContext)
	{
		TSubclassOf<USMEquippableInventoryComponent> EquippableClass = USMEquippableInventoryComponent::StaticClass();
		//return Cast<ASMGunBase>(Actor->GetComponentByClass(EquippableClass));
		return Cast<ASMGunBase>(Cast<USMEquippableInventoryComponent>(Actor->GetComponentByClass(EquippableClass))->CurrentEquippable);
	}

	FORCEINLINE static ASMManualRechamberGunBase* GetCurrentEquippableIfRechamberGun(const AActor* Actor, UWorld* WorldContext)
	{
		const TSubclassOf<USMEquippableInventoryComponent> EquippableClass = USMEquippableInventoryComponent::StaticClass();
		return Cast<ASMManualRechamberGunBase>(Cast<USMEquippableInventoryComponent>(Actor->GetComponentByClass(EquippableClass))->CurrentEquippable);
	}

	
};
