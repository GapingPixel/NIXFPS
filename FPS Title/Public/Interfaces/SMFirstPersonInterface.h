// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SMEquippableInventoryComponent.h"
#include "UObject/Interface.h"
#include "SMFirstPersonInterface.generated.h"

class USMAbilitySystemComponent;
// Enum type to determine what kind of mesh to get from the GetMeshOfType interface function.
UENUM(BlueprintType)
enum class EMeshType : uint8
{
	FirstPersonHands,
	ThirdPersonBody /* This also will be the shadow for first person legs. */,
	FirstPersonLegs
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USMFirstPersonInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ISMFirstPersonInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/* Mesh
	***********************************************************************************/
	
	virtual USkeletalMeshComponent* GetMeshOfType(EMeshType MeshType) = 0;
	virtual FName GetBoneAttachName(EMeshType MeshType) = 0;
	virtual USMEquippableInventoryComponent* GetInventoryComponent() = 0;
	virtual USMAbilitySystemComponent* GetSMAbilitySystemComponent() const = 0;
};
