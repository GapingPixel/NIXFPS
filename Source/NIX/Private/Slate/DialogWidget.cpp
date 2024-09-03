// Fill out your copyright notice in the Description page of Project Settings.
#include "Slate/DialogWidget.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

void SDialogWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(400.0f)
		.HeightOverride(200.0f)
		.VAlign(VAlign_Bottom)//Adjust the Widget Vertical Position
		[
			SNew(SVerticalBox)

			// Dialog Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10)
			[
				SAssignNew(DialogTitle, STextBlock)
				.Text(FText::FromString("Default Title"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
			]

			// Input Field
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10)
			[
				SAssignNew(InputTextBox, SEditableTextBox)
				.HintText(FText::FromString("Enter text..."))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)  // Center the button horizontally
			.Padding(10)
			[
				SNew(SBox)
				.HAlign(HAlign_Left)  // Center the content within the box
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.Text(FText::FromString("Submit"))
					.OnClicked(this, &SDialogWidget::OnButtonClick)
				]
			]
		]
	];
}

FReply SDialogWidget::OnButtonClick() const
{
	if (InputTextBox.IsValid())
	{
		const FText InputText = InputTextBox->GetText();
		UE_LOG(LogTemp, Log, TEXT("Input Text: %s"), *InputText.ToString());

		//Update the dialog title with the input text
		if (DialogTitle.IsValid())
		{
			DialogTitle->SetText(InputText);
		}
		//Clear the text box
		InputTextBox->SetText(FText::GetEmpty());
	}
	return FReply::Handled();
}
