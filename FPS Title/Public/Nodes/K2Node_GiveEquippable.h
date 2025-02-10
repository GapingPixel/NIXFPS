// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_ConstructObjectFromClass.h"
#include "K2Node_GiveEquippable.generated.h"

/**
 * 
 */
UCLASS()
class NIX_API UK2Node_GiveEquippable : public UK2Node_ConstructObjectFromClass
{
	GENERATED_BODY()

	//~ Begin UEdGraphNode Interface.
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;
	virtual void AllocateDefaultPins() override;
	//~ End UEdGraphNode Interface.

	//~ Begin UK2Node Interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	//virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	//~ End UK2Node Interface
};
