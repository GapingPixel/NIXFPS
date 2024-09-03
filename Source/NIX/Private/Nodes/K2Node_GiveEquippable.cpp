// Fill out your copyright notice in the Description page of Project Settings.


#include "Nodes/K2Node_GiveEquippable.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
//#include "K2Node_CallFunction.h"
#include "K2Node_SpawnActorFromClass.h"
/*#include "KismetCompiler.h"
#include "Components/SMEquippableInventoryComponent.h"
#include "Kismet/GameplayStatics.h"*/

#define LOCTEXT_NAMESPACE "K2Node_GiveEquippable"

FText UK2Node_GiveEquippable::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("GiveEquippableK2Node_Title", "Give Equippable");
}

FText UK2Node_GiveEquippable::GetTooltipText() const
{
	return LOCTEXT("GiveEquippableK2Node_Tooltip", "Gives brand new Equippable to a character.");
}

FText UK2Node_GiveEquippable::GetMenuCategory() const
{
	return LOCTEXT("GiveEquippableK2Node_MenuCategory", "nfPopulationSystem");
}

void UK2Node_GiveEquippable::AllocateDefaultPins()
{
	// Add execution pins
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// Add blueprint pin
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Class, GetClassPinBaseClass(), TEXT("EquippableClass"));

	Super::AllocateDefaultPins();
}

void UK2Node_GiveEquippable::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);
	const UClass* Action = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(Action))
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
		check(Spawner != nullptr);
		ActionRegistrar.AddBlueprintAction(Action, Spawner);
	}
}


#undef LOCTEXT_NAMESPACE
