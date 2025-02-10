#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Items/SMGunBase.h"
#include "Items/SMManualRechamberGunBase.h"
#include "NIX/Public/Components/SMEquippableInventoryComponent.h"
#include "FunctionLibrary.generated.h"


UCLASS()
class NIX_API UFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE  ASMEquippableBase* GetCurrentEquippable(const AActor* Actor)
	{
		const TSubclassOf<USMEquippableInventoryComponent> EquippableClass = USMEquippableInventoryComponent::StaticClass();
		return Cast<USMEquippableInventoryComponent>(Actor->GetComponentByClass(EquippableClass))->CurrentEquippable;
	}

	FORCEINLINE static ASMGunBase* GetCurrentEquippableIfGun(const AActor* Actor, UWorld* WorldContext)
	{
		const TSubclassOf<USMEquippableInventoryComponent> EquippableClass = USMEquippableInventoryComponent::StaticClass();
		return Cast<ASMGunBase>(Actor->GetComponentByClass(EquippableClass));
	}

	FORCEINLINE static ASMGunBase* GetCurrentEquippableIfRechamberGun(const AActor* Actor, UWorld* WorldContext)
	{
		const TSubclassOf<USMEquippableInventoryComponent> EquippableClass = USMEquippableInventoryComponent::StaticClass();
		return Cast<ASMManualRechamberGunBase>(Actor->GetComponentByClass(EquippableClass));
	}
	
};

