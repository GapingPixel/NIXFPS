// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeComponent.h"
#include "SMStateTreeComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NIX_API USMStateTreeComponent : public UStateTreeComponent
{
	GENERATED_BODY()

public:
	USMStateTreeComponent();

	const UStateTree* GetStateTree();

	void SetStateTree(UStateTree* InStateTree);
};
