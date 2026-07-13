#include "UI/ProjectSocialCardWidget.h"

#include "Fonts/SlateFontInfo.h"
#include "Styling/CoreStyle.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float ProjectSocialCardPanelWidth = 400.0f;
	constexpr float ProjectSocialCardPanelMinHeight = 276.0f;
	const FVector2D ProjectSocialCardViewportPosition(8.0f, 270.0f);

	const FLinearColor ProjectSocialCardOuterColor(0.03f, 0.03f, 0.04f, 0.96f);
	const FLinearColor ProjectSocialCardInnerColor(0.09f, 0.09f, 0.11f, 0.98f);
	const FLinearColor ProjectSocialCardFrameColor(0.86f, 0.70f, 0.28f, 0.82f);
	const FLinearColor ProjectSocialCardTitleColor(0.95f, 0.84f, 0.38f, 1.0f);
	const FLinearColor ProjectSocialCardLabelColor(0.84f, 0.84f, 0.88f, 1.0f);
	const FLinearColor ProjectSocialCardValueColor(0.97f, 0.97f, 0.99f, 1.0f);
	const FLinearColor ProjectSocialCardMutedValueColor(0.62f, 0.62f, 0.68f, 1.0f);

	FSlateFontInfo MakeSocialCardFont(const TCHAR* Weight, const int32 Size)
	{
		return FCoreStyle::GetDefaultFontStyle(Weight, Size);
	}

	FText BuildSocialCardValueText(const FProjectEnemyCombatStatRow& Row)
	{
		if (!Row.ValueOverride.IsEmpty())
		{
			return Row.ValueOverride;
		}

		if (!Row.bIsAvailable)
		{
			return FText::FromString(TEXT("--"));
		}

		return FText::FromString(FString::Printf(TEXT("%.0f -> %.0f"), Row.BaseValue, Row.FinalValue));
	}
}

class SProjectSocialCardPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SProjectSocialCardPanel)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		ChildSlot
		[
			SNew(SBox)
			.WidthOverride(ProjectSocialCardPanelWidth)
			.MinDesiredHeight(ProjectSocialCardPanelMinHeight)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(ProjectSocialCardFrameColor)
				.Padding(FMargin(2.0f))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(ProjectSocialCardOuterColor)
					.Padding(FMargin(4.0f))
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(ProjectSocialCardInnerColor)
						.Padding(FMargin(6.0f, 6.0f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Social Card")))
								.Font(MakeSocialCardFont(TEXT("Bold"), 14))
								.ColorAndOpacity(ProjectSocialCardTitleColor)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
							[
								SAssignNew(RowsBox, SVerticalBox)
							]
						]
					]
				]
			]
		];
	}

	void SetSocialCardSnapshot(const FProjectSocialCardSnapshot& Snapshot)
	{
		if (!RowsBox.IsValid())
		{
			return;
		}

		RowsBox->ClearChildren();
		for (const FProjectEnemyCombatStatRow& Row : Snapshot.Rows)
		{
			RowsBox->AddSlot()
			.AutoHeight()
			.Padding(FMargin(3.0f, 0.0f))
			[
				BuildRow(Row)
			];
		}
	}

private:
	TSharedRef<SWidget> BuildRow(const FProjectEnemyCombatStatRow& Row) const
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.61f)
			.VAlign(VAlign_Center)
			.Padding(FMargin(0.0f, 0.0f, 4.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(Row.Label.IsEmpty() ? FText::FromString(TEXT("--")) : Row.Label)
				.Font(MakeSocialCardFont(TEXT("Bold"), 12))
				.ColorAndOpacity(ProjectSocialCardLabelColor)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.39f)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(BuildSocialCardValueText(Row))
				.Font(MakeSocialCardFont(TEXT("Regular"), 12))
				.ColorAndOpacity(Row.bIsAvailable ? ProjectSocialCardValueColor : ProjectSocialCardMutedValueColor)
				.Justification(ETextJustify::Right)
			];
	}

private:
	TSharedPtr<SVerticalBox> RowsBox;
};

UProjectSocialCardWidget::UProjectSocialCardWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CachedSnapshot.Rows.Reserve(10);
}

TSharedRef<SWidget> UProjectSocialCardWidget::RebuildWidget()
{
	SAssignNew(SocialCardPanel, SProjectSocialCardPanel);
	ApplyCachedSnapshot();
	SetVisibility(bHudVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	return SocialCardPanel.ToSharedRef();
}

void UProjectSocialCardWidget::ReleaseSlateResources(const bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	SocialCardPanel.Reset();
}

void UProjectSocialCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ApplyFixedViewportPlacement();
	SetVisibility(bHudVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	ApplyCachedSnapshot();
}

void UProjectSocialCardWidget::SetSocialCardSnapshot(const FProjectSocialCardSnapshot& InSnapshot)
{
	CachedSnapshot = InSnapshot;
	ApplyCachedSnapshot();
}

void UProjectSocialCardWidget::SetHudVisible(const bool bVisible)
{
	bHudVisible = bVisible;
	SetVisibility(bHudVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (SocialCardPanel.IsValid())
	{
		SocialCardPanel->SetVisibility(bHudVisible ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
	}
}

bool UProjectSocialCardWidget::IsHudVisible() const
{
	return bHudVisible;
}

void UProjectSocialCardWidget::ApplyCachedSnapshot()
{
	if (SocialCardPanel.IsValid())
	{
		SocialCardPanel->SetSocialCardSnapshot(CachedSnapshot);
	}
}

void UProjectSocialCardWidget::ApplyFixedViewportPlacement()
{
	SetDesiredSizeInViewport(FVector2D(ProjectSocialCardPanelWidth, ProjectSocialCardPanelMinHeight));
	SetAnchorsInViewport(FAnchors(0.0f, 0.0f));
	SetAlignmentInViewport(FVector2D(0.0f, 0.0f));
	SetPositionInViewport(ProjectSocialCardViewportPosition, false);
}
