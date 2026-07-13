#include "UI/ProjectEmoteMenuOptionWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "ProjectEmoteMenuOptionWidget"

namespace ProjectEmoteMenuOptionWidgetPrivate
{
	const FLinearColor RowColor(0.30f, 0.08f, 0.24f, 0.66f);
	const FLinearColor RowInnerColor(0.022f, 0.010f, 0.032f, 0.94f);
	const FLinearColor RowFrameColor(0.74f, 0.25f, 0.55f, 0.26f);
	const FLinearColor RowGlowColor(1.0f, 0.18f, 0.62f, 0.18f);
	const FLinearColor DisabledRowColor(0.020f, 0.015f, 0.028f, 0.50f);
	const FLinearColor SelectedRowColor(0.23f, 0.030f, 0.135f, 0.96f);
	const FLinearColor SelectedBorderColor(1.0f, 0.28f, 0.68f, 1.0f);
	const FLinearColor TextColor(0.96f, 0.78f, 0.92f, 1.0f);
	const FLinearColor SelectedTextColor(1.0f, 0.62f, 0.84f, 1.0f);
	const FLinearColor DescriptionColor(0.83f, 0.73f, 0.84f, 0.96f);
	const FLinearColor DisabledTextColor(0.44f, 0.36f, 0.49f, 0.74f);
	const FLinearColor IconWellColor(0.055f, 0.018f, 0.070f, 0.74f);
	const FLinearColor ShadowColor(0.0f, 0.0f, 0.0f, 0.52f);

	const TCHAR* RowFrameTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_RowFrame.T_SinfulAscension_Altar_RowFrame");
	const TCHAR* PersonTexturePath = TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Person.T_LADS_Icon_Person");

	UObject* GetDesignerSafeDefaultFontObject()
	{
		static TWeakObjectPtr<UObject> CachedFontObject;
		if (!CachedFontObject.IsValid())
		{
			CachedFontObject = LoadObject<UObject>(nullptr, TEXT("/Engine/EngineFonts/Roboto.Roboto"));
		}

		return CachedFontObject.Get();
	}

	UTexture2D* LoadTextureAsset(const TCHAR* AssetPath)
	{
		return AssetPath && AssetPath[0] != 0
			? LoadObject<UTexture2D>(nullptr, AssetPath)
			: nullptr;
	}

	FProjectEmoteMenuOption MakeRootPreviewOption(
		const FName OptionId,
		const FText& Label,
		const FText& Description,
		const FName VisualIconId,
		const EProjectEmoteMenuNodeType NodeType = EProjectEmoteMenuNodeType::Folder)
	{
		FProjectEmoteMenuOption Option;
		Option.OptionId = OptionId;
		Option.Label = Label;
		Option.Description = Description;
		Option.bEnabled = true;
		Option.NodeType = NodeType;
		Option.VisualIconId = VisualIconId;
		Option.VisualAttribute = TEXT("None");
		return Option;
	}

	const TCHAR* ResolveRootPreviewIconPath(const FName VisualIconId)
	{
		if (VisualIconId == TEXT("Actions"))
		{
			return TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Actions.T_LADS_Icon_Actions");
		}
		if (VisualIconId == TEXT("Objects"))
		{
			return TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Objects.T_LADS_Icon_Objects");
		}
		if (VisualIconId == TEXT("Social"))
		{
			return TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Social.T_LADS_Icon_Social");
		}
		if (VisualIconId == TEXT("Special"))
		{
			return TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Special.T_LADS_Icon_Special");
		}
		if (VisualIconId == TEXT("Cancel"))
		{
			return TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Cancel.T_LADS_Icon_Cancel");
		}

		return TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Default.T_SinfulAscension_Altar_Icon_Default");
	}

	void ConfigureRootRowDefaults(
		UProjectEmoteMenuOptionWidget& RowWidget,
		const FProjectEmoteMenuOption& PreviewOption,
		const int32 PreviewIndex)
	{
		UTexture2D* PreviewIcon = LoadTextureAsset(ResolveRootPreviewIconPath(PreviewOption.VisualIconId));
		RowWidget.SetDesignerPreviewOption(PreviewOption, EProjectEmoteMenuVisualMode::Root, PreviewIndex);
		RowWidget.SetUseDesignerIconOverride(true);
		RowWidget.SetDesignerIconOverride(PreviewIcon);
		RowWidget.ConfigureOption(
			PreviewOption,
			EProjectEmoteMenuVisualMode::Root,
			PreviewIndex,
			82.0f,
			PreviewIcon,
			SelectedBorderColor,
			true);
	}
}

UProjectEmoteMenuOptionWidget::UProjectEmoteMenuOptionWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DesignerPreviewOption.OptionId = TEXT("Designer.Option");
	DesignerPreviewOption.Label = LOCTEXT("DefaultDesignerPreviewLabel", "Sample Action");
	DesignerPreviewOption.Description = LOCTEXT("DefaultDesignerPreviewDescription", "Editable row state preview");
	DesignerPreviewOption.bEnabled = true;
	DesignerPreviewOption.NodeType = EProjectEmoteMenuNodeType::Action;
	DesignerPreviewOption.VisualIconId = TEXT("Actions");
	DesignerPreviewOption.VisualAttribute = TEXT("None");
}

UProjectEmoteMenuActionsRowWidget::UProjectEmoteMenuActionsRowWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectEmoteMenuOptionWidgetPrivate::ConfigureRootRowDefaults(
		*this,
		ProjectEmoteMenuOptionWidgetPrivate::MakeRootPreviewOption(
			TEXT("Root.Actions"),
			LOCTEXT("ActionsRowPreviewLabel", "Actions"),
			LOCTEXT("ActionsRowPreviewDescription", "Open action categories."),
			TEXT("Actions")),
		0);
}

UProjectEmoteMenuObjectsRowWidget::UProjectEmoteMenuObjectsRowWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectEmoteMenuOptionWidgetPrivate::ConfigureRootRowDefaults(
		*this,
		ProjectEmoteMenuOptionWidgetPrivate::MakeRootPreviewOption(
			TEXT("Root.Objects"),
			LOCTEXT("ObjectsRowPreviewLabel", "Objects"),
			LOCTEXT("ObjectsRowPreviewDescription", "Interact with inventory"),
			TEXT("Objects")),
		1);
}

UProjectEmoteMenuSocialRowWidget::UProjectEmoteMenuSocialRowWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectEmoteMenuOptionWidgetPrivate::ConfigureRootRowDefaults(
		*this,
		ProjectEmoteMenuOptionWidgetPrivate::MakeRootPreviewOption(
			TEXT("Root.Social"),
			LOCTEXT("SocialRowPreviewLabel", "Social"),
			LOCTEXT("SocialRowPreviewDescription", "Social interactions and reactions."),
			TEXT("Social")),
		2);
}

UProjectEmoteMenuSpecialRowWidget::UProjectEmoteMenuSpecialRowWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectEmoteMenuOptionWidgetPrivate::ConfigureRootRowDefaults(
		*this,
		ProjectEmoteMenuOptionWidgetPrivate::MakeRootPreviewOption(
			TEXT("Root.Special"),
			LOCTEXT("SpecialRowPreviewLabel", "Special"),
			LOCTEXT("SpecialRowPreviewDescription", "Unique and situational actions."),
			TEXT("Special")),
		3);
}

UProjectEmoteMenuCancelRowWidget::UProjectEmoteMenuCancelRowWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectEmoteMenuOptionWidgetPrivate::ConfigureRootRowDefaults(
		*this,
		ProjectEmoteMenuOptionWidgetPrivate::MakeRootPreviewOption(
			TEXT("Root.Cancel"),
			LOCTEXT("CancelRowPreviewLabel", "Cancel"),
			LOCTEXT("CancelRowPreviewDescription", "Close the interaction menu."),
			TEXT("Cancel"),
			EProjectEmoteMenuNodeType::Cancel),
		4);
}

void UProjectEmoteMenuOptionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshOptionData();
	RefreshVisualState();
}

void UProjectEmoteMenuOptionWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	BuildWidgetTree();
	ApplyDesignerPreviewOptionIfNeeded();
	RefreshOptionData();
	RefreshVisualState();
}

void UProjectEmoteMenuOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetTree();
	RefreshOptionData();
	RefreshVisualState();
}

bool UProjectEmoteMenuOptionWidget::BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree)
	{
		return false;
	}

	MenuOption.OptionId = TEXT("Designer.Option");
	MenuOption.Label = LOCTEXT("DesignerOptionLabel", "Sample Action");
	MenuOption.Description = LOCTEXT("DesignerOptionDescription", "Editable row state preview");
	MenuOption.bEnabled = true;
	MenuOption.NodeType = EProjectEmoteMenuNodeType::Action;
	MenuOption.VisualIconId = TEXT("Actions");
	MenuOption.VisualAttribute = TEXT("Allure");
	MenuOption.RequiredExtraNpcCount = 1;
	VisualMode = EProjectEmoteMenuVisualMode::AnimationList;
	RowHeight = 82.0f;
	AccentColor = ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor;
	DesignerPreviewOption = MenuOption;
	DesignerPreviewVisualMode = VisualMode;

	const bool bBuiltTree = BuildDefaultOptionTree(TargetWidgetTree);
	RefreshOptionData();
	RefreshVisualState();
	return bBuiltTree;
}

void UProjectEmoteMenuOptionWidget::ConfigureOption(
	const FProjectEmoteMenuOption& InOption,
	const EProjectEmoteMenuVisualMode InVisualMode,
	const int32 InOptionIndex,
	const float InRowHeight,
	UTexture2D* InIconTexture,
	const FLinearColor& InAccentColor,
	const bool bInUseDesignerIconOverride)
{
	MenuOption = InOption;
	VisualMode = InVisualMode;
	OptionIndex = InOptionIndex;
	RowHeight = InRowHeight;
	IconTexture = InIconTexture;
	AccentColor = InAccentColor;
	bOptionEnabled = InOption.bEnabled;
	bUseDesignerIconOverride = bInUseDesignerIconOverride;
	RefreshOptionData();
	RefreshVisualState();
}

void UProjectEmoteMenuOptionWidget::SetOptionVisualState(const bool bInSelected, const bool bInEnabled)
{
	bSelected = bInSelected;
	bOptionEnabled = bInEnabled;
	RefreshVisualState();
}

void UProjectEmoteMenuOptionWidget::SetDesignerIconOverride(UTexture2D* InDesignerIconOverride)
{
	DesignerIconOverride = InDesignerIconOverride;
	RefreshOptionData();
}

void UProjectEmoteMenuOptionWidget::SetUseDesignerIconOverride(const bool bInUseDesignerIconOverride)
{
	bUseDesignerIconOverride = bInUseDesignerIconOverride;
	RefreshOptionData();
}

void UProjectEmoteMenuOptionWidget::SetDesignerPreviewOption(
	const FProjectEmoteMenuOption& InDesignerPreviewOption,
	const EProjectEmoteMenuVisualMode InDesignerPreviewVisualMode,
	const int32 InDesignerPreviewIndex)
{
	DesignerPreviewOption = InDesignerPreviewOption;
	DesignerPreviewVisualMode = InDesignerPreviewVisualMode;
	if (IsDesignTime() && MenuOption.Label.IsEmpty())
	{
		MenuOption = DesignerPreviewOption;
		VisualMode = DesignerPreviewVisualMode;
		OptionIndex = InDesignerPreviewIndex;
		bOptionEnabled = DesignerPreviewOption.bEnabled;
	}
	RefreshOptionData();
	RefreshVisualState();
}

void UProjectEmoteMenuOptionWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (WidgetTree->RootWidget)
	{
		bUsingNativeFallbackTree = false;
		return;
	}

	bUsingNativeFallbackTree = true;
	BuildDefaultOptionTree(WidgetTree);
}

bool UProjectEmoteMenuOptionWidget::BuildDefaultOptionTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree || TargetWidgetTree->RootWidget)
	{
		return false;
	}

	RowSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RowSizeBox"));
	RowSizeBox->SetHeightOverride(RowHeight);
	TargetWidgetTree->RootWidget = RowSizeBox;

	RowOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RowOverlay"));
	RowSizeBox->SetContent(RowOverlay);

	OptionGlowBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OptionGlowBorder"));
	OptionGlowBorder->SetBrushColor(ProjectEmoteMenuOptionWidgetPrivate::RowGlowColor);
	OptionGlowBorder->SetVisibility(ESlateVisibility::Hidden);
	if (UOverlaySlot* GlowSlot = RowOverlay->AddChildToOverlay(OptionGlowBorder))
	{
		GlowSlot->SetHorizontalAlignment(HAlign_Fill);
		GlowSlot->SetVerticalAlignment(VAlign_Fill);
	}

	OptionSelectionFrame = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OptionSelectionFrame"));
	OptionSelectionFrame->SetBrushColor(ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor);
	OptionSelectionFrame->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (UOverlaySlot* SelectionSlot = RowOverlay->AddChildToOverlay(OptionSelectionFrame))
	{
		SelectionSlot->SetHorizontalAlignment(HAlign_Fill);
		SelectionSlot->SetVerticalAlignment(VAlign_Fill);
		SelectionSlot->SetPadding(FMargin(1.0f));
	}

	OptionFrameImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("OptionFrameImage"));
	if (UTexture2D* RowFrameTexture = LoadTextureByPath(ProjectEmoteMenuOptionWidgetPrivate::RowFrameTexturePath))
	{
		OptionFrameImage->SetBrushFromTexture(RowFrameTexture, true);
	}
	OptionFrameImage->SetColorAndOpacity(ProjectEmoteMenuOptionWidgetPrivate::RowFrameColor);
	OptionFrameImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (UOverlaySlot* FrameSlot = RowOverlay->AddChildToOverlay(OptionFrameImage))
	{
		FrameSlot->SetHorizontalAlignment(HAlign_Fill);
		FrameSlot->SetVerticalAlignment(VAlign_Fill);
		FrameSlot->SetPadding(FMargin(1.0f));
	}

	OptionBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OptionBorder"));
	OptionBorder->SetBrushColor(ProjectEmoteMenuOptionWidgetPrivate::RowColor);
	OptionBorder->SetPadding(FMargin(1.0f));
	if (UOverlaySlot* BorderSlot = RowOverlay->AddChildToOverlay(OptionBorder))
	{
		BorderSlot->SetHorizontalAlignment(HAlign_Fill);
		BorderSlot->SetVerticalAlignment(VAlign_Fill);
		BorderSlot->SetPadding(FMargin(3.0f));
	}

	OptionInnerBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OptionInnerBorder"));
	OptionInnerBorder->SetBrushColor(ProjectEmoteMenuOptionWidgetPrivate::RowInnerColor);
	OptionInnerBorder->SetPadding(FMargin(14.0f, 10.0f));
	OptionBorder->SetContent(OptionInnerBorder);

	UHorizontalBox* RowBox = TargetWidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("OptionRowBox"));
	OptionInnerBorder->SetContent(RowBox);

	SelectorText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SelectorText"));
	SelectorText->SetText(FText::FromString(TEXT(">>")));
	SelectorText->SetJustification(ETextJustify::Center);
	SelectorText->SetColorAndOpacity(FSlateColor(ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor));
	SelectorText->SetFont(MakeTitleFont(26));
	if (UHorizontalBoxSlot* SelectorSlot = RowBox->AddChildToHorizontalBox(SelectorText))
	{
		SelectorSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		SelectorSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));
		SelectorSlot->SetVerticalAlignment(VAlign_Center);
	}

	USizeBox* IconSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("OptionIconSizeBox"));
	IconSizeBox->SetWidthOverride(70.0f);
	IconSizeBox->SetHeightOverride(70.0f);
	if (UHorizontalBoxSlot* IconSlot = RowBox->AddChildToHorizontalBox(IconSizeBox))
	{
		IconSlot->SetPadding(FMargin(0.0f, 0.0f, 22.0f, 0.0f));
		IconSlot->SetVerticalAlignment(VAlign_Center);
	}

	OptionIconBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OptionIconBorder"));
	OptionIconBorder->SetBrushColor(ProjectEmoteMenuOptionWidgetPrivate::IconWellColor);
	OptionIconBorder->SetPadding(FMargin(7.0f));
	IconSizeBox->SetContent(OptionIconBorder);

	OptionIconImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("OptionIconImage"));
	OptionIconImage->SetColorAndOpacity(AccentColor);
	OptionIconBorder->SetContent(OptionIconImage);

	UVerticalBox* TextColumn = TargetWidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OptionTextColumn"));
	if (UHorizontalBoxSlot* TextSlot = RowBox->AddChildToHorizontalBox(TextColumn))
	{
		TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TextSlot->SetVerticalAlignment(VAlign_Center);
	}

	OptionLabelText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OptionLabelText"));
	OptionLabelText->SetText(LOCTEXT("FallbackLabel", "SAMPLE ACTION"));
	OptionLabelText->SetColorAndOpacity(FSlateColor(ProjectEmoteMenuOptionWidgetPrivate::TextColor));
	OptionLabelText->SetFont(MakeTitleFont(22));
	OptionLabelText->SetShadowOffset(FVector2D(0.0f, 0.45f));
	OptionLabelText->SetShadowColorAndOpacity(ProjectEmoteMenuOptionWidgetPrivate::ShadowColor.CopyWithNewOpacity(0.36f));
	if (UVerticalBoxSlot* LabelSlot = TextColumn->AddChildToVerticalBox(OptionLabelText))
	{
		LabelSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	}

	OptionDescriptionText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OptionDescriptionText"));
	OptionDescriptionText->SetText(LOCTEXT("FallbackDescription", "Editable row state preview"));
	OptionDescriptionText->SetAutoWrapText(true);
	OptionDescriptionText->SetColorAndOpacity(FSlateColor(ProjectEmoteMenuOptionWidgetPrivate::DescriptionColor));
	OptionDescriptionText->SetFont(MakeBodyFont(18));
	OptionDescriptionText->SetShadowOffset(FVector2D(0.0f, 0.25f));
	OptionDescriptionText->SetShadowColorAndOpacity(ProjectEmoteMenuOptionWidgetPrivate::ShadowColor.CopyWithNewOpacity(0.20f));
	TextColumn->AddChildToVerticalBox(OptionDescriptionText);

	OptionArrowText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OptionArrowText"));
	OptionArrowText->SetText(FText::FromString(TEXT(">")));
	OptionArrowText->SetJustification(ETextJustify::Center);
	OptionArrowText->SetColorAndOpacity(FSlateColor(ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor));
	OptionArrowText->SetFont(MakeTitleFont(32));
	if (UHorizontalBoxSlot* ArrowSlot = RowBox->AddChildToHorizontalBox(OptionArrowText))
	{
		ArrowSlot->SetPadding(FMargin(18.0f, 0.0f, 0.0f, 0.0f));
		ArrowSlot->SetVerticalAlignment(VAlign_Center);
	}

	NpcBox = TargetWidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("NpcBox"));
	NpcBox->SetVisibility(ESlateVisibility::Hidden);
	if (UHorizontalBoxSlot* NpcBoxSlot = RowBox->AddChildToHorizontalBox(NpcBox))
	{
		NpcBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		NpcBoxSlot->SetPadding(FMargin(18.0f, 0.0f, 0.0f, 0.0f));
		NpcBoxSlot->SetVerticalAlignment(VAlign_Center);
	}

	NpcNumberText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NpcNumberText"));
	NpcNumberText->SetText(FText::AsNumber(1));
	NpcNumberText->SetColorAndOpacity(FSlateColor(ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor));
	NpcNumberText->SetFont(MakeTitleFont(24));
	if (UHorizontalBoxSlot* NpcNumberSlot = NpcBox->AddChildToHorizontalBox(NpcNumberText))
	{
		NpcNumberSlot->SetPadding(FMargin(0.0f, 0.0f, 9.0f, 0.0f));
		NpcNumberSlot->SetVerticalAlignment(VAlign_Center);
	}

	USizeBox* PersonIconSize = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("NpcIconSizeBox"));
	PersonIconSize->SetWidthOverride(25.0f);
	PersonIconSize->SetHeightOverride(25.0f);
	if (UHorizontalBoxSlot* PersonIconSlot = NpcBox->AddChildToHorizontalBox(PersonIconSize))
	{
		PersonIconSlot->SetVerticalAlignment(VAlign_Center);
	}

	UOverlay* PersonIconOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("NpcIconOverlay"));
	PersonIconSize->SetContent(PersonIconOverlay);

	NpcIconImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("NpcIconImage"));
	if (UTexture2D* PersonTexture = LoadTextureByPath(ProjectEmoteMenuOptionWidgetPrivate::PersonTexturePath))
	{
		NpcIconImage->SetBrushFromTexture(PersonTexture, true);
	}
	NpcIconImage->SetColorAndOpacity(ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor);
	PersonIconOverlay->AddChildToOverlay(NpcIconImage);

	NpcFallbackText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NpcFallbackText"));
	NpcFallbackText->SetText(FText::FromString(TEXT("P")));
	NpcFallbackText->SetJustification(ETextJustify::Center);
	NpcFallbackText->SetColorAndOpacity(FSlateColor(ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor));
	NpcFallbackText->SetFont(MakeTitleFont(22));
	NpcFallbackText->SetVisibility(ESlateVisibility::Hidden);
	if (UOverlaySlot* FallbackSlot = PersonIconOverlay->AddChildToOverlay(NpcFallbackText))
	{
		FallbackSlot->SetHorizontalAlignment(HAlign_Fill);
		FallbackSlot->SetVerticalAlignment(VAlign_Center);
	}

	OptionDisabledOverlay = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OptionDisabledOverlay"));
	OptionDisabledOverlay->SetBrushColor(ProjectEmoteMenuOptionWidgetPrivate::DisabledRowColor);
	OptionDisabledOverlay->SetVisibility(ESlateVisibility::Hidden);
	if (UOverlaySlot* DisabledSlot = RowOverlay->AddChildToOverlay(OptionDisabledOverlay))
	{
		DisabledSlot->SetHorizontalAlignment(HAlign_Fill);
		DisabledSlot->SetVerticalAlignment(VAlign_Fill);
		DisabledSlot->SetPadding(FMargin(3.0f));
	}

	return true;
}

void UProjectEmoteMenuOptionWidget::ApplyDesignerPreviewOptionIfNeeded()
{
	if (!IsDesignTime() || !MenuOption.Label.IsEmpty())
	{
		return;
	}

	MenuOption = DesignerPreviewOption;
	VisualMode = DesignerPreviewVisualMode;
	bOptionEnabled = DesignerPreviewOption.bEnabled;
}

void UProjectEmoteMenuOptionWidget::RefreshOptionData()
{
	const bool bAnimationEntry = IsAnimationEntry();

	if (RowSizeBox && bUsingNativeFallbackTree)
	{
		RowSizeBox->SetHeightOverride(RowHeight);
	}

	if (OptionInnerBorder && bUsingNativeFallbackTree)
	{
		OptionInnerBorder->SetPadding(bAnimationEntry ? FMargin(12.0f, 6.0f) : FMargin(14.0f, 10.0f));
	}

	if (OptionLabelText)
	{
		FString LabelString = MenuOption.Label.ToString();
		if (!bAnimationEntry)
		{
			LabelString.ToUpperInline();
		}
		OptionLabelText->SetText(FText::FromString(LabelString));

		if (bUsingNativeFallbackTree)
		{
			OptionLabelText->SetFont(MakeTitleFont(22));
		}
	}

	if (OptionDescriptionText)
	{
		OptionDescriptionText->SetText(MenuOption.Description.IsEmpty() ? FText::GetEmpty() : MenuOption.Description);
		if (bUsingNativeFallbackTree)
		{
			OptionDescriptionText->SetFont(MakeBodyFont(18));
		}
	}

	UTexture2D* EffectiveIconTexture = IconTexture;
	if (bUseDesignerIconOverride)
	{
		EffectiveIconTexture = DesignerIconOverride;
	}

	if (OptionIconImage && EffectiveIconTexture)
	{
		OptionIconImage->SetBrushFromTexture(EffectiveIconTexture, true);
	}

	if (OptionArrowText)
	{
		if (bUsingNativeFallbackTree)
		{
			OptionArrowText->SetText(IsNavigationOption() ? FText::FromString(TEXT(">")) : FText::GetEmpty());
		}
		OptionArrowText->SetVisibility(!bAnimationEntry && IsNavigationOption() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (NpcBox)
	{
		NpcBox->SetVisibility(bAnimationEntry ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (NpcNumberText)
	{
		NpcNumberText->SetText(FText::AsNumber(FMath::Clamp(MenuOption.RequiredExtraNpcCount, 0, 3)));
	}

	if (NpcIconImage)
	{
		NpcIconImage->SetVisibility(NpcIconImage->GetBrush().GetResourceObject() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	if (NpcFallbackText)
	{
		NpcFallbackText->SetVisibility((NpcIconImage && NpcIconImage->GetVisibility() != ESlateVisibility::Hidden) ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}

	OnOptionDataChanged(MenuOption, VisualMode, bAnimationEntry, EffectiveIconTexture, AccentColor);
}

void UProjectEmoteMenuOptionWidget::RefreshVisualState()
{
	const bool bEnabled = bOptionEnabled && MenuOption.bEnabled;

	if (bUsingNativeFallbackTree)
	{
		SetIsEnabled(bEnabled);
	}

	if (OptionSelectionFrame)
	{
		OptionSelectionFrame->SetVisibility(bEnabled && bSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	if (OptionGlowBorder)
	{
		OptionGlowBorder->SetVisibility(bEnabled && bSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	if (OptionDisabledOverlay)
	{
		OptionDisabledOverlay->SetVisibility(bEnabled ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}

	if (SelectorText)
	{
		if (bUsingNativeFallbackTree)
		{
			SelectorText->SetText(bSelected ? FText::FromString(TEXT(">>")) : FText::GetEmpty());
		}
		SelectorText->SetVisibility(bSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	if (bUsingNativeFallbackTree)
	{
		if (UBorder* Border = OptionBorder)
		{
			Border->SetBrushColor(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::DisabledRowColor
				: (bSelected ? ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor : ProjectEmoteMenuOptionWidgetPrivate::RowColor));
		}

		if (UBorder* InnerBorder = OptionInnerBorder)
		{
			InnerBorder->SetBrushColor(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::DisabledRowColor
				: (bSelected ? ProjectEmoteMenuOptionWidgetPrivate::SelectedRowColor : ProjectEmoteMenuOptionWidgetPrivate::RowInnerColor));
		}

		if (UBorder* IconBorder = OptionIconBorder)
		{
			IconBorder->SetBrushColor(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::DisabledRowColor
				: (bSelected ? FLinearColor(0.24f, 0.035f, 0.15f, 0.92f) : ProjectEmoteMenuOptionWidgetPrivate::IconWellColor));
		}

		if (UImage* FrameImage = OptionFrameImage)
		{
			FrameImage->SetColorAndOpacity(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::RowFrameColor.CopyWithNewOpacity(0.18f)
				: (bSelected ? ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor.CopyWithNewOpacity(0.90f) : ProjectEmoteMenuOptionWidgetPrivate::RowFrameColor));
		}

		if (UTextBlock* Selector = SelectorText)
		{
			Selector->SetColorAndOpacity(FSlateColor(bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor
				: ProjectEmoteMenuOptionWidgetPrivate::DisabledTextColor));
		}

		if (UImage* IconImage = OptionIconImage)
		{
			IconImage->SetColorAndOpacity(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::DisabledTextColor
				: (bSelected ? ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor : AccentColor));
			IconImage->SetRenderOpacity(bEnabled ? 1.0f : 0.45f);
		}

		if (UTextBlock* Label = OptionLabelText)
		{
			Label->SetColorAndOpacity(FSlateColor(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::DisabledTextColor
				: (bSelected ? ProjectEmoteMenuOptionWidgetPrivate::SelectedTextColor : ProjectEmoteMenuOptionWidgetPrivate::TextColor)));
		}

		if (UTextBlock* Description = OptionDescriptionText)
		{
			Description->SetColorAndOpacity(FSlateColor(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::DisabledTextColor
				: (bSelected ? FLinearColor(0.96f, 0.78f, 0.90f, 1.0f) : ProjectEmoteMenuOptionWidgetPrivate::DescriptionColor)));
		}

		if (UTextBlock* Arrow = OptionArrowText)
		{
			Arrow->SetColorAndOpacity(FSlateColor(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::DisabledTextColor
				: (bSelected ? ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor : FLinearColor(0.96f, 0.34f, 0.72f, 0.92f))));
		}

		if (UTextBlock* NpcNumber = NpcNumberText)
		{
			NpcNumber->SetColorAndOpacity(FSlateColor(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::DisabledTextColor
				: (bSelected ? ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor : ProjectEmoteMenuOptionWidgetPrivate::SelectedTextColor)));
		}

		if (UImage* NpcIcon = NpcIconImage)
		{
			NpcIcon->SetColorAndOpacity(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::DisabledTextColor
				: (bSelected ? ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor : ProjectEmoteMenuOptionWidgetPrivate::SelectedTextColor));
		}

		if (UTextBlock* NpcFallback = NpcFallbackText)
		{
			NpcFallback->SetColorAndOpacity(FSlateColor(!bEnabled
				? ProjectEmoteMenuOptionWidgetPrivate::DisabledTextColor
				: (bSelected ? ProjectEmoteMenuOptionWidgetPrivate::SelectedBorderColor : ProjectEmoteMenuOptionWidgetPrivate::SelectedTextColor)));
		}
	}

	OnOptionVisualStateChanged(bSelected, bEnabled);
}

bool UProjectEmoteMenuOptionWidget::IsAnimationEntry() const
{
	return VisualMode == EProjectEmoteMenuVisualMode::AnimationList
		&& MenuOption.NodeType == EProjectEmoteMenuNodeType::Action;
}

bool UProjectEmoteMenuOptionWidget::IsNavigationOption() const
{
	return MenuOption.NodeType == EProjectEmoteMenuNodeType::Folder;
}

FSlateFontInfo UProjectEmoteMenuOptionWidget::MakeTitleFont(const int32 Size) const
{
	if (UObject* FontObject = ProjectEmoteMenuOptionWidgetPrivate::GetDesignerSafeDefaultFontObject())
	{
		return FSlateFontInfo(FontObject, Size, FName(TEXT("Bold")));
	}

	return FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), Size);
}

FSlateFontInfo UProjectEmoteMenuOptionWidget::MakeBodyFont(const int32 Size) const
{
	if (UObject* FontObject = ProjectEmoteMenuOptionWidgetPrivate::GetDesignerSafeDefaultFontObject())
	{
		return FSlateFontInfo(FontObject, Size, FName(TEXT("Regular")));
	}

	return FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Size);
}

UTexture2D* UProjectEmoteMenuOptionWidget::LoadTextureByPath(const TCHAR* TexturePath) const
{
	return TexturePath ? LoadObject<UTexture2D>(nullptr, TexturePath) : nullptr;
}

#undef LOCTEXT_NAMESPACE
