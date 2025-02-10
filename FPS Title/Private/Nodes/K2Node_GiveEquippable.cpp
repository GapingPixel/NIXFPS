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
/*
void UK2Node_GiveEquippable::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	static const FName SpawnEquippableFuncName = GET_FUNCTION_NAME_CHECKED(USMEquippableInventoryComponent, GiveNewEquippable);
	static const FName EquippableClassParamName(TEXT("EquippableClass"));
	static const FName EquippableActorParamName(TEXT("EquippableReceiver"));

	UK2Node_GiveEquippable* GiveNode = this;
	UEdGraphPin* GiveNodeExec = GiveNode->GetExecPin();
	UEdGraphPin* GiveClassPin = GiveNode->GetClassPin();
	UEdGraphPin* GiveNodeThen = GiveNode->GetThenPin();

	UClass* GiveClass = (GiveClassPin != NULL) ? Cast<UClass>(GiveClassPin->DefaultObject) : NULL;
	if ( !GiveClassPin || ((0 == GiveClassPin->LinkedTo.Num()) && (NULL == GiveClass)))
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("GiveEquippableNodeMissingClass_Error", "Spawn node @@ must have a @@ specified.").ToString(), GiveNode, GiveClassPin);
		// we break exec links so this is the only error we get, don't want the SpawnActor node being considered and giving 'unexpected node' type warnings
		GiveNode->BreakAllNodeLinks();
		return;
	}

	UK2Node_CallFunction* CallGiveEquippableNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(GiveNode, SourceGraph);
	CallGiveEquippableNode->FunctionReference.SetExternalMember(SpawnEquippableFuncName, USMEquippableInventoryComponent::StaticClass());
	CallGiveEquippableNode->AllocateDefaultPins();

	UEdGraphPin* CallBeginExec = CallGiveEquippableNode->GetExecPin();
	UEdGraphPin* CallFinishThen = CallGiveEquippableNode->GetThenPin();
	UEdGraphPin* CallGiveEquippable_ClassPin = CallGiveEquippableNode->FindPinChecked(EquippableClassParamName);
	UEdGraphPin* CallGiveEquippable_ActorPin = CallGiveEquippableNode->FindPinChecked(EquippableActorParamName);

	// Move 'exec' connection from spawn node to 'begin spawn'
	CompilerContext.MovePinLinksToIntermediate(*GiveNodeExec, *CallBeginExec);

	// Move 'then' connection from spawn node to 'finish spawn'
	CompilerContext.MovePinLinksToIntermediate(*GiveNodeThen, *CallFinishThen);

	if(GiveClassPin->LinkedTo.Num() > 0)
	{
		// Copy the 'blueprint' connection from the spawn node to 'begin spawn'
		CompilerContext.MovePinLinksToIntermediate(*GiveClassPin, *CallGiveEquippable_ActorPin);
	}
	else
	{
		// Copy blueprint literal onto begin spawn call 
		CallGiveEquippable_ClassPin->DefaultObject = GiveClass;
	}

	// Break any links to the expanded node
	GiveNode->BreakAllNodeLinks();
}*/


#undef LOCTEXT_NAMESPACE
