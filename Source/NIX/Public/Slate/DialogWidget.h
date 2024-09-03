// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */

class NIX_API SDialogWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDialogWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	
	TSharedPtr<STextBlock> DialogTitle; 
	TSharedPtr<SEditableTextBox> InputTextBox;

	FReply OnButtonClick() const;
	
};