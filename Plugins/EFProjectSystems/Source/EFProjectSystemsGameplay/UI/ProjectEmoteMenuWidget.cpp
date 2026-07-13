#include "UI/ProjectEmoteMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "UI/ProjectEmoteMenuOptionWidget.h"
#include "UI/ProjectWidgetClassResolver.h"

#define LOCTEXT_NAMESPACE "ProjectEmoteMenuWidget"

namespace ProjectEmoteMenuWidgetPrivate
{
	const FLinearColor BackdropColor(0.0f, 0.0f, 0.0f, 0.52f);
	const FLinearColor PanelOuterColor(0.95f, 0.31f, 0.63f, 0.86f);
	const FLinearColor PanelFrameColor(1.0f, 0.24f, 0.62f, 0.42f);
	const FLinearColor PanelGlowColor(1.0f, 0.16f, 0.58f, 0.16f);
	const FLinearColor PanelInnerColor(0.014f, 0.007f, 0.025f, 1.0f);
	const FLinearColor PanelHazeColor(1.0f, 0.40f, 0.78f, 0.075f);
	const FLinearColor DividerColor(1.0f, 0.30f, 0.68f, 0.72f);
	const FLinearColor TitleColor(1.0f, 0.48f, 0.76f, 1.0f);
	const FLinearColor HintColor(0.88f, 0.70f, 0.86f, 0.96f);
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
	const FLinearColor FooterColor(0.045f, 0.016f, 0.050f, 0.84f);
	const FLinearColor ShadowColor(0.0f, 0.0f, 0.0f, 0.52f);
	constexpr float InteractionPanelWidth = 620.0f;
	constexpr float InteractionPanelHeight = 1040.0f;
	constexpr float CategoryPanelWidth = 760.0f;
	constexpr float CategoryPanelHeight = 900.0f;
	constexpr float AnimationListPanelWidth = 800.0f;
	constexpr float AnimationListPanelHeight = 980.0f;
	constexpr float OptionRowGap = 12.0f;
	constexpr float PanelChromeHeight = 263.0f;
	constexpr float ViewportWidthLimit = 1.20f;
	constexpr float ViewportHeightLimit = 1.20f;
	constexpr float InteractionViewportHeightLimit = 1.35f;
	constexpr float FontScale = 0.80f;

	const TCHAR* FrameTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Frame.T_SinfulAscension_Altar_Frame");
	const TCHAR* HazeTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Haze.T_SinfulAscension_Altar_Haze");
	const TCHAR* DividerTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Divider.T_SinfulAscension_Altar_Divider");
	const TCHAR* FooterTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Footer.T_SinfulAscension_Altar_Footer");
	const TCHAR* GlyphTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Glyph.T_SinfulAscension_Altar_Glyph");
	static UObject* LoadStyleAsset(const TCHAR* AssetPath)
	{
		return AssetPath && AssetPath[0] != 0
			? StaticLoadObject(UObject::StaticClass(), nullptr, AssetPath)
			: nullptr;
	}

	static UTexture2D* LoadTextureAsset(const TCHAR* AssetPath)
	{
		return Cast<UTexture2D>(LoadStyleAsset(AssetPath));
	}

	static UObject* GetDesignerSafeDefaultFontObject()
	{
		static TWeakObjectPtr<UObject> CachedFontObject;
		if (!CachedFontObject.IsValid())
		{
			CachedFontObject = LoadObject<UObject>(nullptr, TEXT("/Engine/EngineFonts/Roboto.Roboto"));
		}

		return CachedFontObject.Get();
	}

	static FSlateFontInfo MakeTitleFont(const int32 Size)
	{
		const int32 ScaledSize = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Size) * FontScale));
		if (UObject* FontObject = GetDesignerSafeDefaultFontObject())
		{
			FSlateFontInfo FontInfo(FontObject, ScaledSize, FName(TEXT("Bold")));
			FontInfo.LetterSpacing = 0;
			return FontInfo;
		}

		FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), ScaledSize);
		FontInfo.Size = ScaledSize;
		FontInfo.LetterSpacing = 0;
		return FontInfo;
	}

	static FSlateFontInfo MakeBodyFont(const int32 Size)
	{
		const int32 ScaledSize = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Size) * FontScale));
		if (UObject* FontObject = GetDesignerSafeDefaultFontObject())
		{
			FSlateFontInfo FontInfo(FontObject, ScaledSize, FName(TEXT("Regular")));
			FontInfo.LetterSpacing = 0;
			return FontInfo;
		}

		FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), ScaledSize);
		FontInfo.Size = ScaledSize;
		FontInfo.LetterSpacing = 0;
		return FontInfo;
	}

	static FText ToUpperText(const FText& InText)
	{
		FString TextString = InText.ToString();
		TextString.ToUpperInline();
		return FText::FromString(TextString);
	}

	static FProjectEmoteMenuOption MakeRootPreviewOption(
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

	static TArray<FProjectEmoteMenuOption> MakeRootPreviewOptions()
	{
		TArray<FProjectEmoteMenuOption> PreviewOptions;
		PreviewOptions.Reserve(5);
		PreviewOptions.Add(MakeRootPreviewOption(
			TEXT("Root.Actions"),
			LOCTEXT("RootActionsPreviewLabel", "Actions"),
			LOCTEXT("RootActionsPreviewDescription", "Open action categories."),
			TEXT("Actions")));
		PreviewOptions.Add(MakeRootPreviewOption(
			TEXT("Root.Objects"),
			LOCTEXT("RootObjectsPreviewLabel", "Objects"),
			LOCTEXT("RootObjectsPreviewDescription", "Interact with inventory"),
			TEXT("Objects")));
		PreviewOptions.Add(MakeRootPreviewOption(
			TEXT("Root.Social"),
			LOCTEXT("RootSocialPreviewLabel", "Social"),
			LOCTEXT("RootSocialPreviewDescription", "Social interactions and reactions."),
			TEXT("Social")));
		PreviewOptions.Add(MakeRootPreviewOption(
			TEXT("Root.Special"),
			LOCTEXT("RootSpecialPreviewLabel", "Special"),
			LOCTEXT("RootSpecialPreviewDescription", "Unique and situational actions."),
			TEXT("Special")));
		PreviewOptions.Add(MakeRootPreviewOption(
			TEXT("Root.Cancel"),
			LOCTEXT("RootCancelPreviewLabel", "Cancel"),
			LOCTEXT("RootCancelPreviewDescription", "Close the interaction menu."),
			TEXT("Cancel"),
			EProjectEmoteMenuNodeType::Cancel));
		return PreviewOptions;
	}
}

UProjectEmoteMenuWidget::UProjectEmoteMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UProjectEmoteMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisualState();
}

void UProjectEmoteMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetTree();
	RefreshVisualState();
	FocusInitialOption();
}

FReply UProjectEmoteMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (HandleKeyNavigation(InKeyEvent.GetKey()))
	{
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

bool UProjectEmoteMenuWidget::BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree)
	{
		return false;
	}

	VisualMode = EProjectEmoteMenuVisualMode::Root;
	MenuOptions = ProjectEmoteMenuWidgetPrivate::MakeRootPreviewOptions();
	SelectedIndex = 0;
	bUsingNativeFallbackTree = true;
	WidgetTree = TargetWidgetTree;

	const bool bBuiltTree = BuildDefaultMenuTree(TargetWidgetTree);
	if (bBuiltTree)
	{
		RebuildOptionWidgets();
		RefreshVisualState();
	}

	return bBuiltTree;
}

void UProjectEmoteMenuWidget::GatherCodeWidgetDesignerChildWidgetClasses(TArray<TSubclassOf<UUserWidget>>& OutWidgetClasses) const
{
	OutWidgetClasses.Add(UProjectEmoteMenuOptionWidget::StaticClass());
	OutWidgetClasses.Add(UProjectEmoteMenuActionsRowWidget::StaticClass());
	OutWidgetClasses.Add(UProjectEmoteMenuObjectsRowWidget::StaticClass());
	OutWidgetClasses.Add(UProjectEmoteMenuSocialRowWidget::StaticClass());
	OutWidgetClasses.Add(UProjectEmoteMenuSpecialRowWidget::StaticClass());
	OutWidgetClasses.Add(UProjectEmoteMenuCancelRowWidget::StaticClass());
}

void UProjectEmoteMenuWidget::GatherCodeWidgetDesignerChildWidgetSpecs(TArray<FCodeWidgetDesignerChildWidgetSpec>& OutWidgetSpecs) const
{
	FCodeWidgetDesignerChildWidgetSpec GlobalRowSpec;
	GlobalRowSpec.WidgetClass = UProjectEmoteMenuOptionWidget::StaticClass();
	GlobalRowSpec.RelativeFolder = TEXT("Main");
	GlobalRowSpec.AssetNameOverride = TEXT("WBP_ProjectEmoteMenuOptionRow");
	OutWidgetSpecs.Add(GlobalRowSpec);

	const auto AddRootRowSpec = [&OutWidgetSpecs](TSubclassOf<UUserWidget> WidgetClass)
	{
		FCodeWidgetDesignerChildWidgetSpec Spec;
		Spec.WidgetClass = WidgetClass;
		Spec.RelativeFolder = TEXT("Interactions_Row");
		OutWidgetSpecs.Add(Spec);
	};

	AddRootRowSpec(UProjectEmoteMenuActionsRowWidget::StaticClass());
	AddRootRowSpec(UProjectEmoteMenuObjectsRowWidget::StaticClass());
	AddRootRowSpec(UProjectEmoteMenuSocialRowWidget::StaticClass());
	AddRootRowSpec(UProjectEmoteMenuSpecialRowWidget::StaticClass());
	AddRootRowSpec(UProjectEmoteMenuCancelRowWidget::StaticClass());
}

void UProjectEmoteMenuWidget::SetMenuContent(
	const FText& InTitle,
	const FText& InHint,
	const TArray<FProjectEmoteMenuOption>& InOptions)
{
	SetMenuContent(InTitle, InHint, InOptions, EProjectEmoteMenuVisualMode::Root);
}

void UProjectEmoteMenuWidget::SetMenuContent(
	const FText& InTitle,
	const FText& InHint,
	const TArray<FProjectEmoteMenuOption>& InOptions,
	const EProjectEmoteMenuVisualMode InVisualMode)
{
	VisualMode = InVisualMode;
	BuildWidgetTree();

	MenuOptions = InOptions;
	if (TitleText)
	{
		TitleText->SetText(ProjectEmoteMenuWidgetPrivate::ToUpperText(InTitle));
		if (bUsingNativeFallbackTree)
		{
			TitleText->SetFont(ProjectEmoteMenuWidgetPrivate::MakeTitleFont(VisualMode == EProjectEmoteMenuVisualMode::Root ? 30 : 34));
		}
	}

	if (HintText)
	{
		HintText->SetText(InHint);
		if (bUsingNativeFallbackTree)
		{
			HintText->SetFont(ProjectEmoteMenuWidgetPrivate::MakeBodyFont(18));
		}
	}

	if (PanelSizeBox && bUsingNativeFallbackTree)
	{
		const FVector2D PanelSize = ResolvePanelSize();
		PanelSizeBox->SetWidthOverride(PanelSize.X);
		PanelSizeBox->SetHeightOverride(PanelSize.Y);
	}

	if (PanelBorder)
	{
		if (bUsingNativeFallbackTree)
		{
			PanelBorder->SetPadding(FMargin(3.0f));
		}
	}

	if (PanelInnerBorder)
	{
		if (bUsingNativeFallbackTree)
		{
			PanelInnerBorder->SetPadding(ResolvePanelPadding());
		}
	}

	if (OptionsScrollBox)
	{
		OptionsScrollBox->SetScrollBarVisibility(ShouldShowScrollBar() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (bUsingNativeFallbackTree)
		{
			OptionsScrollBox->SetScrollbarThickness(FVector2D(5.0f, 16.0f));
			OptionsScrollBox->SetScrollbarPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
		}
	}

	RebuildOptionWidgets();
	SelectedIndex = FindFirstEnabledOptionIndex();
	RefreshVisualState();
	ScrollSelectedOptionIntoView();
}

void UProjectEmoteMenuWidget::SetAlternateCancelKey(const FKey InAlternateCancelKey)
{
	AlternateCancelKey = InAlternateCancelKey;
}

void UProjectEmoteMenuWidget::FocusInitialOption()
{
	if (SelectedIndex == INDEX_NONE)
	{
		SelectedIndex = FindFirstEnabledOptionIndex();
	}

	RefreshVisualState();
	FocusMenuWidget();
	ScrollSelectedOptionIntoView();
}

int32 UProjectEmoteMenuWidget::GetSelectedIndex() const
{
	return SelectedIndex;
}

void UProjectEmoteMenuWidget::SelectOptionByIndex(const int32 InIndex)
{
	if (MenuOptions.IsEmpty())
	{
		SelectedIndex = INDEX_NONE;
		RefreshVisualState();
		return;
	}

	int32 NewIndex = FMath::Clamp(InIndex, 0, MenuOptions.Num() - 1);
	if (!MenuOptions[NewIndex].bEnabled)
	{
		const int32 ForwardIndex = FindNextEnabledOptionIndex(NewIndex, 1);
		const int32 BackwardIndex = FindNextEnabledOptionIndex(NewIndex, -1);
		NewIndex = ForwardIndex != INDEX_NONE ? ForwardIndex : BackwardIndex;
	}

	if (NewIndex == INDEX_NONE)
	{
		NewIndex = FMath::Clamp(InIndex, 0, MenuOptions.Num() - 1);
	}

	SelectedIndex = NewIndex;
	RefreshVisualState();
	ScrollSelectedOptionIntoView();
}

void UProjectEmoteMenuWidget::NavigateSelectionByDirection(const int32 Direction)
{
	if (Direction == 0 || MenuOptions.IsEmpty())
	{
		return;
	}

	const int32 NewIndex = FindNextEnabledOptionIndex(SelectedIndex, Direction);
	if (NewIndex != INDEX_NONE)
	{
		SelectOptionByIndex(NewIndex);
	}
}

void UProjectEmoteMenuWidget::ConfirmCurrentSelection()
{
	ActivateOptionByIndex(SelectedIndex);
}

void UProjectEmoteMenuWidget::ActivateOptionByIndex(const int32 InIndex)
{
	if (!MenuOptions.IsValidIndex(InIndex) || !MenuOptions[InIndex].bEnabled || MenuOptions[InIndex].OptionId.IsNone())
	{
		return;
	}

	OnOptionConfirmed.Broadcast(MenuOptions[InIndex].OptionId);
}

void UProjectEmoteMenuWidget::RequestCancel()
{
	OnCancelRequested.Broadcast();
}

void UProjectEmoteMenuWidget::RequestBack()
{
	OnBackRequested.Broadcast();
}

void UProjectEmoteMenuWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (RootCanvas || WidgetTree->RootWidget)
	{
		bUsingNativeFallbackTree = false;
		return;
	}

	bUsingNativeFallbackTree = true;
	BuildDefaultMenuTree(WidgetTree);
}

bool UProjectEmoteMenuWidget::BuildDefaultMenuTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree || TargetWidgetTree->RootWidget)
	{
		return false;
	}

	OptionWidgets.Reset();
	OptionRowWidgets.Reset();

	RootCanvas = TargetWidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	TargetWidgetTree->RootWidget = RootCanvas;

	BackdropBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackdropBorder"));
	BackdropBorder->SetBrushColor(ProjectEmoteMenuWidgetPrivate::BackdropColor);
	if (UCanvasPanelSlot* BackdropSlot = RootCanvas->AddChildToCanvas(BackdropBorder))
	{
		BackdropSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackdropSlot->SetOffsets(FMargin(0.0f));
	}

	PanelSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("PanelSizeBox"));
	const FVector2D PanelSize = ResolvePanelSize();
	PanelSizeBox->SetWidthOverride(PanelSize.X);
	PanelSizeBox->SetHeightOverride(PanelSize.Y);
	if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelSizeBox))
	{
		PanelSlot->SetAutoSize(true);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetPosition(FVector2D::ZeroVector);
	}

	UOverlay* PanelOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("MenuPanelOverlay"));
	PanelSizeBox->SetContent(PanelOverlay);

	if (UTexture2D* HazeTexture = ProjectEmoteMenuWidgetPrivate::LoadTextureAsset(ProjectEmoteMenuWidgetPrivate::HazeTexturePath))
	{
		UImage* HazeImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MenuPanelHaze"));
		HazeImage->SetBrushFromTexture(HazeTexture, true);
		HazeImage->SetColorAndOpacity(ProjectEmoteMenuWidgetPrivate::PanelHazeColor);
		HazeImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UOverlaySlot* HazeSlot = PanelOverlay->AddChildToOverlay(HazeImage))
		{
			HazeSlot->SetHorizontalAlignment(HAlign_Fill);
			HazeSlot->SetVerticalAlignment(VAlign_Fill);
			HazeSlot->SetPadding(FMargin(8.0f));
		}
	}

	if (UTexture2D* FrameTexture = ProjectEmoteMenuWidgetPrivate::LoadTextureAsset(ProjectEmoteMenuWidgetPrivate::FrameTexturePath))
	{
		UImage* PanelGlowImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MenuPanelFrameGlow"));
		PanelGlowImage->SetBrushFromTexture(FrameTexture, true);
		PanelGlowImage->SetColorAndOpacity(ProjectEmoteMenuWidgetPrivate::PanelGlowColor);
		PanelGlowImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UOverlaySlot* GlowSlot = PanelOverlay->AddChildToOverlay(PanelGlowImage))
		{
			GlowSlot->SetHorizontalAlignment(HAlign_Fill);
			GlowSlot->SetVerticalAlignment(VAlign_Fill);
			GlowSlot->SetPadding(FMargin(0.0f));
		}
	}

	if (UTexture2D* FrameTexture = ProjectEmoteMenuWidgetPrivate::LoadTextureAsset(ProjectEmoteMenuWidgetPrivate::FrameTexturePath))
	{
		UImage* PanelFrameImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MenuPanelFrame"));
		PanelFrameImage->SetBrushFromTexture(FrameTexture, true);
		PanelFrameImage->SetColorAndOpacity(ProjectEmoteMenuWidgetPrivate::PanelFrameColor);
		PanelFrameImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UOverlaySlot* FrameSlot = PanelOverlay->AddChildToOverlay(PanelFrameImage))
		{
			FrameSlot->SetHorizontalAlignment(HAlign_Fill);
			FrameSlot->SetVerticalAlignment(VAlign_Fill);
			FrameSlot->SetPadding(FMargin(0.0f));
		}
	}

	PanelBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MenuPanelBorder"));
	PanelBorder->SetBrushColor(ProjectEmoteMenuWidgetPrivate::PanelOuterColor);
	PanelBorder->SetPadding(FMargin(3.0f));
	if (UOverlaySlot* PanelBorderSlot = PanelOverlay->AddChildToOverlay(PanelBorder))
	{
		PanelBorderSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelBorderSlot->SetVerticalAlignment(VAlign_Fill);
		PanelBorderSlot->SetPadding(FMargin(12.0f));
	}

	PanelInnerBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PanelInnerBorder"));
	PanelInnerBorder->SetBrushColor(ProjectEmoteMenuWidgetPrivate::PanelInnerColor);
	PanelInnerBorder->SetPadding(ResolvePanelPadding());
	PanelBorder->SetContent(PanelInnerBorder);

	PanelLayout = TargetWidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuPanelLayout"));
	PanelInnerBorder->SetContent(PanelLayout);

	UHorizontalBox* HeaderRow = TargetWidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MenuHeaderRow"));
	if (UVerticalBoxSlot* HeaderSlot = PanelLayout->AddChildToVerticalBox(HeaderRow))
	{
		HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 24.0f));
	}

	USizeBox* HeaderIconSize = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MenuHeaderIconSize"));
	HeaderIconSize->SetWidthOverride(96.0f);
	HeaderIconSize->SetHeightOverride(96.0f);
	if (UHorizontalBoxSlot* HeaderIconSlot = HeaderRow->AddChildToHorizontalBox(HeaderIconSize))
	{
		HeaderIconSlot->SetPadding(FMargin(0.0f, 0.0f, 26.0f, 0.0f));
		HeaderIconSlot->SetVerticalAlignment(VAlign_Center);
	}

	UBorder* HeaderIconBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MenuHeaderIconBorder"));
	HeaderIconBorder->SetBrushColor(ProjectEmoteMenuWidgetPrivate::IconWellColor);
	HeaderIconBorder->SetPadding(FMargin(8.0f));
	HeaderIconSize->SetContent(HeaderIconBorder);

	UImage* HeaderGlyph = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MenuHeaderGlyph"));
	if (UTexture2D* GlyphTexture = ProjectEmoteMenuWidgetPrivate::LoadTextureAsset(ProjectEmoteMenuWidgetPrivate::GlyphTexturePath))
	{
		HeaderGlyph->SetBrushFromTexture(GlyphTexture, true);
	}
	HeaderGlyph->SetColorAndOpacity(ProjectEmoteMenuWidgetPrivate::TitleColor);
	HeaderIconBorder->SetContent(HeaderGlyph);

	UVerticalBox* HeaderTextColumn = TargetWidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuHeaderTextColumn"));
	if (UHorizontalBoxSlot* HeaderTextSlot = HeaderRow->AddChildToHorizontalBox(HeaderTextColumn))
	{
		HeaderTextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		HeaderTextSlot->SetVerticalAlignment(VAlign_Center);
	}

	TitleText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(LOCTEXT("MenuTitleLabel", "INTERACTIONS"));
	TitleText->SetColorAndOpacity(FSlateColor(ProjectEmoteMenuWidgetPrivate::TitleColor));
	TitleText->SetFont(ProjectEmoteMenuWidgetPrivate::MakeTitleFont(30));
	TitleText->SetShadowOffset(FVector2D(0.0f, 0.8f));
	TitleText->SetShadowColorAndOpacity(ProjectEmoteMenuWidgetPrivate::ShadowColor);
	if (UVerticalBoxSlot* TitleSlot = HeaderTextColumn->AddChildToVerticalBox(TitleText))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 5.0f));
	}

	HintText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HintText"));
	HintText->SetText(LOCTEXT("MenuHintLabel", "Use Up/Down, Enter, Y or Esc"));
	HintText->SetAutoWrapText(true);
	HintText->SetColorAndOpacity(FSlateColor(ProjectEmoteMenuWidgetPrivate::HintColor));
	HintText->SetFont(ProjectEmoteMenuWidgetPrivate::MakeBodyFont(18));
	HintText->SetShadowOffset(FVector2D(0.0f, 0.35f));
	HintText->SetShadowColorAndOpacity(ProjectEmoteMenuWidgetPrivate::ShadowColor.CopyWithNewOpacity(0.32f));
	HeaderTextColumn->AddChildToVerticalBox(HintText);

	USizeBox* DividerSize = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MenuDividerSize"));
	DividerSize->SetHeightOverride(10.0f);
	if (UVerticalBoxSlot* DividerSlot = PanelLayout->AddChildToVerticalBox(DividerSize))
	{
		DividerSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 22.0f));
	}

	UImage* DividerImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MenuDivider"));
	if (UTexture2D* DividerTexture = ProjectEmoteMenuWidgetPrivate::LoadTextureAsset(ProjectEmoteMenuWidgetPrivate::DividerTexturePath))
	{
		DividerImage->SetBrushFromTexture(DividerTexture, true);
	}
	DividerImage->SetColorAndOpacity(ProjectEmoteMenuWidgetPrivate::DividerColor);
	DividerSize->SetContent(DividerImage);

	OptionsLayout = TargetWidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OptionsLayout"));
	if (UVerticalBoxSlot* RootOptionsSlot = PanelLayout->AddChildToVerticalBox(OptionsLayout))
	{
		RootOptionsSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RootOptionsSlot->SetPadding(FMargin(0.0f));
	}

	const TArray<FProjectEmoteMenuOption> PreviewOptions = MenuOptions.IsEmpty()
		? ProjectEmoteMenuWidgetPrivate::MakeRootPreviewOptions()
		: MenuOptions;
	for (int32 PreviewIndex = 0; PreviewIndex < PreviewOptions.Num(); ++PreviewIndex)
	{
		const FProjectEmoteMenuOption& PreviewOption = PreviewOptions[PreviewIndex];
		if (PreviewOption.OptionId == TEXT("Root.Actions"))
		{
			ActionsRow = ConstructFixedRootRow(TargetWidgetTree, OptionsLayout, PreviewOption, TEXT("ActionsRow"), PreviewIndex);
		}
		else if (PreviewOption.OptionId == TEXT("Root.Objects"))
		{
			ObjectsRow = ConstructFixedRootRow(TargetWidgetTree, OptionsLayout, PreviewOption, TEXT("ObjectsRow"), PreviewIndex);
		}
		else if (PreviewOption.OptionId == TEXT("Root.Social"))
		{
			SocialRow = ConstructFixedRootRow(TargetWidgetTree, OptionsLayout, PreviewOption, TEXT("SocialRow"), PreviewIndex);
		}
		else if (PreviewOption.OptionId == TEXT("Root.Special"))
		{
			SpecialRow = ConstructFixedRootRow(TargetWidgetTree, OptionsLayout, PreviewOption, TEXT("SpecialRow"), PreviewIndex);
		}
		else if (PreviewOption.OptionId == TEXT("Root.Cancel"))
		{
			CancelRow = ConstructFixedRootRow(TargetWidgetTree, OptionsLayout, PreviewOption, TEXT("CancelRow"), PreviewIndex);
		}
	}

	OptionsScrollBox = TargetWidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("OptionsScrollBox"));
	OptionsScrollBox->SetOrientation(Orient_Vertical);
	OptionsScrollBox->SetVisibility(ESlateVisibility::Collapsed);
	OptionsScrollBox->SetScrollBarVisibility(ESlateVisibility::Collapsed);
	OptionsScrollBox->SetAlwaysShowScrollbar(false);
	OptionsScrollBox->SetAnimateWheelScrolling(true);
	OptionsScrollBox->SetNavigationDestination(EDescendantScrollDestination::IntoView);
	if (UVerticalBoxSlot* OptionsSlot = PanelLayout->AddChildToVerticalBox(OptionsScrollBox))
	{
		OptionsSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		OptionsSlot->SetPadding(FMargin(0.0f));
	}

	AddControlFooter(TargetWidgetTree);

	return true;
}

void UProjectEmoteMenuWidget::RebuildOptionWidgets()
{
	if (CanUseFixedRootOptionWidgets())
	{
		RebuildFixedRootOptionWidgets();
		return;
	}

	RebuildDynamicOptionWidgets();
}

void UProjectEmoteMenuWidget::RebuildFixedRootOptionWidgets()
{
	OptionWidgets.Reset();
	OptionRowWidgets.Reset();

	ResetFixedRootOptionVisibility();

	if (OptionsLayout)
	{
		OptionsLayout->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (OptionsScrollBox)
	{
		OptionsScrollBox->ClearChildren();
		OptionsScrollBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	for (int32 OptionIndex = 0; OptionIndex < MenuOptions.Num(); ++OptionIndex)
	{
		const FProjectEmoteMenuOption& Option = MenuOptions[OptionIndex];
		UProjectEmoteMenuOptionWidget* RowWidget = ResolveFixedRootRowForOption(Option);
		if (!RowWidget)
		{
			continue;
		}

		RowWidget->SetUseDesignerIconOverride(true);
		RowWidget->ConfigureOption(
			Option,
			VisualMode,
			OptionIndex,
			ResolveOptionHeight(),
			ResolveIconTexture(Option),
			ResolveAccentColor(Option),
			true);
		RowWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		OptionWidgets.Add(RowWidget);
		OptionRowWidgets.Add(RowWidget);
	}

	RefreshVisualState();
}

void UProjectEmoteMenuWidget::RebuildDynamicOptionWidgets()
{
	if (!OptionsScrollBox)
	{
		return;
	}

	if (OptionsLayout)
	{
		OptionsLayout->SetVisibility(ESlateVisibility::Collapsed);
	}

	OptionsScrollBox->ClearChildren();
	OptionsScrollBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	OptionWidgets.Reset();
	OptionRowWidgets.Reset();

	TSubclassOf<UProjectEmoteMenuOptionWidget> RowWidgetClass = ResolveOptionRowWidgetClass();
	if (!RowWidgetClass)
	{
		RowWidgetClass = UProjectEmoteMenuOptionWidget::StaticClass();
	}

	for (int32 OptionIndex = 0; OptionIndex < MenuOptions.Num(); ++OptionIndex)
	{
		const FProjectEmoteMenuOption& Option = MenuOptions[OptionIndex];
		UProjectEmoteMenuOptionWidget* RowWidget = nullptr;
		// Let UMG assign unique UObject names; converted menus can swap row WBP classes before old rows are GC'd.
		if (APlayerController* OwningPlayer = GetOwningPlayer())
		{
			RowWidget = CreateWidget<UProjectEmoteMenuOptionWidget>(OwningPlayer, RowWidgetClass);
		}
		else if (UWorld* World = GetWorld())
		{
			RowWidget = CreateWidget<UProjectEmoteMenuOptionWidget>(World, RowWidgetClass);
		}

		if (!RowWidget)
		{
			continue;
		}

		RowWidget->ConfigureOption(
			Option,
			VisualMode,
			OptionIndex,
			ResolveOptionHeight(),
			ResolveIconTexture(Option),
			ResolveAccentColor(Option));

		if (UScrollBoxSlot* OptionSlot = Cast<UScrollBoxSlot>(OptionsScrollBox->AddChild(RowWidget)))
		{
			OptionSlot->SetPadding(FMargin(0.0f, OptionIndex == 0 ? 0.0f : 12.0f, 0.0f, 0.0f));
		}

		OptionWidgets.Add(RowWidget);
		OptionRowWidgets.Add(RowWidget);
	}

	RefreshVisualState();
	ScrollSelectedOptionIntoView();
}

bool UProjectEmoteMenuWidget::CanUseFixedRootOptionWidgets() const
{
	if (VisualMode != EProjectEmoteMenuVisualMode::Root || MenuOptions.IsEmpty() || !OptionsLayout)
	{
		return false;
	}

	for (const FProjectEmoteMenuOption& Option : MenuOptions)
	{
		if (!ResolveFixedRootRowForOption(Option))
		{
			return false;
		}
	}

	return true;
}

void UProjectEmoteMenuWidget::ResetFixedRootOptionVisibility()
{
	if (ActionsRow)
	{
		ActionsRow->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ObjectsRow)
	{
		ObjectsRow->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (SocialRow)
	{
		SocialRow->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (SpecialRow)
	{
		SpecialRow->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (CancelRow)
	{
		CancelRow->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UProjectEmoteMenuOptionWidget* UProjectEmoteMenuWidget::ResolveFixedRootRowForOption(const FProjectEmoteMenuOption& Option) const
{
	if (Option.OptionId == TEXT("Root.Actions"))
	{
		return ActionsRow;
	}
	if (Option.OptionId == TEXT("Root.Objects"))
	{
		return ObjectsRow;
	}
	if (Option.OptionId == TEXT("Root.Social"))
	{
		return SocialRow;
	}
	if (Option.OptionId == TEXT("Root.Special"))
	{
		return SpecialRow;
	}
	if (Option.OptionId == TEXT("Root.Cancel"))
	{
		return CancelRow;
	}

	return nullptr;
}

UProjectEmoteMenuOptionWidget* UProjectEmoteMenuWidget::ConstructFixedRootRow(
	UWidgetTree* TargetWidgetTree,
	UVerticalBox* Parent,
	const FProjectEmoteMenuOption& PreviewOption,
	const FName WidgetName,
	const int32 PreviewIndex)
{
	if (!TargetWidgetTree || !Parent)
	{
		return nullptr;
	}

	TSubclassOf<UProjectEmoteMenuOptionWidget> RowWidgetClass = ResolveFixedRootRowWidgetClass(PreviewOption);
	if (!RowWidgetClass)
	{
		RowWidgetClass = UProjectEmoteMenuOptionWidget::StaticClass();
	}

	UProjectEmoteMenuOptionWidget* RowWidget = TargetWidgetTree->ConstructWidget<UProjectEmoteMenuOptionWidget>(RowWidgetClass, WidgetName);
	if (!RowWidget)
	{
		return nullptr;
	}

	UTexture2D* PreviewIcon = ResolveIconTexture(PreviewOption);
	RowWidget->SetDesignerPreviewOption(PreviewOption, EProjectEmoteMenuVisualMode::Root, PreviewIndex);
	RowWidget->SetDesignerIconOverride(PreviewIcon);
	RowWidget->ConfigureOption(
		PreviewOption,
		EProjectEmoteMenuVisualMode::Root,
		PreviewIndex,
		ResolveOptionHeight(),
		PreviewIcon,
		ResolveAccentColor(PreviewOption),
		true);

	if (UVerticalBoxSlot* RowSlot = Parent->AddChildToVerticalBox(RowWidget))
	{
		RowSlot->SetPadding(FMargin(0.0f, PreviewIndex == 0 ? 0.0f : ProjectEmoteMenuWidgetPrivate::OptionRowGap, 0.0f, 0.0f));
	}

	return RowWidget;
}

void UProjectEmoteMenuWidget::RefreshVisualState()
{
	for (int32 OptionIndex = 0; OptionIndex < MenuOptions.Num(); ++OptionIndex)
	{
		const bool bSelected = OptionIndex == SelectedIndex;
		const bool bEnabled = MenuOptions.IsValidIndex(OptionIndex) ? MenuOptions[OptionIndex].bEnabled : false;
		if (UProjectEmoteMenuOptionWidget* OptionWidget = OptionWidgets.IsValidIndex(OptionIndex) ? OptionWidgets[OptionIndex] : nullptr)
		{
			OptionWidget->SetOptionVisualState(bSelected, bEnabled);
		}
	}
}

void UProjectEmoteMenuWidget::ScrollSelectedOptionIntoView()
{
	if (CanUseFixedRootOptionWidgets())
	{
		return;
	}

	if (!OptionsScrollBox || !OptionRowWidgets.IsValidIndex(SelectedIndex))
	{
		return;
	}

	if (UWidget* SelectedRow = OptionRowWidgets[SelectedIndex])
	{
		OptionsScrollBox->EndInertialScrolling();
		OptionsScrollBox->ScrollWidgetIntoView(SelectedRow, true, EDescendantScrollDestination::IntoView, 18.0f);
	}
}

TSubclassOf<UProjectEmoteMenuOptionWidget> UProjectEmoteMenuWidget::ResolveOptionRowWidgetClass() const
{
	if (OptionRowWidgetClass)
	{
		return OptionRowWidgetClass;
	}

	return ProjectWidgetClassResolver::DiscoverWidgetClass<UProjectEmoteMenuOptionWidget>(TEXT("ProjectEmoteMenuOptionRow"));
}

TSubclassOf<UProjectEmoteMenuOptionWidget> UProjectEmoteMenuWidget::ResolveFixedRootRowWidgetClass(const FProjectEmoteMenuOption& Option) const
{
	TSubclassOf<UProjectEmoteMenuOptionWidget> ConfiguredClass;
	UClass* NativeRowClass = nullptr;

	if (Option.OptionId == TEXT("Root.Actions"))
	{
		ConfiguredClass = ActionsRowWidgetClass;
		NativeRowClass = UProjectEmoteMenuActionsRowWidget::StaticClass();
	}
	else if (Option.OptionId == TEXT("Root.Objects"))
	{
		ConfiguredClass = ObjectsRowWidgetClass;
		NativeRowClass = UProjectEmoteMenuObjectsRowWidget::StaticClass();
	}
	else if (Option.OptionId == TEXT("Root.Social"))
	{
		ConfiguredClass = SocialRowWidgetClass;
		NativeRowClass = UProjectEmoteMenuSocialRowWidget::StaticClass();
	}
	else if (Option.OptionId == TEXT("Root.Special"))
	{
		ConfiguredClass = SpecialRowWidgetClass;
		NativeRowClass = UProjectEmoteMenuSpecialRowWidget::StaticClass();
	}
	else if (Option.OptionId == TEXT("Root.Cancel"))
	{
		ConfiguredClass = CancelRowWidgetClass;
		NativeRowClass = UProjectEmoteMenuCancelRowWidget::StaticClass();
	}

	if (ConfiguredClass)
	{
		return ConfiguredClass;
	}

	if (NativeRowClass)
	{
		if (UClass* DiscoveredClass = ProjectWidgetClassResolver::ResolveWidgetClass(
			FSoftClassPath(),
			NativeRowClass,
			TEXT("ProjectEmoteMenuFixedRootRow")))
		{
			return DiscoveredClass;
		}
	}

	return ResolveOptionRowWidgetClass();
}

void UProjectEmoteMenuWidget::FocusMenuWidget()
{
	if (IsRunningCommandlet())
	{
		return;
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		SetUserFocus(PlayerController);
	}

	SetKeyboardFocus();

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetKeyboardFocus(TakeWidget(), EFocusCause::SetDirectly);
	}
}

bool UProjectEmoteMenuWidget::HandleKeyNavigation(const FKey& Key)
{
	if (Key == EKeys::Up)
	{
		NavigateSelectionByDirection(-1);
		return true;
	}

	if (Key == EKeys::Down)
	{
		NavigateSelectionByDirection(1);
		return true;
	}

	if (Key == EKeys::Enter)
	{
		ConfirmCurrentSelection();
		return true;
	}

	if (Key == EKeys::Q)
	{
		RequestBack();
		return true;
	}

	if (Key == EKeys::Y || Key == EKeys::Escape || (AlternateCancelKey.IsValid() && Key == AlternateCancelKey))
	{
		RequestCancel();
		return true;
	}

	return false;
}

int32 UProjectEmoteMenuWidget::FindFirstEnabledOptionIndex() const
{
	for (int32 OptionIndex = 0; OptionIndex < MenuOptions.Num(); ++OptionIndex)
	{
		if (MenuOptions[OptionIndex].bEnabled)
		{
			return OptionIndex;
		}
	}

	return MenuOptions.IsEmpty() ? INDEX_NONE : 0;
}

int32 UProjectEmoteMenuWidget::FindNextEnabledOptionIndex(const int32 StartIndex, const int32 Direction) const
{
	if (MenuOptions.IsEmpty())
	{
		return INDEX_NONE;
	}

	const int32 Step = Direction >= 0 ? 1 : -1;
	int32 CandidateIndex = StartIndex;
	if (CandidateIndex == INDEX_NONE)
	{
		CandidateIndex = Step > 0 ? -1 : 0;
	}

	for (int32 Attempt = 0; Attempt < MenuOptions.Num(); ++Attempt)
	{
		CandidateIndex += Step;
		if (CandidateIndex < 0)
		{
			CandidateIndex = MenuOptions.Num() - 1;
		}
		else if (CandidateIndex >= MenuOptions.Num())
		{
			CandidateIndex = 0;
		}

		if (MenuOptions[CandidateIndex].bEnabled)
		{
			return CandidateIndex;
		}
	}

	return INDEX_NONE;
}

FVector2D UProjectEmoteMenuWidget::ResolvePanelSize() const
{
	FVector2D PanelSize(ProjectEmoteMenuWidgetPrivate::InteractionPanelWidth, ProjectEmoteMenuWidgetPrivate::InteractionPanelHeight);
	switch (VisualMode)
	{
	case EProjectEmoteMenuVisualMode::Root:
		PanelSize = FVector2D(ProjectEmoteMenuWidgetPrivate::InteractionPanelWidth, ProjectEmoteMenuWidgetPrivate::InteractionPanelHeight);
		break;
	case EProjectEmoteMenuVisualMode::Category:
		PanelSize = FVector2D(ProjectEmoteMenuWidgetPrivate::CategoryPanelWidth, ProjectEmoteMenuWidgetPrivate::CategoryPanelHeight);
		break;
	case EProjectEmoteMenuVisualMode::AnimationList:
		PanelSize = FVector2D(ProjectEmoteMenuWidgetPrivate::AnimationListPanelWidth, ProjectEmoteMenuWidgetPrivate::AnimationListPanelHeight);
		break;
	default:
		break;
	}

	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize = FVector2D::ZeroVector;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		if (ViewportSize.X > 0.0f && ViewportSize.Y > 0.0f)
		{
			const float HeightLimit = VisualMode == EProjectEmoteMenuVisualMode::Root
				? ProjectEmoteMenuWidgetPrivate::InteractionViewportHeightLimit
				: ProjectEmoteMenuWidgetPrivate::ViewportHeightLimit;
			PanelSize.X = FMath::Min(PanelSize.X, ViewportSize.X * ProjectEmoteMenuWidgetPrivate::ViewportWidthLimit);
			PanelSize.Y = FMath::Min(PanelSize.Y, ViewportSize.Y * HeightLimit);
		}
	}

	if (VisualMode == EProjectEmoteMenuVisualMode::Root && !MenuOptions.IsEmpty())
	{
		const float RequiredListHeight = (MenuOptions.Num() * ResolveOptionHeight())
			+ (FMath::Max(0, MenuOptions.Num() - 1) * ProjectEmoteMenuWidgetPrivate::OptionRowGap);
		const float RequiredPanelHeight = ProjectEmoteMenuWidgetPrivate::PanelChromeHeight + RequiredListHeight;
		PanelSize.Y = FMath::Max(PanelSize.Y, RequiredPanelHeight);
	}

	return PanelSize;
}

FMargin UProjectEmoteMenuWidget::ResolvePanelPadding() const
{
	switch (VisualMode)
	{
	case EProjectEmoteMenuVisualMode::Category:
	case EProjectEmoteMenuVisualMode::Root:
		return FMargin(34.0f, 30.0f, 34.0f, 24.0f);
	case EProjectEmoteMenuVisualMode::AnimationList:
		return FMargin(28.0f, 28.0f, 28.0f, 20.0f);
	default:
		return FMargin(34.0f, 30.0f, 34.0f, 24.0f);
	}
}

float UProjectEmoteMenuWidget::ResolveOptionHeight() const
{
	switch (VisualMode)
	{
	case EProjectEmoteMenuVisualMode::Category:
	case EProjectEmoteMenuVisualMode::Root:
		return 82.0f;
	case EProjectEmoteMenuVisualMode::AnimationList:
		return 74.0f;
	default:
		return 82.0f;
	}
}

FLinearColor UProjectEmoteMenuWidget::ResolveAccentColor(const FProjectEmoteMenuOption& Option) const
{
	const FName Attribute = Option.VisualAttribute;
	if (Attribute == TEXT("Willpower"))
	{
		return FLinearColor(0.95f, 0.76f, 0.33f, 1.0f);
	}
	if (Attribute == TEXT("Sadism"))
	{
		return FLinearColor(1.0f, 0.16f, 0.38f, 1.0f);
	}
	if (Attribute == TEXT("Masochism"))
	{
		return FLinearColor(0.62f, 0.22f, 1.0f, 1.0f);
	}
	if (Attribute == TEXT("Faith"))
	{
		return FLinearColor(0.36f, 0.62f, 1.0f, 1.0f);
	}
	if (Attribute == TEXT("Cunning"))
	{
		return FLinearColor(0.24f, 0.72f, 0.42f, 1.0f);
	}
	if (Attribute == TEXT("Celerity"))
	{
		return FLinearColor(0.20f, 0.86f, 1.0f, 1.0f);
	}
	if (Attribute == TEXT("Allure"))
	{
		return FLinearColor(1.0f, 0.32f, 0.72f, 1.0f);
	}

	if (Option.VisualIconId == TEXT("Combat"))
	{
		return FLinearColor(1.0f, 0.20f, 0.44f, 1.0f);
	}
	if (Option.VisualIconId == TEXT("Objects"))
	{
		return FLinearColor(0.68f, 0.30f, 1.0f, 1.0f);
	}
	if (Option.VisualIconId == TEXT("Social") || Option.VisualIconId == TEXT("Partner"))
	{
		return FLinearColor(0.95f, 0.42f, 0.78f, 1.0f);
	}

	return FLinearColor(0.76f, 0.34f, 0.98f, 1.0f);
}

UTexture2D* UProjectEmoteMenuWidget::ResolveIconTexture(const FProjectEmoteMenuOption& Option) const
{
	if (!Option.MenuIconTexture.IsNull())
	{
		if (UTexture2D* LoadedTexture = Option.MenuIconTexture.LoadSynchronous())
		{
			return LoadedTexture;
		}
	}

	const FName Attribute = Option.VisualAttribute;
	if (Attribute == TEXT("Willpower"))
	{
		return LoadTextureByPath(TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Willpower.T_SinfulAscension_Altar_Icon_Willpower"));
	}
	if (Attribute == TEXT("Sadism"))
	{
		return LoadTextureByPath(TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Sadism.T_SinfulAscension_Altar_Icon_Sadism"));
	}
	if (Attribute == TEXT("Masochism"))
	{
		return LoadTextureByPath(TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Masochism.T_SinfulAscension_Altar_Icon_Masochism"));
	}
	if (Attribute == TEXT("Faith"))
	{
		return LoadTextureByPath(TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Faith.T_SinfulAscension_Altar_Icon_Faith"));
	}
	if (Attribute == TEXT("Cunning"))
	{
		return LoadTextureByPath(TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Cunning.T_SinfulAscension_Altar_Icon_Cunning"));
	}
	if (Attribute == TEXT("Celerity"))
	{
		return LoadTextureByPath(TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Celerity.T_SinfulAscension_Altar_Icon_Celerity"));
	}
	if (Attribute == TEXT("Allure"))
	{
		return LoadTextureByPath(TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Allure.T_SinfulAscension_Altar_Icon_Allure"));
	}

	if (Option.VisualIconId == TEXT("Actions"))
	{
		if (UTexture2D* IconTexture = LoadTextureByPath(TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Actions.T_LADS_Icon_Actions")))
		{
			return IconTexture;
		}
	}
	if (Option.VisualIconId == TEXT("Basic"))
	{
		if (UTexture2D* IconTexture = LoadTextureByPath(TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Basic.T_LADS_Icon_Basic")))
		{
			return IconTexture;
		}
	}
	if (Option.VisualIconId == TEXT("Emotes"))
	{
		if (UTexture2D* IconTexture = LoadTextureByPath(TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Emotes.T_LADS_Icon_Emotes")))
		{
			return IconTexture;
		}
	}
	if (Option.VisualIconId == TEXT("Combat"))
	{
		if (UTexture2D* IconTexture = LoadTextureByPath(TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Combat.T_LADS_Icon_Combat")))
		{
			return IconTexture;
		}
	}
	if (Option.VisualIconId == TEXT("Partner"))
	{
		if (UTexture2D* IconTexture = LoadTextureByPath(TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Partner.T_LADS_Icon_Partner")))
		{
			return IconTexture;
		}
	}
	if (Option.VisualIconId == TEXT("Objects"))
	{
		if (UTexture2D* IconTexture = LoadTextureByPath(TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Objects.T_LADS_Icon_Objects")))
		{
			return IconTexture;
		}
	}
	if (Option.VisualIconId == TEXT("Social"))
	{
		if (UTexture2D* IconTexture = LoadTextureByPath(TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Social.T_LADS_Icon_Social")))
		{
			return IconTexture;
		}
	}
	if (Option.VisualIconId == TEXT("Special"))
	{
		if (UTexture2D* IconTexture = LoadTextureByPath(TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Special.T_LADS_Icon_Special")))
		{
			return IconTexture;
		}
	}
	if (Option.VisualIconId == TEXT("Cancel"))
	{
		if (UTexture2D* IconTexture = LoadTextureByPath(TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Cancel.T_LADS_Icon_Cancel")))
		{
			return IconTexture;
		}
	}
	if (Option.VisualIconId == TEXT("Back"))
	{
		if (UTexture2D* IconTexture = LoadTextureByPath(TEXT("/Game/UI/Emote/Textures/T_LADS_Icon_Back.T_LADS_Icon_Back")))
		{
			return IconTexture;
		}
	}

	return LoadTextureByPath(TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Default.T_SinfulAscension_Altar_Icon_Default"));
}

UTexture2D* UProjectEmoteMenuWidget::LoadTextureByPath(const TCHAR* TexturePath) const
{
	return TexturePath ? LoadObject<UTexture2D>(nullptr, TexturePath) : nullptr;
}

bool UProjectEmoteMenuWidget::ShouldShowScrollBar() const
{
	if (MenuOptions.IsEmpty())
	{
		return false;
	}

	const FVector2D PanelSize = ResolvePanelSize();
	const float RowGap = ProjectEmoteMenuWidgetPrivate::OptionRowGap;
	const float RowSpan = ResolveOptionHeight() + RowGap;
	const float ChromeHeight = ProjectEmoteMenuWidgetPrivate::PanelChromeHeight;
	const float EstimatedListHeight = FMath::Max(ResolveOptionHeight(), PanelSize.Y - ChromeHeight);
	const int32 EstimatedVisibleRows = FMath::Max(1, FMath::FloorToInt((EstimatedListHeight + RowGap) / RowSpan));
	return MenuOptions.Num() > EstimatedVisibleRows;
}

void UProjectEmoteMenuWidget::AddControlFooter(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree)
	{
		return;
	}

	UBorder* FooterBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MenuFooterBorder"));
	FooterBorder->SetBrushColor(ProjectEmoteMenuWidgetPrivate::FooterColor);
	FooterBorder->SetPadding(FMargin(14.0f, 8.0f));
	if (UVerticalBoxSlot* FooterSlot = PanelLayout->AddChildToVerticalBox(FooterBorder))
	{
		FooterSlot->SetPadding(FMargin(0.0f, 24.0f, 0.0f, 0.0f));
	}

	UOverlay* FooterOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("MenuFooterOverlay"));
	FooterBorder->SetContent(FooterOverlay);

	if (UTexture2D* FooterTexture = ProjectEmoteMenuWidgetPrivate::LoadTextureAsset(ProjectEmoteMenuWidgetPrivate::FooterTexturePath))
	{
		UImage* FooterImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MenuFooterOrnament"));
		FooterImage->SetBrushFromTexture(FooterTexture, true);
		FooterImage->SetColorAndOpacity(ProjectEmoteMenuWidgetPrivate::DividerColor.CopyWithNewOpacity(0.14f));
		FooterImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UOverlaySlot* FooterImageSlot = FooterOverlay->AddChildToOverlay(FooterImage))
		{
			FooterImageSlot->SetHorizontalAlignment(HAlign_Fill);
			FooterImageSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	UTextBlock* FooterText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MenuFooterText"));
	FooterText->SetText(LOCTEXT("MenuKeyboardFooter", "Up/Down Navigate  |  Enter Select  |  Q Back  |  Esc Close"));
	FooterText->SetAutoWrapText(true);
	FooterText->SetColorAndOpacity(FSlateColor(ProjectEmoteMenuWidgetPrivate::HintColor));
	FooterText->SetFont(ProjectEmoteMenuWidgetPrivate::MakeBodyFont(13));
	FooterText->SetShadowOffset(FVector2D(0.0f, 0.25f));
	FooterText->SetShadowColorAndOpacity(ProjectEmoteMenuWidgetPrivate::ShadowColor.CopyWithNewOpacity(0.28f));
	if (UOverlaySlot* FooterTextSlot = FooterOverlay->AddChildToOverlay(FooterText))
	{
		FooterTextSlot->SetHorizontalAlignment(HAlign_Fill);
		FooterTextSlot->SetVerticalAlignment(VAlign_Center);
	}
	FooterText->SetJustification(ETextJustify::Center);
}

#undef LOCTEXT_NAMESPACE
