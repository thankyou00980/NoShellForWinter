#include "CharacterBackground/UI/ProjectCharacterBackgroundEffectEntryWidget.h"

#include "CharacterBackground/ProjectCharacterBackgroundSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"

namespace ProjectCharacterBackgroundEffectEntryPrivate
{
	const FLinearColor PositiveTint(0.78f, 0.88f, 1.0f, 1.0f);
	const FLinearColor NegativeTint(1.0f, 0.58f, 0.72f, 1.0f);
	const FLinearColor FillTint(0.02f, 0.016f, 0.035f, 0.90f);
	const FLinearColor OutlineTint(0.58f, 0.34f, 0.62f, 0.35f);

	const FProjectCharacterBackgroundUILayoutTuning& GetLayoutTuning()
	{
		if (const UProjectCharacterBackgroundSettings* Settings = UProjectCharacterBackgroundSettings::Get())
		{
			return Settings->UILayout;
		}

		static const FProjectCharacterBackgroundUILayoutTuning DefaultLayout;
		return DefaultLayout;
	}
}

void UProjectCharacterBackgroundEffectEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	InitializeVisualTree();
}

void UProjectCharacterBackgroundEffectEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetTree();
	InitializeVisualTree();
}

void UProjectCharacterBackgroundEffectEntryWidget::SetEffectText(const FText& InText, const bool bInNegative)
{
	BuildWidgetTree();
	InitializeVisualTree();
	bNegative = bInNegative;

	if (EffectText)
	{
		EffectText->SetText(InText);
		EffectText->SetColorAndOpacity(bNegative
			? ProjectCharacterBackgroundEffectEntryPrivate::NegativeTint
			: ProjectCharacterBackgroundEffectEntryPrivate::PositiveTint);
	}
}

void UProjectCharacterBackgroundEffectEntryWidget::BuildWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackgroundEffectRootBorder"));
	WidgetTree->RootWidget = RootBorder;
	RootBorder->SetPadding(ProjectCharacterBackgroundEffectEntryPrivate::GetLayoutTuning().EffectEntryPadding);

	EffectText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BackgroundEffectText"));
	EffectText->SetAutoWrapText(true);
	RootBorder->SetContent(EffectText);
}

void UProjectCharacterBackgroundEffectEntryWidget::InitializeVisualTree()
{
	if (bVisualTreeInitialized)
	{
		return;
	}

	if (RootBorder)
	{
		const FSlateRoundedBoxBrush Brush(
			ProjectCharacterBackgroundEffectEntryPrivate::FillTint,
			ProjectCharacterBackgroundEffectEntryPrivate::GetLayoutTuning().PanelCornerRadius,
			FSlateColor(ProjectCharacterBackgroundEffectEntryPrivate::OutlineTint),
			0.8f,
			FVector2f(
				ProjectCharacterBackgroundEffectEntryPrivate::GetLayoutTuning().EffectEntryWidth,
				ProjectCharacterBackgroundEffectEntryPrivate::GetLayoutTuning().EffectEntryHeight));
		RootBorder->SetBrush(Brush);
	}

	if (EffectText)
	{
		EffectText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", ProjectCharacterBackgroundEffectEntryPrivate::GetLayoutTuning().EffectFontSize));
		EffectText->SetColorAndOpacity(bNegative
			? ProjectCharacterBackgroundEffectEntryPrivate::NegativeTint
			: ProjectCharacterBackgroundEffectEntryPrivate::PositiveTint);
	}

	bVisualTreeInitialized = true;
}
