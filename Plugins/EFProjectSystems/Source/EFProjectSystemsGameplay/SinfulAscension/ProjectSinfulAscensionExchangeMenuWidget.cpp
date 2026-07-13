#include "SinfulAscension/ProjectSinfulAscensionExchangeMenuWidget.h"

#include "EFProjectUISettings.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScaleBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Input/Reply.h"
#include "SinfulAscension/ProjectSinfulAscensionComponent.h"
#include "Styling/CoreStyle.h"
#include "UI/ProjectWidgetClassResolver.h"

#define LOCTEXT_NAMESPACE "ProjectSinfulAscensionExchangeMenuWidget"

namespace ProjectSinfulExchangeMenuWidgetPrivate
{
	constexpr float FullscreenInset = 32.0f;
	constexpr float FrameCornerRadius = 18.0f;
	constexpr float FrameOutlineWidth = 1.2f;
	constexpr float ResourceBoxHeight = 56.0f;
	constexpr float AttributeListWidth = 560.0f;
	constexpr float RowWidth = 560.0f;
	constexpr float RowHeight = 88.0f;
	constexpr int32 RuntimeFallbackSortBase = 1000;

	const FLinearColor BackdropTint(0.0f, 0.0f, 0.0f, 0.74f);
	const FLinearColor FrameFillTint(0.022f, 0.014f, 0.044f, 0.97f);
	const FLinearColor FrameOutlineTint(0.95f, 0.55f, 0.84f, 0.88f);
	const FLinearColor SectionFillTint(0.032f, 0.022f, 0.056f, 0.95f);
	const FLinearColor SectionOutlineTint(0.60f, 0.34f, 0.62f, 0.44f);
	const FLinearColor HazeTint(1.0f, 0.54f, 0.84f, 0.14f);
	const FLinearColor TitleTint(0.97f, 0.62f, 0.84f, 1.0f);
	const FLinearColor PrimaryTextTint(0.98f, 0.97f, 0.99f, 1.0f);
	const FLinearColor SecondaryTextTint(0.84f, 0.78f, 0.90f, 0.96f);
	const FLinearColor AccentTint(0.97f, 0.54f, 0.82f, 1.0f);
	const FLinearColor WarningTint(0.98f, 0.67f, 0.77f, 1.0f);
	const FLinearColor ShadowTint(0.0f, 0.0f, 0.0f, 0.34f);

	const TCHAR* TitleFontPath = TEXT("/Game/_Game/Widgets/Chronicle/Assets/Fonts/F_Chronicle_Bebas.F_Chronicle_Bebas");
	const TCHAR* BodyFontPath = TEXT("/Game/_Game/Widgets/Chronicle/Assets/Fonts/F_Chronicle_Cormorant.F_Chronicle_Cormorant");
	const TCHAR* FrameTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Frame.T_SinfulAscension_Altar_Frame");
	const TCHAR* HazeTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Haze.T_SinfulAscension_Altar_Haze");
	const TCHAR* DividerTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Divider.T_SinfulAscension_Altar_Divider");
	const TCHAR* StatBoxTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_StatBox.T_SinfulAscension_Altar_StatBox");
	const TCHAR* FooterOrnamentTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Footer.T_SinfulAscension_Altar_Footer");
	const TCHAR* ModeGlyphTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Glyph.T_SinfulAscension_Altar_Glyph");
	const TCHAR* DefaultIconTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Default.T_SinfulAscension_Altar_Icon_Default");
	const TCHAR* DefaultWatermarkTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Watermark_Default.T_SinfulAscension_Altar_Watermark_Default");

	UObject* LoadObjectByPath(const TCHAR* AssetPath)
	{
		return AssetPath && AssetPath[0] != 0
			? StaticLoadObject(UObject::StaticClass(), nullptr, AssetPath)
			: nullptr;
	}

	FName BuildAttributeName(const EProjectSinAttribute Attribute)
	{
		if (const UEnum* AttributeEnum = StaticEnum<EProjectSinAttribute>())
		{
			const FString NameString = AttributeEnum->GetNameStringByValue(static_cast<int64>(Attribute));
			if (!NameString.IsEmpty() && NameString != TEXT("Count"))
			{
				return FName(*NameString);
			}
		}

		return NAME_None;
	}

	FString NormalizeMenuLabel(const FText& DisplayName)
	{
		FString Result = DisplayName.ToString();
		Result.TrimStartAndEndInline();
		return Result.IsEmpty() ? TEXT("Unknown") : Result;
	}

	TArray<FName> MenuWidgetNames()
	{
		return {
			TEXT("RootCanvas"),
			TEXT("BackdropBorder"),
			TEXT("FrameOverlay"),
			TEXT("BackgroundBorder"),
			TEXT("HazeImage"),
			TEXT("FrameTextureImage"),
			TEXT("ContentBorder"),
			TEXT("ContentBox"),
			TEXT("HeaderRow"),
			TEXT("TitleText"),
			TEXT("ModeBox"),
			TEXT("ModeGlyphImage"),
			TEXT("ModeText"),
			TEXT("TopDividerImage"),
			TEXT("ResourceRow"),
			TEXT("RunSxpBorder"),
			TEXT("RunSxpLabelText"),
			TEXT("RunSxpValueScaleBox"),
			TEXT("RunSxpValueText"),
			TEXT("MetaSxpBorder"),
			TEXT("MetaSxpLabelText"),
			TEXT("MetaSxpValueScaleBox"),
			TEXT("MetaSxpValueText"),
			TEXT("BodyRow"),
			TEXT("AttributeListBorder"),
			TEXT("RowsGlobalWidget"),
			TEXT("DetailBorder"),
			TEXT("DetailOverlay"),
			TEXT("DetailWatermarkImage"),
			TEXT("DetailContentBorder"),
			TEXT("DetailContentBox"),
			TEXT("DetailNameText"),
			TEXT("DetailLevelText"),
			TEXT("DetailCostText"),
			TEXT("DetailMilestoneDividerImage"),
			TEXT("DetailMilestoneFiveText"),
			TEXT("DetailMilestoneTenText"),
			TEXT("DetailFlavorText"),
			TEXT("FooterDividerImage"),
			TEXT("FooterRow"),
			TEXT("FooterStatusText"),
			TEXT("FooterControlsText")
		};
	}

	TArray<FName> RowWidgetNames()
	{
		return {
			TEXT("RootSizeBox"),
			TEXT("DesignerRootOverlay"),
			TEXT("RootOverlay"),
			TEXT("BackgroundBorder"),
			TEXT("FrameImage"),
			TEXT("ContentBorder"),
			TEXT("ContentBox"),
			TEXT("IconSizeBox"),
			TEXT("IconBackgroundBorder"),
			TEXT("IconImage"),
			TEXT("TextColumn"),
			TEXT("NameText"),
			TEXT("MetaRow"),
			TEXT("LevelText"),
			TEXT("SeparatorText"),
			TEXT("CostText"),
			TEXT("SelectionGlyphSizeBox"),
			TEXT("SelectionGlyphImage")
		};
	}

	TArray<FName> RowsGlobalWidgetNames()
	{
		return {
			TEXT("DesignerRootOverlay"),
			TEXT("RootSizeBox"),
			TEXT("RootOverlay"),
			TEXT("RowsScrollBox"),
			TEXT("RowsLayout"),
			TEXT("WillpowerRow"),
			TEXT("SadismRow"),
			TEXT("MasochismRow"),
			TEXT("FaithRow"),
			TEXT("CunningRow"),
			TEXT("CelerityRow"),
			TEXT("AllureRow"),
			TEXT("ExtraRowsLayout")
		};
	}

	TArray<FName> DetailWidgetNames()
	{
		return {
			TEXT("DetailBorder"),
			TEXT("DetailOverlay"),
			TEXT("DetailWatermarkImage"),
			TEXT("DetailContentBorder"),
			TEXT("DetailContentBox"),
			TEXT("DetailNameText"),
			TEXT("DetailLevelText"),
			TEXT("DetailCostText"),
			TEXT("DetailMilestoneDividerImage"),
			TEXT("DetailMilestoneFiveText"),
			TEXT("DetailMilestoneTenText"),
			TEXT("DetailFlavorText")
		};
	}

	TArray<FString> AttributePreviewTexts()
	{
		return {
			TEXT("Willpower"),
			TEXT("Sadism"),
			TEXT("Masochism"),
			TEXT("Faith"),
			TEXT("Cunning"),
			TEXT("Celerity"),
			TEXT("Allure")
		};
	}

	TArray<FString> MenuPreviewTexts()
	{
		return {
			TEXT("SINFUL ASCENSION"),
			TEXT("ETERNAL"),
			TEXT("Run SXP"),
			TEXT("Meta SXP"),
			TEXT("Ready to ascend."),
			TEXT("W/S navigate"),
			TEXT("E/Enter ascend"),
			TEXT("R withdraw Meta SXP"),
			TEXT("Q/Esc close")
		};
	}

	void AddManifestSpec(
		FCodeWidgetDesignerConversionManifest& Manifest,
		TSubclassOf<UUserWidget> WidgetClass,
		const FString& TargetAssetPath,
		const ECodeWidgetDesignerAssetRole Role,
		const int32 PriorityRank,
		const bool bRuntimeDefault,
		const TArray<FName>& ExpectedWidgetNames,
		const TArray<FName>& ExpectedBlueprintEvents,
		const TArray<FName>& ExpectedVisualStates,
		const bool bRequiresStableRootWrapper = false,
		TArray<FString> ExpectedPreviewTexts = TArray<FString>())
	{
		FCodeWidgetDesignerWidgetAssetSpec Spec;
		Spec.WidgetClass = WidgetClass;
		Spec.TargetAssetPath = TargetAssetPath;
		Spec.Role = Role;
		Spec.PriorityGroup = TEXT("SinfulAscensionAltar");
		Spec.PriorityRank = PriorityRank;
		Spec.bRuntimeDefault = bRuntimeDefault;
		Spec.bRequiresStableRootWrapper = bRequiresStableRootWrapper;
		Spec.ExpectedWidgetNames = ExpectedWidgetNames;
		Spec.ExpectedBlueprintEvents = ExpectedBlueprintEvents;
		Spec.ExpectedVisualStates = ExpectedVisualStates;
		Spec.ExpectedPreviewTexts = MoveTemp(ExpectedPreviewTexts);
		Manifest.WidgetAssets.Add(Spec);
	}
}

UProjectSinfulAscensionExchangeMenuWidget::UProjectSinfulAscensionExchangeMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	TitleFontAsset = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::TitleFontPath);
	BodyFontAsset = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::BodyFontPath);
	FrameTexture = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::FrameTexturePath);
	HazeTexture = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::HazeTexturePath);
	DividerTexture = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::DividerTexturePath);
	StatBoxTexture = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::StatBoxTexturePath);
	FooterOrnamentTexture = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::FooterOrnamentTexturePath);
	ModeGlyphTexture = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::ModeGlyphTexturePath);
	DefaultIconTexture = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::DefaultIconTexturePath);
	DefaultWatermarkTexture = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::DefaultWatermarkTexturePath);
	AttributeRowWidgetClass.Reset();
}

void UProjectSinfulAscensionExchangeMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshDisplay();
}

void UProjectSinfulAscensionExchangeMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshDisplay();
	FocusMenuWidget();
}

void UProjectSinfulAscensionExchangeMenuWidget::NativeDestruct()
{
	RowWidgetsByName.Empty();
	CachedRowOrder.Reset();
	ResolvedEntries.Reset();
	SinfulAscensionComponent.Reset();
	Super::NativeDestruct();
}

FReply UProjectSinfulAscensionExchangeMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (HandleMenuKey(InKeyEvent.GetKey()))
	{
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UProjectSinfulAscensionExchangeMenuWidget::SetSinfulAscensionComponent(UProjectSinfulAscensionComponent* InComponent)
{
	if (SinfulAscensionComponent.Get() == InComponent)
	{
		return;
	}

	SinfulAscensionComponent = InComponent;
	RefreshDisplay();
}

void UProjectSinfulAscensionExchangeMenuWidget::RefreshDisplay()
{
	BuildWidgetTree();
	InitializeVisualTree();

	CachedSnapshot = SinfulAscensionComponent.IsValid()
		? SinfulAscensionComponent->BuildSnapshot()
		: FProjectSinfulAscensionSnapshot();
	ResolvedEntries = BuildResolvedEntries();

	if (ResolvedEntries.IsEmpty())
	{
		SelectedIndex = INDEX_NONE;
	}
	else if (SelectedIndex == INDEX_NONE)
	{
		SelectedIndex = 0;
	}
	else
	{
		SelectedIndex = FMath::Clamp(SelectedIndex, 0, ResolvedEntries.Num() - 1);
	}

	if (DoesRowLayoutNeedRebuild(ResolvedEntries))
	{
		RebuildAttributeRows(ResolvedEntries);
	}

	RefreshVisualState();
}

void UProjectSinfulAscensionExchangeMenuWidget::FocusMenuWidget()
{
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

int32 UProjectSinfulAscensionExchangeMenuWidget::GetSelectedIndex() const
{
	return SelectedIndex;
}

FProjectSinfulAscensionSnapshot UProjectSinfulAscensionExchangeMenuWidget::GetCachedSnapshot() const
{
	return CachedSnapshot;
}

bool UProjectSinfulAscensionExchangeMenuWidget::BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree)
	{
		return false;
	}

	WidgetTree = TargetWidgetTree;
	bUsingNativeFallbackTree = true;
	bVisualTreeInitialized = false;
	BuildWidgetTree();
	InitializeVisualTree();

	CachedSnapshot = FProjectSinfulAscensionSnapshot();
	CachedSnapshot.CurrentRunSxp = 2400;
	CachedSnapshot.MetaBankSxp = 3271;
	CachedSnapshot.bEternalSinMode = true;
	ResolvedEntries = BuildDesignerPreviewEntries();
	SelectedIndex = ResolvedEntries.IsEmpty() ? INDEX_NONE : 0;
	RebuildAttributeRows(ResolvedEntries);
	RefreshVisualState();

	return TargetWidgetTree->RootWidget != nullptr;
}

bool UProjectSinfulAscensionExchangeMenuWidget::GatherCodeWidgetDesignerConversionManifest(
	FCodeWidgetDesignerConversionManifest& OutManifest) const
{
	using namespace ProjectSinfulExchangeMenuWidgetPrivate;

	OutManifest = FCodeWidgetDesignerConversionManifest();
	OutManifest.SystemName = TEXT("SinfulAscensionAltar");
	OutManifest.RootPath = TEXT("/Game/_Game/Widgets");
	OutManifest.MainFolder = TEXT("Main");
	OutManifest.GlobalFolder = TEXT("Global");
	OutManifest.AssetFolders = {
		TEXT("SinfulAscensionAltar/Assets/Fonts"),
		TEXT("SinfulAscensionAltar/Assets/Textures")
	};

	OutManifest.HostWidget.WidgetClass = UProjectSinfulAscensionExchangeMenuWidget::StaticClass();
	OutManifest.HostWidget.TargetAssetPath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Main/WBP_ProjectSinfulAscensionExchangeMenu");
	OutManifest.HostWidget.Role = ECodeWidgetDesignerAssetRole::Host;
	OutManifest.HostWidget.PriorityGroup = TEXT("SinfulAscensionAltar");
	OutManifest.HostWidget.PriorityRank = 10000;
	OutManifest.HostWidget.ExpectedWidgetNames = MenuWidgetNames();
	OutManifest.HostWidget.ExpectedBlueprintEvents = { TEXT("OnExchangeMenuStateApplied"), TEXT("OnExchangeMenuSelectionChanged") };
	OutManifest.HostWidget.ExpectedVisualStates = { TEXT("Selected"), TEXT("Hover"), TEXT("Disabled"), TEXT("Locked"), TEXT("Affordable"), TEXT("Unaffordable") };
	OutManifest.HostWidget.ExpectedPreviewTexts = MenuPreviewTexts();

	const TArray<FName> RowEvents = { TEXT("OnExchangeAttributeRowDataApplied"), TEXT("OnExchangeAttributeRowVisualStateChanged") };
	const TArray<FName> RowStates = { TEXT("Selected"), TEXT("Hover"), TEXT("Disabled"), TEXT("Locked"), TEXT("Affordable"), TEXT("Unaffordable"), TEXT("Marker") };

	AddManifestSpec(
		OutManifest,
		UProjectSinfulAscensionExchangeMenuGlobalWidget::StaticClass(),
		TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Global/WBP_ProjectSinfulAscensionExchangeMenuGlobal"),
		ECodeWidgetDesignerAssetRole::GlobalPanel,
		30000,
		true,
		MenuWidgetNames(),
		{ TEXT("OnExchangeMenuStateApplied"), TEXT("OnExchangeMenuSelectionChanged") },
		{ TEXT("Selected"), TEXT("Hover"), TEXT("Disabled"), TEXT("Locked"), TEXT("Affordable"), TEXT("Unaffordable") },
		false,
		MenuPreviewTexts());
	AddManifestSpec(
		OutManifest,
		UProjectSinfulAscensionExchangeAttributeRowGlobalWidget::StaticClass(),
		TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Global/WBP_ProjectSinfulAscensionExchangeAttributeRowGlobal"),
		ECodeWidgetDesignerAssetRole::GlobalTemplate,
		29000,
		true,
		RowWidgetNames(),
		RowEvents,
		RowStates,
		true);
	AddManifestSpec(
		OutManifest,
		UProjectSinfulAscensionExchangeRowsGlobalWidget::StaticClass(),
		TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Global/WBP_ProjectSinfulAscensionExchangeRowsGlobal"),
		ECodeWidgetDesignerAssetRole::GlobalPanel,
		28000,
		false,
		RowsGlobalWidgetNames(),
		{ TEXT("OnExchangeRowsApplied") },
		{ TEXT("Selected"), TEXT("Hover"), TEXT("Disabled"), TEXT("Locked"), TEXT("Affordable"), TEXT("Unaffordable") },
		true);
	AddManifestSpec(
		OutManifest,
		UProjectSinfulAscensionExchangeAttributeRowWidget::StaticClass(),
		TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Main/WBP_ProjectSinfulAscensionExchangeAttributeRow"),
		ECodeWidgetDesignerAssetRole::MainBase,
		10000,
		false,
		RowWidgetNames(),
		RowEvents,
		RowStates,
		true);
	AddManifestSpec(
		OutManifest,
		UProjectSinfulAscensionExchangeDetailPanelWidget::StaticClass(),
		TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Detail/WBP_ProjectSinfulAscensionExchangeDetailPanel"),
		ECodeWidgetDesignerAssetRole::Individual,
		9000,
		false,
		DetailWidgetNames(),
		{ TEXT("OnExchangeDetailPanelDataApplied") },
		{ TEXT("Selected"), TEXT("Locked"), TEXT("Unlocked"), TEXT("Watermark") },
		false,
		{ TEXT("Willpower") });

	const TArray<TPair<UClass*, FString>> IndividualRows = {
		{ UProjectSinfulAscensionExchangeWillpowerRowWidget::StaticClass(), TEXT("WBP_ProjectSinfulAscensionExchangeWillpowerRow") },
		{ UProjectSinfulAscensionExchangeSadismRowWidget::StaticClass(), TEXT("WBP_ProjectSinfulAscensionExchangeSadismRow") },
		{ UProjectSinfulAscensionExchangeMasochismRowWidget::StaticClass(), TEXT("WBP_ProjectSinfulAscensionExchangeMasochismRow") },
		{ UProjectSinfulAscensionExchangeFaithRowWidget::StaticClass(), TEXT("WBP_ProjectSinfulAscensionExchangeFaithRow") },
		{ UProjectSinfulAscensionExchangeCunningRowWidget::StaticClass(), TEXT("WBP_ProjectSinfulAscensionExchangeCunningRow") },
		{ UProjectSinfulAscensionExchangeCelerityRowWidget::StaticClass(), TEXT("WBP_ProjectSinfulAscensionExchangeCelerityRow") },
		{ UProjectSinfulAscensionExchangeAllureRowWidget::StaticClass(), TEXT("WBP_ProjectSinfulAscensionExchangeAllureRow") },
	};

	int32 RowPriority = 8000;
	for (const TPair<UClass*, FString>& RowSpec : IndividualRows)
	{
		AddManifestSpec(
			OutManifest,
			RowSpec.Key,
			FString::Printf(TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Rows/%s"), *RowSpec.Value),
			ECodeWidgetDesignerAssetRole::Individual,
			RowPriority--,
			false,
			RowWidgetNames(),
			RowEvents,
			RowStates,
			true,
			{ RowSpec.Value.Replace(TEXT("WBP_ProjectSinfulAscensionExchange"), TEXT("")).Replace(TEXT("Row"), TEXT("")) });
	}

	return true;
}

void UProjectSinfulAscensionExchangeMenuWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (RootCanvas && WidgetTree->RootWidget == RootCanvas)
	{
		return;
	}

	if (WidgetTree->RootWidget)
	{
		bUsingNativeFallbackTree = false;
		return;
	}

	bUsingNativeFallbackTree = true;

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	BackdropBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackdropBorder"));
	BackdropBorder->SetPadding(FMargin(0.0f));
	if (UCanvasPanelSlot* BackdropSlot = RootCanvas->AddChildToCanvas(BackdropBorder))
	{
		BackdropSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackdropSlot->SetOffsets(FMargin(0.0f));
	}

	FrameOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("FrameOverlay"));
	if (UCanvasPanelSlot* FrameSlot = RootCanvas->AddChildToCanvas(FrameOverlay))
	{
		FrameSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		FrameSlot->SetOffsets(FMargin(
			ProjectSinfulExchangeMenuWidgetPrivate::FullscreenInset,
			ProjectSinfulExchangeMenuWidgetPrivate::FullscreenInset,
			ProjectSinfulExchangeMenuWidgetPrivate::FullscreenInset,
			ProjectSinfulExchangeMenuWidgetPrivate::FullscreenInset));
	}

	BackgroundBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackgroundBorder"));
	BackgroundBorder->SetPadding(FMargin(0.0f));
	if (UOverlaySlot* BackgroundSlot = FrameOverlay->AddChildToOverlay(BackgroundBorder))
	{
		BackgroundSlot->SetHorizontalAlignment(HAlign_Fill);
		BackgroundSlot->SetVerticalAlignment(VAlign_Fill);
	}

	HazeImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("HazeImage"));
	if (UOverlaySlot* HazeSlot = FrameOverlay->AddChildToOverlay(HazeImage))
	{
		HazeSlot->SetHorizontalAlignment(HAlign_Fill);
		HazeSlot->SetVerticalAlignment(VAlign_Fill);
		HazeSlot->SetPadding(FMargin(10.0f));
	}

	FrameTextureImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("FrameTextureImage"));
	if (UOverlaySlot* FrameTextureSlot = FrameOverlay->AddChildToOverlay(FrameTextureImage))
	{
		FrameTextureSlot->SetHorizontalAlignment(HAlign_Fill);
		FrameTextureSlot->SetVerticalAlignment(VAlign_Fill);
	}

	ContentBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ContentBorder"));
	ContentBorder->SetPadding(FMargin(34.0f, 26.0f, 34.0f, 24.0f));
	ContentBorder->SetBrushColor(FLinearColor::Transparent);
	if (UOverlaySlot* ContentSlot = FrameOverlay->AddChildToOverlay(ContentBorder))
	{
		ContentSlot->SetHorizontalAlignment(HAlign_Fill);
		ContentSlot->SetVerticalAlignment(VAlign_Fill);
	}

	ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContentBox"));
	ContentBorder->SetContent(ContentBox);

	HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HeaderRow"));
	if (UVerticalBoxSlot* HeaderSlot = ContentBox->AddChildToVerticalBox(HeaderRow))
	{
		HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	}

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	if (UHorizontalBoxSlot* TitleSlot = HeaderRow->AddChildToHorizontalBox(TitleText))
	{
		TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TitleSlot->SetHorizontalAlignment(HAlign_Left);
		TitleSlot->SetVerticalAlignment(VAlign_Center);
		TitleSlot->SetPadding(FMargin(50.0f, 30.0f, 0.0f, 0.0f));
	}

	ModeBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ModeBox"));
	if (UHorizontalBoxSlot* ModeSlot = HeaderRow->AddChildToHorizontalBox(ModeBox))
	{
		ModeSlot->SetHorizontalAlignment(HAlign_Right);
		ModeSlot->SetVerticalAlignment(VAlign_Center);
		ModeSlot->SetPadding(FMargin(18.0f, 30.0f, 80.0f, 0.0f));
	}

	USizeBox* ModeGlyphSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ModeGlyphSizeBox"));
	ModeGlyphSizeBox->SetWidthOverride(12.0f);
	ModeGlyphSizeBox->SetHeightOverride(12.0f);
	if (UHorizontalBoxSlot* ModeGlyphSlot = ModeBox->AddChildToHorizontalBox(ModeGlyphSizeBox))
	{
		ModeGlyphSlot->SetHorizontalAlignment(HAlign_Center);
		ModeGlyphSlot->SetVerticalAlignment(VAlign_Center);
		ModeGlyphSlot->SetPadding(FMargin(0.0f, 1.0f, 8.0f, 0.0f));
	}

	ModeGlyphImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ModeGlyphImage"));
	ModeGlyphSizeBox->AddChild(ModeGlyphImage);

	ModeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ModeText"));
	ModeBox->AddChildToHorizontalBox(ModeText);

	USizeBox* TopDividerSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("TopDividerSizeBox"));
	TopDividerSizeBox->SetHeightOverride(10.0f);
	if (UVerticalBoxSlot* DividerSlot = ContentBox->AddChildToVerticalBox(TopDividerSizeBox))
	{
		DividerSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	}

	TopDividerImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("TopDividerImage"));
	TopDividerSizeBox->AddChild(TopDividerImage);

	ResourceRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ResourceRow"));
	if (UVerticalBoxSlot* ResourceSlot = ContentBox->AddChildToVerticalBox(ResourceRow))
	{
		ResourceSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 22.0f));
	}

	RunSxpBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RunSxpBorder"));
	RunSxpBorder->SetPadding(FMargin(22.0f, 10.0f, 22.0f, 10.0f));
	if (UHorizontalBoxSlot* RunSlot = ResourceRow->AddChildToHorizontalBox(RunSxpBorder))
	{
		RunSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RunSlot->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
	}

	UHorizontalBox* RunSxpContent = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RunSxpContent"));
	RunSxpBorder->SetContent(RunSxpContent);

	RunSxpLabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RunSxpLabelText"));
	if (UHorizontalBoxSlot* RunLabelSlot = RunSxpContent->AddChildToHorizontalBox(RunSxpLabelText))
	{
		RunLabelSlot->SetHorizontalAlignment(HAlign_Left);
		RunLabelSlot->SetVerticalAlignment(VAlign_Center);
	}

	RunSxpValueScaleBox = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("RunSxpValueScaleBox"));
	RunSxpValueScaleBox->SetStretch(EStretch::ScaleToFit);
	if (UHorizontalBoxSlot* RunValueScaleSlot = RunSxpContent->AddChildToHorizontalBox(RunSxpValueScaleBox))
	{
		RunValueScaleSlot->SetHorizontalAlignment(HAlign_Left);
		RunValueScaleSlot->SetVerticalAlignment(VAlign_Center);
		RunValueScaleSlot->SetPadding(FMargin(12.0f, 0.0f, 0.0f, 0.0f));
	}

	RunSxpValueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RunSxpValueText"));
	RunSxpValueScaleBox->AddChild(RunSxpValueText);

	MetaSxpBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MetaSxpBorder"));
	MetaSxpBorder->SetPadding(FMargin(22.0f, 10.0f, 22.0f, 10.0f));
	if (UHorizontalBoxSlot* MetaSlot = ResourceRow->AddChildToHorizontalBox(MetaSxpBorder))
	{
		MetaSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		MetaSlot->SetPadding(FMargin(6.0f, 0.0f, 0.0f, 0.0f));
	}

	UHorizontalBox* MetaSxpContent = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MetaSxpContent"));
	MetaSxpBorder->SetContent(MetaSxpContent);

	MetaSxpLabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MetaSxpLabelText"));
	if (UHorizontalBoxSlot* MetaLabelSlot = MetaSxpContent->AddChildToHorizontalBox(MetaSxpLabelText))
	{
		MetaLabelSlot->SetHorizontalAlignment(HAlign_Left);
		MetaLabelSlot->SetVerticalAlignment(VAlign_Center);
	}

	MetaSxpValueScaleBox = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("MetaSxpValueScaleBox"));
	MetaSxpValueScaleBox->SetStretch(EStretch::ScaleToFit);
	if (UHorizontalBoxSlot* MetaValueScaleSlot = MetaSxpContent->AddChildToHorizontalBox(MetaSxpValueScaleBox))
	{
		MetaValueScaleSlot->SetHorizontalAlignment(HAlign_Left);
		MetaValueScaleSlot->SetVerticalAlignment(VAlign_Center);
		MetaValueScaleSlot->SetPadding(FMargin(12.0f, 0.0f, 0.0f, 0.0f));
	}

	MetaSxpValueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MetaSxpValueText"));
	MetaSxpValueScaleBox->AddChild(MetaSxpValueText);

	BodyRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BodyRow"));
	if (UVerticalBoxSlot* BodySlot = ContentBox->AddChildToVerticalBox(BodyRow))
	{
		BodySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		BodySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}

	AttributeListBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AttributeListBorder"));
	AttributeListBorder->SetPadding(FMargin(0.0f));
	if (UHorizontalBoxSlot* ListSlot = BodyRow->AddChildToHorizontalBox(AttributeListBorder))
	{
		FSlateChildSize LeftSize(ESlateSizeRule::Automatic);
		ListSlot->SetSize(LeftSize);
		ListSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
	}

	RowsGlobalWidget = WidgetTree->ConstructWidget<UProjectSinfulAscensionExchangeRowsGlobalWidget>(
		UProjectSinfulAscensionExchangeRowsGlobalWidget::StaticClass(),
		TEXT("RowsGlobalWidget"));
	AttributeListBorder->SetContent(RowsGlobalWidget);
	AttributesScrollBox = nullptr;
	AttributesLayout = nullptr;

	DetailBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DetailBorder"));
	DetailBorder->SetPadding(FMargin(0.0f));
	if (UHorizontalBoxSlot* DetailSlot = BodyRow->AddChildToHorizontalBox(DetailBorder))
	{
		DetailSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	DetailOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("DetailOverlay"));
	DetailBorder->SetContent(DetailOverlay);

	DetailWatermarkImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("DetailWatermarkImage"));
	if (UOverlaySlot* WatermarkSlot = DetailOverlay->AddChildToOverlay(DetailWatermarkImage))
	{
		WatermarkSlot->SetHorizontalAlignment(HAlign_Right);
		WatermarkSlot->SetVerticalAlignment(VAlign_Center);
		WatermarkSlot->SetPadding(FMargin(0.0f, 28.0f, 42.0f, 34.0f));
	}

	DetailContentBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DetailContentBorder"));
	DetailContentBorder->SetPadding(FMargin(32.0f, 28.0f, 32.0f, 26.0f));
	DetailContentBorder->SetBrushColor(FLinearColor::Transparent);
	if (UOverlaySlot* DetailContentSlot = DetailOverlay->AddChildToOverlay(DetailContentBorder))
	{
		DetailContentSlot->SetHorizontalAlignment(HAlign_Fill);
		DetailContentSlot->SetVerticalAlignment(VAlign_Fill);
	}

	DetailContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DetailContentBox"));
	DetailContentBorder->SetContent(DetailContentBox);

	DetailNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailNameText"));
	if (UVerticalBoxSlot* DetailNameSlot = DetailContentBox->AddChildToVerticalBox(DetailNameText))
	{
		DetailNameSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	}

	DetailLevelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailLevelText"));
	if (UVerticalBoxSlot* DetailLevelSlot = DetailContentBox->AddChildToVerticalBox(DetailLevelText))
	{
		DetailLevelSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	}

	DetailCostText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailCostText"));
	if (UVerticalBoxSlot* DetailCostSlot = DetailContentBox->AddChildToVerticalBox(DetailCostText))
	{
		DetailCostSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}

	USizeBox* DetailDividerSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DetailDividerSizeBox"));
	DetailDividerSizeBox->SetWidthOverride(340.0f);
	DetailDividerSizeBox->SetHeightOverride(10.0f);
	if (UVerticalBoxSlot* DetailDividerSlot = DetailContentBox->AddChildToVerticalBox(DetailDividerSizeBox))
	{
		DetailDividerSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 14.0f));
	}

	DetailMilestoneDividerImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("DetailMilestoneDividerImage"));
	DetailDividerSizeBox->AddChild(DetailMilestoneDividerImage);

	DetailMilestoneFiveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailMilestoneFiveText"));
	if (UVerticalBoxSlot* MilestoneFiveSlot = DetailContentBox->AddChildToVerticalBox(DetailMilestoneFiveText))
	{
		MilestoneFiveSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	}

	DetailMilestoneTenText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailMilestoneTenText"));
	if (UVerticalBoxSlot* MilestoneTenSlot = DetailContentBox->AddChildToVerticalBox(DetailMilestoneTenText))
	{
		MilestoneTenSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}

	DetailFlavorText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailFlavorText"));
	DetailFlavorText->SetAutoWrapText(true);
	if (UVerticalBoxSlot* DetailFlavorSlot = DetailContentBox->AddChildToVerticalBox(DetailFlavorText))
	{
		DetailFlavorSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	USizeBox* FooterDividerSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("FooterDividerSizeBox"));
	FooterDividerSizeBox->SetHeightOverride(10.0f);
	if (UVerticalBoxSlot* FooterDividerSlot = ContentBox->AddChildToVerticalBox(FooterDividerSizeBox))
	{
		FooterDividerSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	}

	FooterDividerImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("FooterDividerImage"));
	FooterDividerSizeBox->AddChild(FooterDividerImage);

	FooterRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("FooterRow"));
	if (UVerticalBoxSlot* FooterRowSlot = ContentBox->AddChildToVerticalBox(FooterRow))
	{
		FooterRowSlot->SetPadding(FMargin(6.0f, 0.0f, 6.0f, 2.0f));
	}

	FooterStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FooterStatusText"));
	if (UHorizontalBoxSlot* FooterStatusSlot = FooterRow->AddChildToHorizontalBox(FooterStatusText))
	{
		FooterStatusSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		FooterStatusSlot->SetHorizontalAlignment(HAlign_Left);
		FooterStatusSlot->SetVerticalAlignment(VAlign_Center);
		FooterStatusSlot->SetPadding(FMargin(2.0f, 0.0f, 0.0f, 0.0f));
	}

	FooterControlsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FooterControlsText"));
	if (UHorizontalBoxSlot* FooterControlsSlot = FooterRow->AddChildToHorizontalBox(FooterControlsText))
	{
		FooterControlsSlot->SetHorizontalAlignment(HAlign_Right);
		FooterControlsSlot->SetVerticalAlignment(VAlign_Center);
		FooterControlsSlot->SetPadding(FMargin(20.0f, 0.0f, 0.0f, 0.0f));
	}
}

void UProjectSinfulAscensionExchangeMenuWidget::InitializeVisualTree()
{
	if (bVisualTreeInitialized)
	{
		return;
	}

	if (!bUsingNativeFallbackTree)
	{
		bVisualTreeInitialized = true;
		return;
	}

	if (BackdropBorder)
	{
		const FSlateRoundedBoxBrush BackdropBrush(
			ProjectSinfulExchangeMenuWidgetPrivate::BackdropTint,
			0.0f,
			FSlateColor(FLinearColor::Transparent),
			0.0f,
			FVector2f(1920.0f, 1080.0f));
		BackdropBorder->SetBrush(BackdropBrush);
	}

	if (BackgroundBorder)
	{
		const FSlateRoundedBoxBrush BackgroundBrush(
			ProjectSinfulExchangeMenuWidgetPrivate::FrameFillTint,
			ProjectSinfulExchangeMenuWidgetPrivate::FrameCornerRadius,
			FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::FrameOutlineTint),
			ProjectSinfulExchangeMenuWidgetPrivate::FrameOutlineWidth,
			FVector2f(1800.0f, 1000.0f));
		BackgroundBorder->SetBrush(BackgroundBrush);
	}

	if (FrameTextureImage)
	{
		FrameTextureImage->SetBrushFromTexture(
			ResolveTexture(FrameTexture, ProjectSinfulExchangeMenuWidgetPrivate::FrameTexturePath),
			false);
		FrameTextureImage->SetColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::FrameOutlineTint.CopyWithNewOpacity(0.86f));
	}

	if (HazeImage)
	{
		HazeImage->SetBrushFromTexture(
			ResolveTexture(HazeTexture, ProjectSinfulExchangeMenuWidgetPrivate::HazeTexturePath),
			false);
		HazeImage->SetColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::HazeTint);
	}

	const FSlateRoundedBoxBrush SectionBrush(
		ProjectSinfulExchangeMenuWidgetPrivate::SectionFillTint,
		12.0f,
		FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SectionOutlineTint),
		1.0f,
		FVector2f(600.0f, 400.0f));
	if (RunSxpBorder)
	{
		RunSxpBorder->SetBrush(SectionBrush);
	}

	if (MetaSxpBorder)
	{
		MetaSxpBorder->SetBrush(SectionBrush);
	}

	if (AttributeListBorder)
	{
		AttributeListBorder->SetBrush(SectionBrush);
	}

	if (DetailBorder)
	{
		DetailBorder->SetBrush(SectionBrush);
	}

	if (TitleText)
	{
		TitleText->SetText(LOCTEXT("MenuTitle", "SINFUL ASCENSION"));
		TitleText->SetFont(MakeTitleFont(46, 2));
		TitleText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::TitleTint));
		TitleText->SetShadowOffset(FVector2D(0.0f, 0.45f));
		TitleText->SetShadowColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::ShadowTint);
	}

	if (ModeGlyphImage)
	{
		ModeGlyphImage->SetBrushFromTexture(
			ResolveTexture(ModeGlyphTexture, ProjectSinfulExchangeMenuWidgetPrivate::ModeGlyphTexturePath),
			false);
		ModeGlyphImage->SetColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::AccentTint.CopyWithNewOpacity(0.96f));
	}

	if (ModeText)
	{
		ModeText->SetFont(MakeTitleFont(20, 0));
		ModeText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::TitleTint));
		ModeText->SetShadowOffset(FVector2D(0.0f, 0.22f));
		ModeText->SetShadowColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::ShadowTint.CopyWithNewOpacity(0.22f));
	}

	if (TopDividerImage)
	{
		TopDividerImage->SetBrushFromTexture(
			ResolveTexture(DividerTexture, ProjectSinfulExchangeMenuWidgetPrivate::DividerTexturePath),
			false);
		TopDividerImage->SetColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::AccentTint.CopyWithNewOpacity(0.88f));
	}

	if (FooterDividerImage)
	{
		FooterDividerImage->SetBrushFromTexture(
			ResolveTexture(FooterOrnamentTexture, ProjectSinfulExchangeMenuWidgetPrivate::FooterOrnamentTexturePath),
			false);
		FooterDividerImage->SetColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::AccentTint.CopyWithNewOpacity(0.82f));
	}

	if (RunSxpLabelText)
	{
		RunSxpLabelText->SetText(LOCTEXT("RunSxpLabel", "Run SXP"));
		RunSxpLabelText->SetFont(MakeBodyFont(25, 0));
		RunSxpLabelText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::PrimaryTextTint));
	}

	if (RunSxpValueText)
	{
		RunSxpValueText->SetFont(MakeBodyFont(28, 0));
		RunSxpValueText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::AccentTint));
	}

	if (MetaSxpLabelText)
	{
		MetaSxpLabelText->SetText(LOCTEXT("MetaSxpLabel", "Meta SXP"));
		MetaSxpLabelText->SetFont(MakeBodyFont(25, 0));
		MetaSxpLabelText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::PrimaryTextTint));
	}

	if (MetaSxpValueText)
	{
		MetaSxpValueText->SetFont(MakeBodyFont(28, 0));
		MetaSxpValueText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::AccentTint));
	}

	if (DetailNameText)
	{
		DetailNameText->SetFont(MakeTitleFont(50, 0));
		DetailNameText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::PrimaryTextTint));
		DetailNameText->SetShadowOffset(FVector2D(0.0f, 0.65f));
		DetailNameText->SetShadowColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::ShadowTint);
	}

	if (DetailLevelText)
	{
		DetailLevelText->SetFont(MakeBodyFont(28, 0));
		DetailLevelText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::PrimaryTextTint));
	}

	if (DetailCostText)
	{
		DetailCostText->SetFont(MakeBodyFont(31, 0));
		DetailCostText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::AccentTint));
	}

	if (DetailMilestoneDividerImage)
	{
		DetailMilestoneDividerImage->SetBrushFromTexture(
			ResolveTexture(DividerTexture, ProjectSinfulExchangeMenuWidgetPrivate::DividerTexturePath),
			false);
		DetailMilestoneDividerImage->SetColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::AccentTint.CopyWithNewOpacity(0.62f));
	}

	if (DetailMilestoneFiveText)
	{
		DetailMilestoneFiveText->SetFont(MakeBodyFont(23, 0));
		DetailMilestoneFiveText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SecondaryTextTint));
	}

	if (DetailMilestoneTenText)
	{
		DetailMilestoneTenText->SetFont(MakeBodyFont(23, 0));
		DetailMilestoneTenText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SecondaryTextTint));
	}

	if (DetailFlavorText)
	{
		DetailFlavorText->SetFont(MakeBodyFont(22, 0));
		DetailFlavorText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SecondaryTextTint));
		DetailFlavorText->SetShadowOffset(FVector2D(0.0f, 0.35f));
		DetailFlavorText->SetShadowColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::ShadowTint.CopyWithNewOpacity(0.18f));
	}

	if (FooterStatusText)
	{
		FooterStatusText->SetFont(MakeBodyFont(18, 0));
		FooterStatusText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SecondaryTextTint));
	}

	if (FooterControlsText)
	{
		FooterControlsText->SetFont(MakeBodyFont(18, 0));
		FooterControlsText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::TitleTint));
	}

	bVisualTreeInitialized = true;
}

void UProjectSinfulAscensionExchangeMenuWidget::RefreshVisualState()
{
	if (ModeText)
	{
		ModeText->SetText(CachedSnapshot.bEternalSinMode ? LOCTEXT("EternalMode", "ETERNAL") : LOCTEXT("RunMode", "RUN"));
	}

	if (RunSxpValueText)
	{
		RunSxpValueText->SetText(FText::AsNumber(CachedSnapshot.CurrentRunSxp));
	}

	if (MetaSxpValueText)
	{
		MetaSxpValueText->SetText(FText::AsNumber(CachedSnapshot.MetaBankSxp));
	}

	RefreshAttributeRows();
	RefreshDetailPanel();
	RefreshFooterState();

	const FProjectSinfulAscensionExchangeResolvedEntry* SelectedEntry = GetSelectedEntry();
	OnExchangeMenuStateApplied(CachedSnapshot, SelectedIndex);
	OnExchangeMenuSelectionChanged(SelectedIndex, SelectedEntry ? SelectedEntry->AttributeName : NAME_None);
}

TArray<FProjectSinfulAscensionExchangeResolvedEntry> UProjectSinfulAscensionExchangeMenuWidget::BuildResolvedEntries() const
{
	TMap<FName, const FProjectSinAttributeState*> RuntimeDataByName;
	for (const FProjectSinAttributeState& AttributeState : CachedSnapshot.Attributes)
	{
		FName AttributeName = ProjectSinfulExchangeMenuWidgetPrivate::BuildAttributeName(AttributeState.Attribute);
		if (AttributeName.IsNone())
		{
			AttributeName = FName(*AttributeState.DisplayName.ToString().Replace(TEXT(" "), TEXT("")));
		}

		if (!AttributeName.IsNone())
		{
			RuntimeDataByName.Add(AttributeName, &AttributeState);
		}
	}

	TArray<FProjectSinfulAscensionExchangeResolvedEntry> BuiltEntries;
	const UEFProjectUISettings* UISettings = UEFProjectUISettings::Get();
	static const TArray<FProjectSinfulAscensionEntryDefinition> EmptyDefinitions;
	const TArray<FProjectSinfulAscensionEntryDefinition>& Definitions = UISettings
		? UISettings->SinfulAscensionEntryDefinitions
		: EmptyDefinitions;

	for (const FProjectSinfulAscensionEntryDefinition& Definition : Definitions)
	{
		const FProjectSinAttributeState* const* RuntimeState = RuntimeDataByName.Find(Definition.AttributeName);
		if (!RuntimeState || !(*RuntimeState))
		{
			continue;
		}

		FProjectSinfulAscensionExchangeResolvedEntry& Entry = BuiltEntries.AddDefaulted_GetRef();
		Entry.AttributeName = Definition.AttributeName;
		Entry.Attribute = (*RuntimeState)->Attribute;
		Entry.SortOrder = Definition.SortOrder;
		Entry.RowData.AttributeName = Definition.AttributeName;
		Entry.RowData.DisplayLabel = Definition.MenuDisplayLabel.IsEmpty()
			? ProjectSinfulExchangeMenuWidgetPrivate::NormalizeMenuLabel((*RuntimeState)->DisplayName)
			: Definition.MenuDisplayLabel;
		Entry.RowData.IconTexture = Definition.MenuIconTexture.IsNull() ? Definition.IconTexture : Definition.MenuIconTexture;
		Entry.RowData.AccentTint = Definition.AccentTint;
		Entry.RowData.Level = (*RuntimeState)->Level;
		Entry.RowData.Cost = (*RuntimeState)->NextLevelCost;
		Entry.RowData.RowWidth = ProjectSinfulExchangeMenuWidgetPrivate::RowWidth;
		Entry.RowData.RowHeight = ProjectSinfulExchangeMenuWidgetPrivate::RowHeight;
		Entry.RowData.bAffordable = CachedSnapshot.CurrentRunSxp >= (*RuntimeState)->NextLevelCost;
		Entry.DetailName = FText::FromString(Entry.RowData.DisplayLabel);
		Entry.DetailDescription = Definition.DescriptionText.IsEmpty()
			? BuildAttributeFlavor((*RuntimeState)->Attribute)
			: FText::FromString(Definition.DescriptionText);
		Entry.MilestoneFiveLabel = Definition.MilestoneFiveText.IsEmpty()
			? FText::GetEmpty()
			: FText::FromString(Definition.MilestoneFiveText);
		Entry.MilestoneTenLabel = Definition.MilestoneTenText.IsEmpty()
			? FText::GetEmpty()
			: FText::FromString(Definition.MilestoneTenText);
		Entry.DetailWatermarkTexture = Definition.MenuWatermarkTexture.IsNull()
			? (Definition.MenuIconTexture.IsNull() ? Definition.IconTexture : Definition.MenuIconTexture)
			: Definition.MenuWatermarkTexture;
		Entry.bMilestoneFiveUnlocked = (*RuntimeState)->bMilestone5Unlocked;
		Entry.bMilestoneTenUnlocked = (*RuntimeState)->bMilestone10Unlocked;
		RuntimeDataByName.Remove(Definition.AttributeName);
	}

	int32 FallbackSortOffset = 0;
	for (const TPair<FName, const FProjectSinAttributeState*>& Pair : RuntimeDataByName)
	{
		if (!Pair.Value)
		{
			continue;
		}

		FProjectSinfulAscensionExchangeResolvedEntry& Entry = BuiltEntries.AddDefaulted_GetRef();
		Entry.AttributeName = Pair.Key;
		Entry.Attribute = Pair.Value->Attribute;
		Entry.SortOrder = ProjectSinfulExchangeMenuWidgetPrivate::RuntimeFallbackSortBase + FallbackSortOffset++;
		Entry.RowData.AttributeName = Pair.Key;
		Entry.RowData.DisplayLabel = ProjectSinfulExchangeMenuWidgetPrivate::NormalizeMenuLabel(Pair.Value->DisplayName);
		Entry.RowData.IconTexture.Reset();
		Entry.RowData.AccentTint = ProjectSinfulExchangeMenuWidgetPrivate::AccentTint;
		Entry.RowData.Level = Pair.Value->Level;
		Entry.RowData.Cost = Pair.Value->NextLevelCost;
		Entry.RowData.RowWidth = ProjectSinfulExchangeMenuWidgetPrivate::RowWidth;
		Entry.RowData.RowHeight = ProjectSinfulExchangeMenuWidgetPrivate::RowHeight;
		Entry.RowData.bAffordable = CachedSnapshot.CurrentRunSxp >= Pair.Value->NextLevelCost;
		Entry.DetailName = FText::FromString(Entry.RowData.DisplayLabel);
		Entry.DetailDescription = BuildAttributeFlavor(Pair.Value->Attribute);
		Entry.MilestoneFiveLabel = LOCTEXT("FallbackMilestoneFive", "Milestone V");
		Entry.MilestoneTenLabel = LOCTEXT("FallbackMilestoneTen", "Milestone X");
		Entry.DetailWatermarkTexture.Reset();
		Entry.bMilestoneFiveUnlocked = Pair.Value->bMilestone5Unlocked;
		Entry.bMilestoneTenUnlocked = Pair.Value->bMilestone10Unlocked;
	}

	BuiltEntries.Sort([](const FProjectSinfulAscensionExchangeResolvedEntry& Left, const FProjectSinfulAscensionExchangeResolvedEntry& Right)
	{
		if (Left.SortOrder != Right.SortOrder)
		{
			return Left.SortOrder < Right.SortOrder;
		}

		return Left.AttributeName.LexicalLess(Right.AttributeName);
	});

	return BuiltEntries;
}

TArray<FProjectSinfulAscensionExchangeResolvedEntry> UProjectSinfulAscensionExchangeMenuWidget::BuildDesignerPreviewEntries() const
{
	struct FPreviewAttribute
	{
		FName AttributeName = NAME_None;
		EProjectSinAttribute Attribute = EProjectSinAttribute::Willpower;
		int32 Level = 0;
		int32 Cost = 80;
		bool bMilestoneFiveUnlocked = false;
		bool bMilestoneTenUnlocked = false;
	};

	const TArray<FPreviewAttribute> PreviewAttributes = {
		{ FName(TEXT("Willpower")), EProjectSinAttribute::Willpower, 1, 204, false, false },
		{ FName(TEXT("Sadism")), EProjectSinAttribute::Sadism, 0, 80, false, false },
		{ FName(TEXT("Masochism")), EProjectSinAttribute::Masochism, 0, 80, false, false },
		{ FName(TEXT("Faith")), EProjectSinAttribute::Faith, 0, 80, false, false },
		{ FName(TEXT("Cunning")), EProjectSinAttribute::Cunning, 0, 80, false, false },
		{ FName(TEXT("Celerity")), EProjectSinAttribute::Celerity, 0, 80, false, false },
		{ FName(TEXT("Allure")), EProjectSinAttribute::Allure, 0, 80, false, false },
	};

	const UEFProjectUISettings* UISettings = UEFProjectUISettings::Get();
	const auto FindDefinition = [UISettings](const FName AttributeName) -> const FProjectSinfulAscensionEntryDefinition*
	{
		return UISettings
			? UISettings->SinfulAscensionEntryDefinitions.FindByPredicate(
				[AttributeName](const FProjectSinfulAscensionEntryDefinition& Definition)
				{
					return Definition.AttributeName == AttributeName;
				})
			: nullptr;
	};

	TArray<FProjectSinfulAscensionExchangeResolvedEntry> BuiltEntries;
	BuiltEntries.Reserve(PreviewAttributes.Num());

	for (int32 Index = 0; Index < PreviewAttributes.Num(); ++Index)
	{
		const FPreviewAttribute& PreviewAttribute = PreviewAttributes[Index];
		const FProjectSinfulAscensionEntryDefinition* Definition = FindDefinition(PreviewAttribute.AttributeName);

		FString PreviewDisplayLabel = Definition ? Definition->MenuDisplayLabel : FString();
		if (PreviewDisplayLabel.IsEmpty() && Definition)
		{
			PreviewDisplayLabel = Definition->DisplayLabel;
		}
		if (PreviewDisplayLabel.IsEmpty())
		{
			PreviewDisplayLabel = PreviewAttribute.AttributeName.ToString();
		}

		FProjectSinfulAscensionExchangeResolvedEntry& Entry = BuiltEntries.AddDefaulted_GetRef();
		Entry.AttributeName = PreviewAttribute.AttributeName;
		Entry.Attribute = PreviewAttribute.Attribute;
		Entry.SortOrder = Definition ? Definition->SortOrder : ((Index + 1) * 10);
		Entry.RowData.AttributeName = PreviewAttribute.AttributeName;
		Entry.RowData.DisplayLabel = PreviewDisplayLabel;
		Entry.RowData.IconTexture = Definition
			? (Definition->MenuIconTexture.IsNull() ? Definition->IconTexture : Definition->MenuIconTexture)
			: TSoftObjectPtr<UTexture2D>();
		Entry.RowData.AccentTint = Definition ? Definition->AccentTint : ProjectSinfulExchangeMenuWidgetPrivate::AccentTint;
		Entry.RowData.Level = PreviewAttribute.Level;
		Entry.RowData.Cost = PreviewAttribute.Cost;
		Entry.RowData.RowWidth = ProjectSinfulExchangeMenuWidgetPrivate::RowWidth;
		Entry.RowData.RowHeight = ProjectSinfulExchangeMenuWidgetPrivate::RowHeight;
		Entry.RowData.bAffordable = CachedSnapshot.CurrentRunSxp >= PreviewAttribute.Cost;
		Entry.DetailName = FText::FromString(PreviewDisplayLabel);
		Entry.DetailDescription = Definition && !Definition->DescriptionText.IsEmpty()
			? FText::FromString(Definition->DescriptionText)
			: BuildAttributeFlavor(PreviewAttribute.Attribute);
		Entry.MilestoneFiveLabel = Definition
			? (Definition->MilestoneFiveText.IsEmpty() ? FText::GetEmpty() : FText::FromString(Definition->MilestoneFiveText))
			: LOCTEXT("PreviewMilestoneFive", "Milestone V");
		Entry.MilestoneTenLabel = Definition
			? (Definition->MilestoneTenText.IsEmpty() ? FText::GetEmpty() : FText::FromString(Definition->MilestoneTenText))
			: LOCTEXT("PreviewMilestoneTen", "Milestone X");
		Entry.DetailWatermarkTexture = Definition
			? (Definition->MenuWatermarkTexture.IsNull()
				? (Definition->MenuIconTexture.IsNull() ? Definition->IconTexture : Definition->MenuIconTexture)
				: Definition->MenuWatermarkTexture)
			: TSoftObjectPtr<UTexture2D>();
		Entry.bMilestoneFiveUnlocked = PreviewAttribute.bMilestoneFiveUnlocked;
		Entry.bMilestoneTenUnlocked = PreviewAttribute.bMilestoneTenUnlocked;
	}

	BuiltEntries.Sort([](const FProjectSinfulAscensionExchangeResolvedEntry& Left, const FProjectSinfulAscensionExchangeResolvedEntry& Right)
	{
		if (Left.SortOrder != Right.SortOrder)
		{
			return Left.SortOrder < Right.SortOrder;
		}
		return Left.AttributeName.LexicalLess(Right.AttributeName);
	});

	return BuiltEntries;
}

bool UProjectSinfulAscensionExchangeMenuWidget::DoesRowLayoutNeedRebuild(const TArray<FProjectSinfulAscensionExchangeResolvedEntry>& InEntries) const
{
	if (CachedRowOrder.Num() != InEntries.Num())
	{
		return true;
	}

	for (int32 Index = 0; Index < InEntries.Num(); ++Index)
	{
		if (!CachedRowOrder.IsValidIndex(Index) || CachedRowOrder[Index] != InEntries[Index].AttributeName)
		{
			return true;
		}
	}

	return false;
}

void UProjectSinfulAscensionExchangeMenuWidget::RebuildAttributeRows(const TArray<FProjectSinfulAscensionExchangeResolvedEntry>& InEntries)
{
	RowWidgetsByName.Empty();
	CachedRowOrder.Reset();

	const TSubclassOf<UProjectSinfulAscensionExchangeAttributeRowWidget> RowClass = ResolveRowWidgetClass();
	UClass* EffectiveRowClass = RowClass ? RowClass.Get() : UProjectSinfulAscensionExchangeAttributeRowWidget::StaticClass();

	if (RowsGlobalWidget)
	{
		TArray<FProjectSinfulAscensionExchangeAttributeRowDisplayData> RowDataArray;
		RowDataArray.Reserve(InEntries.Num());
		for (int32 Index = 0; Index < InEntries.Num(); ++Index)
		{
			FProjectSinfulAscensionExchangeAttributeRowDisplayData RowData = InEntries[Index].RowData;
			RowData.bSelected = (Index == SelectedIndex);
			RowData.bAffordable = CachedSnapshot.CurrentRunSxp >= RowData.Cost;
			RowDataArray.Add(RowData);
			CachedRowOrder.Add(InEntries[Index].AttributeName);
		}

		RowsGlobalWidget->ApplyRows(RowDataArray, RowClass);
		for (const FProjectSinfulAscensionExchangeResolvedEntry& Entry : InEntries)
		{
			if (UProjectSinfulAscensionExchangeAttributeRowWidget* RowWidget = RowsGlobalWidget->FindRowWidgetByAttribute(Entry.AttributeName))
			{
				RowWidgetsByName.Add(Entry.AttributeName, RowWidget);
			}
		}
		return;
	}

	if (!AttributesLayout)
	{
		return;
	}

	AttributesLayout->ClearChildren();

	for (int32 Index = 0; Index < InEntries.Num(); ++Index)
	{
		const FProjectSinfulAscensionExchangeResolvedEntry& Entry = InEntries[Index];
		UProjectSinfulAscensionExchangeAttributeRowWidget* RowWidget = nullptr;
		if (APlayerController* OwningPlayer = GetOwningPlayer())
		{
			RowWidget = CreateWidget<UProjectSinfulAscensionExchangeAttributeRowWidget>(
				OwningPlayer,
				EffectiveRowClass);
		}

		if (!RowWidget && GetWorld())
		{
			RowWidget = CreateWidget<UProjectSinfulAscensionExchangeAttributeRowWidget>(
				GetWorld(),
				EffectiveRowClass);
		}

		if (!RowWidget)
		{
			continue;
		}

		FProjectSinfulAscensionExchangeAttributeRowDisplayData RowData = Entry.RowData;
		RowData.bSelected = (Index == SelectedIndex);
		RowWidget->ApplyDisplayData(RowData);

		if (UVerticalBoxSlot* RowSlot = AttributesLayout->AddChildToVerticalBox(RowWidget))
		{
			if (Index > 0)
			{
				RowSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
			}
		}

		RowWidgetsByName.Add(Entry.AttributeName, RowWidget);
		CachedRowOrder.Add(Entry.AttributeName);
	}
}

void UProjectSinfulAscensionExchangeMenuWidget::RefreshAttributeRows()
{
	for (int32 Index = 0; Index < ResolvedEntries.Num(); ++Index)
	{
		FProjectSinfulAscensionExchangeResolvedEntry& Entry = ResolvedEntries[Index];
		Entry.RowData.bSelected = (Index == SelectedIndex);
		Entry.RowData.bAffordable = CachedSnapshot.CurrentRunSxp >= Entry.RowData.Cost;

		if (TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget>* ExistingWidget = RowWidgetsByName.Find(Entry.AttributeName))
		{
			if (UProjectSinfulAscensionExchangeAttributeRowWidget* RowWidget = ExistingWidget->Get())
			{
				RowWidget->ApplyDisplayData(Entry.RowData);
				if (RowsGlobalWidget && Entry.RowData.bSelected)
				{
					RowsGlobalWidget->ScrollRowWidgetIntoView(RowWidget);
				}
				else if (AttributesScrollBox && Entry.RowData.bSelected)
				{
					AttributesScrollBox->ScrollWidgetIntoView(RowWidget, true, EDescendantScrollDestination::Center, 24.0f);
				}
			}
		}
	}
}

void UProjectSinfulAscensionExchangeMenuWidget::RefreshDetailPanel()
{
	const FProjectSinfulAscensionExchangeDetailDisplayData DetailData = BuildDetailDisplayData(GetSelectedEntry());

	if (DetailNameText)
	{
		DetailNameText->SetText(DetailData.DetailName);
	}

	if (DetailLevelText)
	{
		DetailLevelText->SetText(DetailData.LevelText);
	}

	if (DetailCostText)
	{
		DetailCostText->SetText(DetailData.CostText);
		if (bUsingNativeFallbackTree)
		{
			DetailCostText->SetColorAndOpacity(FSlateColor(DetailData.AccentTint.CopyWithNewOpacity(0.98f)));
		}
	}

	if (DetailMilestoneFiveText)
	{
		DetailMilestoneFiveText->SetText(DetailData.MilestoneFiveText);
	}

	if (DetailMilestoneTenText)
	{
		DetailMilestoneTenText->SetText(DetailData.MilestoneTenText);
	}

	if (DetailFlavorText)
	{
		DetailFlavorText->SetText(DetailData.FlavorText);
	}

	if (DetailWatermarkImage)
	{
		if (DetailData.bHasSelection)
		{
			const TSoftObjectPtr<UTexture2D> WatermarkTexture = DetailData.WatermarkTexture.IsNull()
				? (DefaultWatermarkTexture.IsNull() ? DefaultIconTexture : DefaultWatermarkTexture)
				: DetailData.WatermarkTexture;
			DetailWatermarkImage->SetBrushFromTexture(
				ResolveTexture(WatermarkTexture, ProjectSinfulExchangeMenuWidgetPrivate::DefaultWatermarkTexturePath),
				false);
			if (bUsingNativeFallbackTree)
			{
				DetailWatermarkImage->SetColorAndOpacity(DetailData.AccentTint.CopyWithNewOpacity(0.08f));
				DetailWatermarkImage->SetDesiredSizeOverride(FVector2D(340.0f, 340.0f));
			}
			DetailWatermarkImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			DetailWatermarkImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UProjectSinfulAscensionExchangeMenuWidget::RefreshFooterState()
{
	if (!FooterControlsText)
	{
		return;
	}

	FooterControlsText->SetText(LOCTEXT(
		"FooterControls",
		"W/S navigate   |   E/Enter ascend   |   R withdraw Meta SXP   |   Q/Esc close"));

	if (!FooterStatusText)
	{
		return;
	}

	const FProjectSinfulAscensionExchangeResolvedEntry* SelectedEntry = GetSelectedEntry();
	if (!SelectedEntry)
	{
		FooterStatusText->SetText(LOCTEXT("FooterUnavailable", "Approach the altar again once the player runtime has initialized."));
		if (bUsingNativeFallbackTree)
		{
			FooterStatusText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SecondaryTextTint));
		}
		return;
	}

	if (SelectedEntry->RowData.Cost <= 0)
	{
		FooterStatusText->SetText(LOCTEXT("FooterMaxed", "Maximum level reached."));
		if (bUsingNativeFallbackTree)
		{
			FooterStatusText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SecondaryTextTint));
		}
		return;
	}

	const int32 MissingSxp = FMath::Max(SelectedEntry->RowData.Cost - CachedSnapshot.CurrentRunSxp, 0);
	if (MissingSxp > 0)
	{
		FooterStatusText->SetText(FText::Format(LOCTEXT("FooterMissingSxp", "Need {0} more SXP."), FText::AsNumber(MissingSxp)));
		if (bUsingNativeFallbackTree)
		{
			FooterStatusText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::WarningTint));
		}
		return;
	}

	FooterStatusText->SetText(LOCTEXT("FooterReady", "Ready to ascend."));
	if (bUsingNativeFallbackTree)
	{
		FooterStatusText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::AccentTint));
	}
}

void UProjectSinfulAscensionExchangeMenuWidget::NavigateSelectionByDirection(const int32 Direction)
{
	if (Direction == 0 || ResolvedEntries.IsEmpty())
	{
		return;
	}

	if (SelectedIndex == INDEX_NONE)
	{
		SelectedIndex = 0;
	}
	else
	{
		SelectedIndex = (SelectedIndex + Direction + ResolvedEntries.Num()) % ResolvedEntries.Num();
	}

	RefreshVisualState();
}

void UProjectSinfulAscensionExchangeMenuWidget::ConfirmSelection()
{
	const FProjectSinfulAscensionExchangeResolvedEntry* SelectedEntry = GetSelectedEntry();
	if (!SelectedEntry)
	{
		return;
	}

	OnPurchaseRequested.Broadcast(SelectedEntry->Attribute);
}

bool UProjectSinfulAscensionExchangeMenuWidget::HandleMenuKey(const FKey& Key)
{
	if (Key == EKeys::Up || Key == EKeys::W)
	{
		NavigateSelectionByDirection(-1);
		return true;
	}

	if (Key == EKeys::Down || Key == EKeys::S)
	{
		NavigateSelectionByDirection(1);
		return true;
	}

	if (Key == EKeys::Enter || Key == EKeys::E)
	{
		ConfirmSelection();
		return true;
	}

	if (Key == EKeys::R)
	{
		OnWithdrawRequested.Broadcast();
		return true;
	}

	if (Key == EKeys::Q || Key == EKeys::Escape)
	{
		OnCloseRequested.Broadcast();
		return true;
	}

	return false;
}

TSubclassOf<UProjectSinfulAscensionExchangeAttributeRowWidget> UProjectSinfulAscensionExchangeMenuWidget::ResolveRowWidgetClass() const
{
	if (!AttributeRowWidgetClass.IsNull())
	{
		if (UClass* LoadedClass = AttributeRowWidgetClass.LoadSynchronous())
		{
			return LoadedClass;
		}
	}

	if (UClass* ResolvedClass = ProjectWidgetClassResolver::ResolveWidgetClassWithPriority(
		FSoftClassPath(),
		UProjectSinfulAscensionExchangeAttributeRowGlobalWidget::StaticClass(),
		TEXT("ProjectSinfulAscensionAltarExchangeAttributeRow"),
		TEXT("SinfulAscensionAltar")))
	{
		if (ResolvedClass->IsChildOf(UProjectSinfulAscensionExchangeAttributeRowWidget::StaticClass()))
		{
			return ResolvedClass;
		}
	}

	return UProjectSinfulAscensionExchangeAttributeRowGlobalWidget::StaticClass();
}

UTexture2D* UProjectSinfulAscensionExchangeMenuWidget::ResolveTexture(
	const TSoftObjectPtr<UTexture2D>& AssetPtr,
	const TCHAR* FallbackPath) const
{
	if (!AssetPtr.IsNull())
	{
		if (UTexture2D* LoadedTexture = AssetPtr.LoadSynchronous())
		{
			return LoadedTexture;
		}
	}

	return Cast<UTexture2D>(ProjectSinfulExchangeMenuWidgetPrivate::LoadObjectByPath(FallbackPath));
}

UObject* UProjectSinfulAscensionExchangeMenuWidget::ResolveStyleAsset(
	const TSoftObjectPtr<UObject>& AssetPtr,
	const TCHAR* FallbackPath) const
{
	if (!AssetPtr.IsNull())
	{
		if (UObject* LoadedObject = AssetPtr.LoadSynchronous())
		{
			return LoadedObject;
		}
	}

	return ProjectSinfulExchangeMenuWidgetPrivate::LoadObjectByPath(FallbackPath);
}

FSlateFontInfo UProjectSinfulAscensionExchangeMenuWidget::MakeTitleFont(const int32 Size, const int32 LetterSpacing) const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), Size);
	FontInfo.Size = Size;
	FontInfo.LetterSpacing = LetterSpacing;
	if (UObject* FontObject = ResolveStyleAsset(TitleFontAsset, ProjectSinfulExchangeMenuWidgetPrivate::TitleFontPath))
	{
		FontInfo.FontObject = FontObject;
		FontInfo.TypefaceFontName = NAME_None;
	}

	return FontInfo;
}

FSlateFontInfo UProjectSinfulAscensionExchangeMenuWidget::MakeBodyFont(const int32 Size, const int32 LetterSpacing) const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Size);
	FontInfo.Size = Size;
	FontInfo.LetterSpacing = LetterSpacing;
	if (UObject* FontObject = ResolveStyleAsset(BodyFontAsset, ProjectSinfulExchangeMenuWidgetPrivate::BodyFontPath))
	{
		FontInfo.FontObject = FontObject;
		FontInfo.TypefaceFontName = NAME_None;
	}

	return FontInfo;
}

FText UProjectSinfulAscensionExchangeMenuWidget::BuildAttributeFlavor(const EProjectSinAttribute Attribute) const
{
	switch (Attribute)
	{
	case EProjectSinAttribute::Willpower:
		return LOCTEXT("WillpowerFlavor", "Endurance for long expeditions: expands survival limits, grants a second breath between floors, and steadies the mind against fear and confusion.");
	case EProjectSinAttribute::Sadism:
		return LOCTEXT("SadismFlavor", "Turns non-spell aggression into lust pressure, then rewards clean executions on weakened male targets.");
	case EProjectSinAttribute::Masochism:
		return LOCTEXT("MasochismFlavor", "Converts endured pain into flat damage denial, rapture protection, Pain Overflow, and Down Arrow surrender or recovery control.");
	case EProjectSinAttribute::Faith:
		return LOCTEXT("FaithFlavor", "Devotion that sharpens spellcraft, wards against magic, calms Madness over time, and lets sleep cleanse the mind.");
	case EProjectSinAttribute::Cunning:
		return LOCTEXT("CunningFlavor", "Slows lockpick and struggle timing, widens lockpick openings, increases mistake tolerance, and preserves gear at mastery.");
	case EProjectSinAttribute::Celerity:
		return FText::GetEmpty();
	case EProjectSinAttribute::Allure:
		return LOCTEXT("AllureFlavor", "Makes sinful encounters more profitable, pulling extra value out of corrupted moments.");
	case EProjectSinAttribute::Count:
	default:
		break;
	}

	return FText::GetEmpty();
}

FText UProjectSinfulAscensionExchangeMenuWidget::BuildMilestoneStatusText(const FText& LabelText, const bool bUnlocked) const
{
	if (LabelText.IsEmpty())
	{
		return FText::GetEmpty();
	}

	return FText::Format(
		LOCTEXT("MilestoneStatusFormat", "{0} {1}"),
		LabelText,
		bUnlocked ? LOCTEXT("MilestoneUnlocked", "unlocked") : LOCTEXT("MilestoneLocked", "locked"));
}

FProjectSinfulAscensionExchangeDetailDisplayData UProjectSinfulAscensionExchangeMenuWidget::BuildDetailDisplayData(
	const FProjectSinfulAscensionExchangeResolvedEntry* SelectedEntry) const
{
	FProjectSinfulAscensionExchangeDetailDisplayData DetailData;
	if (!SelectedEntry)
	{
		DetailData.DetailName = LOCTEXT("UnavailableTitle", "Sinful Ascension unavailable");
		DetailData.LevelText = LOCTEXT("UnavailableLevel", "No player SXP component was found for this interaction.");
		DetailData.CostText = FText::GetEmpty();
		DetailData.MilestoneFiveText = FText::GetEmpty();
		DetailData.MilestoneTenText = FText::GetEmpty();
		DetailData.FlavorText = LOCTEXT("UnavailableFlavor", "Approach the altar again once the player runtime has initialized.");
		DetailData.bHasSelection = false;
		return DetailData;
	}

	DetailData.AttributeName = SelectedEntry->AttributeName;
	DetailData.DetailName = SelectedEntry->DetailName;
	DetailData.LevelText = FText::Format(
		LOCTEXT("SelectedLevelText", "Current Level {0}"),
		FText::AsNumber(SelectedEntry->RowData.Level));
	DetailData.CostText = SelectedEntry->RowData.Cost > 0
		? FText::Format(LOCTEXT("SelectedCostText", "Next ascent costs {0} SXP"), FText::AsNumber(SelectedEntry->RowData.Cost))
		: LOCTEXT("AlreadyMaxedText", "Already maxed");
	DetailData.MilestoneFiveText = BuildMilestoneStatusText(SelectedEntry->MilestoneFiveLabel, SelectedEntry->bMilestoneFiveUnlocked);
	DetailData.MilestoneTenText = BuildMilestoneStatusText(SelectedEntry->MilestoneTenLabel, SelectedEntry->bMilestoneTenUnlocked);
	DetailData.FlavorText = SelectedEntry->DetailDescription;
	DetailData.WatermarkTexture = SelectedEntry->DetailWatermarkTexture;
	DetailData.AccentTint = SelectedEntry->RowData.AccentTint.A > KINDA_SMALL_NUMBER
		? SelectedEntry->RowData.AccentTint.GetClamped(0.0f, 1.0f)
		: ProjectSinfulExchangeMenuWidgetPrivate::AccentTint;
	DetailData.bHasSelection = true;
	return DetailData;
}

const FProjectSinfulAscensionExchangeResolvedEntry* UProjectSinfulAscensionExchangeMenuWidget::GetSelectedEntry() const
{
	return ResolvedEntries.IsValidIndex(SelectedIndex) ? &ResolvedEntries[SelectedIndex] : nullptr;
}

UProjectSinfulAscensionExchangeDetailPanelWidget::UProjectSinfulAscensionExchangeDetailPanelWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TitleFontAsset = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::TitleFontPath);
	BodyFontAsset = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::BodyFontPath);
	DividerTexture = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::DividerTexturePath);
	DefaultWatermarkTexture = FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::DefaultWatermarkTexturePath);

	CurrentDisplayData.AttributeName = FName(TEXT("Willpower"));
	CurrentDisplayData.DetailName = LOCTEXT("DetailPreviewName", "Willpower");
	CurrentDisplayData.LevelText = LOCTEXT("DetailPreviewLevel", "Current Level 1");
	CurrentDisplayData.CostText = LOCTEXT("DetailPreviewCost", "Next ascent costs 204 SXP");
	CurrentDisplayData.MilestoneFiveText = LOCTEXT("DetailPreviewMilestoneFive", "Second Breath locked");
	CurrentDisplayData.MilestoneTenText = LOCTEXT("DetailPreviewMilestoneTen", "Unbroken Mind locked");
	CurrentDisplayData.FlavorText = LOCTEXT("DetailPreviewFlavor", "Endurance for long expeditions: expands survival limits, grants a second breath between floors, and steadies the mind against fear and confusion.");
	CurrentDisplayData.WatermarkTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(ProjectSinfulExchangeMenuWidgetPrivate::DefaultWatermarkTexturePath));
	CurrentDisplayData.AccentTint = ProjectSinfulExchangeMenuWidgetPrivate::AccentTint;
	CurrentDisplayData.bHasSelection = true;
}

void UProjectSinfulAscensionExchangeDetailPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshVisuals();
}

void UProjectSinfulAscensionExchangeDetailPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshVisuals();
}

bool UProjectSinfulAscensionExchangeDetailPanelWidget::BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree)
	{
		return false;
	}

	WidgetTree = TargetWidgetTree;
	bUsingNativeFallbackTree = true;
	bVisualTreeInitialized = false;
	const bool bBuiltTree = BuildDefaultWidgetTree(TargetWidgetTree);
	if (bBuiltTree)
	{
		InitializeVisualTree();
		RefreshVisuals();
	}
	return bBuiltTree;
}

void UProjectSinfulAscensionExchangeDetailPanelWidget::ApplyDetailData(
	const FProjectSinfulAscensionExchangeDetailDisplayData& InDisplayData)
{
	CurrentDisplayData = InDisplayData;
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshVisuals();
}

void UProjectSinfulAscensionExchangeDetailPanelWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (DetailBorder && WidgetTree->RootWidget == DetailBorder)
	{
		return;
	}

	if (WidgetTree->RootWidget)
	{
		bUsingNativeFallbackTree = false;
		return;
	}

	bUsingNativeFallbackTree = true;
	BuildDefaultWidgetTree(WidgetTree);
}

bool UProjectSinfulAscensionExchangeDetailPanelWidget::BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree || TargetWidgetTree->RootWidget)
	{
		return false;
	}

	DetailBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DetailBorder"));
	DetailBorder->SetPadding(FMargin(0.0f));
	TargetWidgetTree->RootWidget = DetailBorder;

	DetailOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("DetailOverlay"));
	DetailBorder->SetContent(DetailOverlay);

	DetailWatermarkImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("DetailWatermarkImage"));
	if (UOverlaySlot* WatermarkSlot = DetailOverlay->AddChildToOverlay(DetailWatermarkImage))
	{
		WatermarkSlot->SetHorizontalAlignment(HAlign_Right);
		WatermarkSlot->SetVerticalAlignment(VAlign_Center);
		WatermarkSlot->SetPadding(FMargin(0.0f, 28.0f, 42.0f, 34.0f));
	}

	DetailContentBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DetailContentBorder"));
	DetailContentBorder->SetPadding(FMargin(32.0f, 28.0f, 32.0f, 26.0f));
	DetailContentBorder->SetBrushColor(FLinearColor::Transparent);
	if (UOverlaySlot* DetailContentSlot = DetailOverlay->AddChildToOverlay(DetailContentBorder))
	{
		DetailContentSlot->SetHorizontalAlignment(HAlign_Fill);
		DetailContentSlot->SetVerticalAlignment(VAlign_Fill);
	}

	DetailContentBox = TargetWidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DetailContentBox"));
	DetailContentBorder->SetContent(DetailContentBox);

	DetailNameText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailNameText"));
	if (UVerticalBoxSlot* DetailNameSlot = DetailContentBox->AddChildToVerticalBox(DetailNameText))
	{
		DetailNameSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	}

	DetailLevelText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailLevelText"));
	if (UVerticalBoxSlot* DetailLevelSlot = DetailContentBox->AddChildToVerticalBox(DetailLevelText))
	{
		DetailLevelSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	}

	DetailCostText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailCostText"));
	if (UVerticalBoxSlot* DetailCostSlot = DetailContentBox->AddChildToVerticalBox(DetailCostText))
	{
		DetailCostSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}

	USizeBox* DetailDividerSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DetailDividerSizeBox"));
	DetailDividerSizeBox->SetWidthOverride(340.0f);
	DetailDividerSizeBox->SetHeightOverride(10.0f);
	if (UVerticalBoxSlot* DetailDividerSlot = DetailContentBox->AddChildToVerticalBox(DetailDividerSizeBox))
	{
		DetailDividerSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 14.0f));
	}

	DetailMilestoneDividerImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("DetailMilestoneDividerImage"));
	DetailDividerSizeBox->AddChild(DetailMilestoneDividerImage);

	DetailMilestoneFiveText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailMilestoneFiveText"));
	if (UVerticalBoxSlot* MilestoneFiveSlot = DetailContentBox->AddChildToVerticalBox(DetailMilestoneFiveText))
	{
		MilestoneFiveSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	}

	DetailMilestoneTenText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailMilestoneTenText"));
	if (UVerticalBoxSlot* MilestoneTenSlot = DetailContentBox->AddChildToVerticalBox(DetailMilestoneTenText))
	{
		MilestoneTenSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}

	DetailFlavorText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailFlavorText"));
	DetailFlavorText->SetAutoWrapText(true);
	if (UVerticalBoxSlot* DetailFlavorSlot = DetailContentBox->AddChildToVerticalBox(DetailFlavorText))
	{
		DetailFlavorSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	return true;
}

void UProjectSinfulAscensionExchangeDetailPanelWidget::InitializeVisualTree()
{
	if (bVisualTreeInitialized)
	{
		return;
	}

	if (!bUsingNativeFallbackTree)
	{
		bVisualTreeInitialized = true;
		return;
	}

	if (DetailBorder)
	{
		const FSlateRoundedBoxBrush SectionBrush(
			ProjectSinfulExchangeMenuWidgetPrivate::SectionFillTint,
			12.0f,
			FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SectionOutlineTint),
			1.0f,
			FVector2f(600.0f, 400.0f));
		DetailBorder->SetBrush(SectionBrush);
	}

	if (DetailNameText)
	{
		DetailNameText->SetFont(MakeTitleFont(50, 0));
		DetailNameText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::PrimaryTextTint));
		DetailNameText->SetShadowOffset(FVector2D(0.0f, 0.65f));
		DetailNameText->SetShadowColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::ShadowTint);
	}

	if (DetailLevelText)
	{
		DetailLevelText->SetFont(MakeBodyFont(28, 0));
		DetailLevelText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::PrimaryTextTint));
	}

	if (DetailCostText)
	{
		DetailCostText->SetFont(MakeBodyFont(31, 0));
		DetailCostText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::AccentTint));
	}

	if (DetailMilestoneDividerImage)
	{
		DetailMilestoneDividerImage->SetBrushFromTexture(
			ResolveTexture(DividerTexture, ProjectSinfulExchangeMenuWidgetPrivate::DividerTexturePath),
			false);
		DetailMilestoneDividerImage->SetColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::AccentTint.CopyWithNewOpacity(0.62f));
	}

	if (DetailMilestoneFiveText)
	{
		DetailMilestoneFiveText->SetFont(MakeBodyFont(23, 0));
		DetailMilestoneFiveText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SecondaryTextTint));
	}

	if (DetailMilestoneTenText)
	{
		DetailMilestoneTenText->SetFont(MakeBodyFont(23, 0));
		DetailMilestoneTenText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SecondaryTextTint));
	}

	if (DetailFlavorText)
	{
		DetailFlavorText->SetFont(MakeBodyFont(22, 0));
		DetailFlavorText->SetColorAndOpacity(FSlateColor(ProjectSinfulExchangeMenuWidgetPrivate::SecondaryTextTint));
		DetailFlavorText->SetShadowOffset(FVector2D(0.0f, 0.35f));
		DetailFlavorText->SetShadowColorAndOpacity(ProjectSinfulExchangeMenuWidgetPrivate::ShadowTint.CopyWithNewOpacity(0.18f));
	}

	bVisualTreeInitialized = true;
}

void UProjectSinfulAscensionExchangeDetailPanelWidget::RefreshVisuals()
{
	if (DetailNameText)
	{
		DetailNameText->SetText(CurrentDisplayData.DetailName);
	}

	if (DetailLevelText)
	{
		DetailLevelText->SetText(CurrentDisplayData.LevelText);
	}

	if (DetailCostText)
	{
		DetailCostText->SetText(CurrentDisplayData.CostText);
		if (bUsingNativeFallbackTree)
		{
			DetailCostText->SetColorAndOpacity(FSlateColor(CurrentDisplayData.AccentTint.CopyWithNewOpacity(0.98f)));
		}
	}

	if (DetailMilestoneFiveText)
	{
		DetailMilestoneFiveText->SetText(CurrentDisplayData.MilestoneFiveText);
	}

	if (DetailMilestoneTenText)
	{
		DetailMilestoneTenText->SetText(CurrentDisplayData.MilestoneTenText);
	}

	if (DetailFlavorText)
	{
		DetailFlavorText->SetText(CurrentDisplayData.FlavorText);
	}

	if (DetailWatermarkImage)
	{
		if (CurrentDisplayData.bHasSelection)
		{
			const TSoftObjectPtr<UTexture2D> WatermarkTexture = CurrentDisplayData.WatermarkTexture.IsNull()
				? DefaultWatermarkTexture
				: CurrentDisplayData.WatermarkTexture;
			DetailWatermarkImage->SetBrushFromTexture(
				ResolveTexture(WatermarkTexture, ProjectSinfulExchangeMenuWidgetPrivate::DefaultWatermarkTexturePath),
				false);
			if (bUsingNativeFallbackTree)
			{
				DetailWatermarkImage->SetColorAndOpacity(CurrentDisplayData.AccentTint.CopyWithNewOpacity(0.08f));
				DetailWatermarkImage->SetDesiredSizeOverride(FVector2D(340.0f, 340.0f));
			}
			DetailWatermarkImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			DetailWatermarkImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	OnExchangeDetailPanelDataApplied(CurrentDisplayData);
}

UTexture2D* UProjectSinfulAscensionExchangeDetailPanelWidget::ResolveTexture(
	const TSoftObjectPtr<UTexture2D>& AssetPtr,
	const TCHAR* FallbackPath) const
{
	if (!AssetPtr.IsNull())
	{
		if (UTexture2D* LoadedTexture = AssetPtr.LoadSynchronous())
		{
			return LoadedTexture;
		}
	}

	return Cast<UTexture2D>(ProjectSinfulExchangeMenuWidgetPrivate::LoadObjectByPath(FallbackPath));
}

UObject* UProjectSinfulAscensionExchangeDetailPanelWidget::ResolveStyleAsset(
	const TSoftObjectPtr<UObject>& AssetPtr,
	const TCHAR* FallbackPath) const
{
	if (!AssetPtr.IsNull())
	{
		if (UObject* LoadedObject = AssetPtr.LoadSynchronous())
		{
			return LoadedObject;
		}
	}

	return ProjectSinfulExchangeMenuWidgetPrivate::LoadObjectByPath(FallbackPath);
}

FSlateFontInfo UProjectSinfulAscensionExchangeDetailPanelWidget::MakeTitleFont(const int32 Size, const int32 LetterSpacing) const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), Size);
	FontInfo.Size = Size;
	FontInfo.LetterSpacing = LetterSpacing;
	if (UObject* FontObject = ResolveStyleAsset(TitleFontAsset, ProjectSinfulExchangeMenuWidgetPrivate::TitleFontPath))
	{
		FontInfo.FontObject = FontObject;
		FontInfo.TypefaceFontName = NAME_None;
	}

	return FontInfo;
}

FSlateFontInfo UProjectSinfulAscensionExchangeDetailPanelWidget::MakeBodyFont(const int32 Size, const int32 LetterSpacing) const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Size);
	FontInfo.Size = Size;
	FontInfo.LetterSpacing = LetterSpacing;
	if (UObject* FontObject = ResolveStyleAsset(BodyFontAsset, ProjectSinfulExchangeMenuWidgetPrivate::BodyFontPath))
	{
		FontInfo.FontObject = FontObject;
		FontInfo.TypefaceFontName = NAME_None;
	}

	return FontInfo;
}

#undef LOCTEXT_NAMESPACE
