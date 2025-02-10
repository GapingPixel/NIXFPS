// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SMStateTreeComponent.h"


USMStateTreeComponent::USMStateTreeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


const UStateTree* USMStateTreeComponent::GetStateTree() 
{
	return StateTreeRef.GetStateTree();
}


void USMStateTreeComponent::SetStateTree(UStateTree* InStateTree)
{
	StateTreeRef.SetStateTree(InStateTree);
	InitializeComponent();
}
