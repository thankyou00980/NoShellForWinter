#include "SinfulAscension/ProjectSinfulAscensionExchangeAttributeRowWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"

namespace ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate
{
	constexpr float DefaultRowWidth = 560.0f;
	constexpr float DefaultRowHeight = 88.0f;
	constexpr float RowCornerRadius = 12.0f;
	constexpr float RowOutlineWidth = 1.2f;
	constexpr float IconSize = 84.0f;
	constexpr float RowsGlobalWidth = 560.0f;
	constexpr float RowsGlobalHeight = 690.0f;

	const FLinearColor DefaultAccentTint(0.96f, 0.53f, 0.80f, 1.0f);
	const FLinearColor UnselectedFillTint(0.030f, 0.018f, 0.048f, 0.92f);
	const FLinearColor SelectedFillTint(0.210f, 0.086f, 0.220f, 0.96f);
	const FLinearColor UnselectedOutlineTint(0.52f, 0.28f, 0.54f, 0.52f);
	const FLinearColor SelectedOutlineTint(0.95f, 0.57f, 0.86f, 0.96f);
	const FLinearColor IconPlateFillTint(0.080f, 0.030f, 0.104f, 0.95f);
	const FLinearColor NameTint(0.98f, 0.97f, 0.99f, 1.0f);
	const FLinearColor MetaTint(0.88f, 0.79f, 0.92f, 0.96f);
	const FLinearColor WarningTint(0.95f, 0.64f, 0.72f, 1.0f);
	const FLinearColor ShadowTint(0.0f, 0.0f, 0.0f, 0.32f);

	const TCHAR* TitleFontPath = TEXT("/Game/_Game/Widgets/Chronicle/Assets/Fonts/F_Chronicle_Bebas.F_Chronicle_Bebas");
	const TCHAR* BodyFontPath = TEXT("/Game/_Game/Widgets/Chronicle/Assets/Fonts/F_Chronicle_Cormorant.F_Chronicle_Cormorant");
	const TCHAR* RowFrameTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_RowFrame.T_SinfulAscension_Altar_RowFrame");
	const TCHAR* DefaultIconTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Default.T_SinfulAscension_Altar_Icon_Default");
	const TCHAR* SelectionGlyphTexturePath = TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Glyph.T_SinfulAscension_Altar_Glyph");

	UObject* LoadObjectByPath(const TCHAR* AssetPath)
	{
		return AssetPath && AssetPath[0] != 0
			? StaticLoadObject(UObject::StaticClass(), nullptr, AssetPath)
			: nullptr;
	}

	FProjectSinfulAscensionExchangeAttributeRowDisplayData MakePreviewData(
		const TCHAR* AttributeName,
		const TCHAR* DisplayLabel,
		const int32 Level,
		const int32 Cost,
		const FLinearColor& AccentTint,
		const TCHAR* IconPath,
		const bool bSelected = false)
	{
		FProjectSinfulAscensionExchangeAttributeRowDisplayData Data;
		Data.AttributeName = FName(AttributeName);
		Data.DisplayLabel = DisplayLabel;
		Data.IconTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(IconPath));
		Data.AccentTint = AccentTint;
		Data.Level = Level;
		Data.Cost = Cost;
		Data.RowWidth = DefaultRowWidth;
		Data.RowHeight = DefaultRowHeight;
		Data.bSelected = bSelected;
		Data.bAffordable = true;
		return Data;
	}

	const TCHAR* IconPathForAttribute(const TCHAR* AttributeName)
	{
		if (FCString::Strcmp(AttributeName, TEXT("Willpower")) == 0)
		{
			return TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Willpower.T_SinfulAscension_Altar_Icon_Willpower");
		}
		if (FCString::Strcmp(AttributeName, TEXT("Sadism")) == 0)
		{
			return TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Sadism.T_SinfulAscension_Altar_Icon_Sadism");
		}
		if (FCString::Strcmp(AttributeName, TEXT("Masochism")) == 0)
		{
			return TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Masochism.T_SinfulAscension_Altar_Icon_Masochism");
		}
		if (FCString::Strcmp(AttributeName, TEXT("Faith")) == 0)
		{
			return TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Faith.T_SinfulAscension_Altar_Icon_Faith");
		}
		if (FCString::Strcmp(AttributeName, TEXT("Cunning")) == 0)
		{
			return TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Cunning.T_SinfulAscension_Altar_Icon_Cunning");
		}
		if (FCString::Strcmp(AttributeName, TEXT("Celerity")) == 0)
		{
			return TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Celerity.T_SinfulAscension_Altar_Icon_Celerity");
		}
		if (FCString::Strcmp(AttributeName, TEXT("Allure")) == 0)
		{
			return TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/T_SinfulAscension_Altar_Icon_Allure.T_SinfulAscension_Altar_Icon_Allure");
		}
		return DefaultIconTexturePath;
	}

	UClass* NativeRowClassForAttribute(const FName AttributeName)
	{
		if (AttributeName == FName(TEXT("Willpower")))
		{
			return UProjectSinfulAscensionExchangeWillpowerRowWidget::StaticClass();
		}
		if (AttributeName == FName(TEXT("Sadism")))
		{
			return UProjectSinfulAscensionExchangeSadismRowWidget::StaticClass();
		}
		if (AttributeName == FName(TEXT("Masochism")))
		{
			return UProjectSinfulAscensionExchangeMasochismRowWidget::StaticClass();
		}
		if (AttributeName == FName(TEXT("Faith")))
		{
			return UProjectSinfulAscensionExchangeFaithRowWidget::StaticClass();
		}
		if (AttributeName == FName(TEXT("Cunning")))
		{
			return UProjectSinfulAscensionExchangeCunningRowWidget::StaticClass();
		}
		if (AttributeName == FName(TEXT("Celerity")))
		{
			return UProjectSinfulAscensionExchangeCelerityRowWidget::StaticClass();
		}
		if (AttributeName == FName(TEXT("Allure")))
		{
			return UProjectSinfulAscensionExchangeAllureRowWidget::StaticClass();
		}
		return UProjectSinfulAscensionExchangeAttributeRowWidget::StaticClass();
	}
}

UProjectSinfulAscensionExchangeAttributeRowWidget::UProjectSinfulAscensionExchangeAttributeRowWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TitleFontAsset = FSoftObjectPath(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::TitleFontPath);
	BodyFontAsset = FSoftObjectPath(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::BodyFontPath);
	RowFrameTexture = FSoftObjectPath(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::RowFrameTexturePath);
	DefaultIconTexture = FSoftObjectPath(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::DefaultIconTexturePath);
	SelectionGlyphTexture = FSoftObjectPath(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::SelectionGlyphTexturePath);
	CurrentDisplayData = MakeDesignerPreviewData();
}

void UProjectSinfulAscensionExchangeAttributeRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshVisuals();
}

void UProjectSinfulAscensionExchangeAttributeRowWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshVisuals();
}

bool UProjectSinfulAscensionExchangeAttributeRowWidget::BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree)
	{
		return false;
	}

	WidgetTree = TargetWidgetTree;
	CurrentDisplayData = MakeDesignerPreviewData();
	bVisualTreeInitialized = false;
	bUsingNativeFallbackTree = true;
	const bool bBuiltTree = BuildDefaultWidgetTree(TargetWidgetTree);
	if (bBuiltTree)
	{
		InitializeVisualTree();
		RefreshVisuals();
	}
	return bBuiltTree;
}

void UProjectSinfulAscensionExchangeAttributeRowWidget::ApplyDisplayData(const FProjectSinfulAscensionExchangeAttributeRowDisplayData& InDisplayData)
{
	CurrentDisplayData = InDisplayData;
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshVisuals();
}

void UProjectSinfulAscensionExchangeAttributeRowWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (RootSizeBox && WidgetTree->RootWidget == RootSizeBox)
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

bool UProjectSinfulAscensionExchangeAttributeRowWidget::BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree || TargetWidgetTree->RootWidget)
	{
		return false;
	}

	RootSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::DefaultRowWidth);
	RootSizeBox->SetHeightOverride(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::DefaultRowHeight);
	TargetWidgetTree->RootWidget = RootSizeBox;

	DesignerRootOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("DesignerRootOverlay"));
	RootSizeBox->AddChild(DesignerRootOverlay);

	RootOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
	if (UOverlaySlot* RootOverlaySlot = DesignerRootOverlay->AddChildToOverlay(RootOverlay))
	{
		RootOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
		RootOverlaySlot->SetVerticalAlignment(VAlign_Fill);
	}

	BackgroundBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackgroundBorder"));
	BackgroundBorder->SetPadding(FMargin(0.0f));
	BackgroundBorder->SetBrushColor(FLinearColor::White);
	if (UOverlaySlot* BackgroundSlot = RootOverlay->AddChildToOverlay(BackgroundBorder))
	{
		BackgroundSlot->SetHorizontalAlignment(HAlign_Fill);
		BackgroundSlot->SetVerticalAlignment(VAlign_Fill);
	}

	FrameImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("FrameImage"));
	if (UOverlaySlot* FrameSlot = RootOverlay->AddChildToOverlay(FrameImage))
	{
		FrameSlot->SetHorizontalAlignment(HAlign_Fill);
		FrameSlot->SetVerticalAlignment(VAlign_Fill);
	}

	ContentBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ContentBorder"));
	ContentBorder->SetPadding(FMargin(10.0f, 10.0f, 12.0f, 10.0f));
	ContentBorder->SetBrushColor(FLinearColor::Transparent);
	if (UOverlaySlot* ContentSlot = RootOverlay->AddChildToOverlay(ContentBorder))
	{
		ContentSlot->SetHorizontalAlignment(HAlign_Fill);
		ContentSlot->SetVerticalAlignment(VAlign_Fill);
	}

	ContentBox = TargetWidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ContentBox"));
	ContentBorder->SetContent(ContentBox);

	IconSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("IconSizeBox"));
	IconSizeBox->SetWidthOverride(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconSize);
	IconSizeBox->SetHeightOverride(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconSize);
	if (UHorizontalBoxSlot* IconSlot = ContentBox->AddChildToHorizontalBox(IconSizeBox))
	{
		IconSlot->SetHorizontalAlignment(HAlign_Left);
		IconSlot->SetVerticalAlignment(VAlign_Center);
	}

	IconBackgroundBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("IconBackgroundBorder"));
	IconBackgroundBorder->SetPadding(FMargin(16.0f));
	IconBackgroundBorder->SetBrushColor(FLinearColor::White);
	IconSizeBox->AddChild(IconBackgroundBorder);

	IconImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("IconImage"));
	IconBackgroundBorder->SetContent(IconImage);

	TextColumn = TargetWidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TextColumn"));
	if (UHorizontalBoxSlot* TextSlot = ContentBox->AddChildToHorizontalBox(TextColumn))
	{
		TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TextSlot->SetHorizontalAlignment(HAlign_Fill);
		TextSlot->SetVerticalAlignment(VAlign_Center);
		TextSlot->SetPadding(FMargin(16.0f, 0.0f, 10.0f, 0.0f));
	}

	NameText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NameText"));
	if (UVerticalBoxSlot* NameSlot = TextColumn->AddChildToVerticalBox(NameText))
	{
		NameSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f));
	}

	MetaRow = TargetWidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MetaRow"));
	TextColumn->AddChildToVerticalBox(MetaRow);

	LevelText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelText"));
	if (UHorizontalBoxSlot* LevelSlot = MetaRow->AddChildToHorizontalBox(LevelText))
	{
		LevelSlot->SetHorizontalAlignment(HAlign_Left);
		LevelSlot->SetVerticalAlignment(VAlign_Center);
	}

	SeparatorText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SeparatorText"));
	if (UHorizontalBoxSlot* SeparatorSlot = MetaRow->AddChildToHorizontalBox(SeparatorText))
	{
		SeparatorSlot->SetPadding(FMargin(8.0f, 0.0f, 8.0f, 0.0f));
		SeparatorSlot->SetHorizontalAlignment(HAlign_Left);
		SeparatorSlot->SetVerticalAlignment(VAlign_Center);
	}

	CostText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CostText"));
	MetaRow->AddChildToHorizontalBox(CostText);

	SelectionGlyphSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SelectionGlyphSizeBox"));
	SelectionGlyphSizeBox->SetWidthOverride(18.0f);
	SelectionGlyphSizeBox->SetHeightOverride(18.0f);
	if (UHorizontalBoxSlot* SelectionSlot = ContentBox->AddChildToHorizontalBox(SelectionGlyphSizeBox))
	{
		SelectionSlot->SetHorizontalAlignment(HAlign_Center);
		SelectionSlot->SetVerticalAlignment(VAlign_Center);
	}

	SelectionGlyphImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("SelectionGlyphImage"));
	SelectionGlyphSizeBox->AddChild(SelectionGlyphImage);

	return true;
}

void UProjectSinfulAscensionExchangeAttributeRowWidget::InitializeVisualTree()
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

	if (FrameImage)
	{
		FrameImage->SetBrushFromTexture(
			ResolveTexture(RowFrameTexture, ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::RowFrameTexturePath),
			false);
	}

	if (SelectionGlyphImage)
	{
		SelectionGlyphImage->SetBrushFromTexture(
			ResolveTexture(SelectionGlyphTexture, ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::SelectionGlyphTexturePath),
			false);
	}

	if (NameText)
	{
		NameText->SetFont(MakeTitleFont(24, 0));
		NameText->SetColorAndOpacity(FSlateColor(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::NameTint));
		NameText->SetShadowOffset(FVector2D(0.0f, 0.30f));
		NameText->SetShadowColorAndOpacity(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::ShadowTint);
	}

	if (LevelText)
	{
		LevelText->SetFont(MakeBodyFont(15, 0));
		LevelText->SetColorAndOpacity(FSlateColor(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MetaTint));
	}

	if (SeparatorText)
	{
		SeparatorText->SetFont(MakeBodyFont(15, 0));
		SeparatorText->SetText(FText::FromString(TEXT("|")));
		SeparatorText->SetColorAndOpacity(FSlateColor(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MetaTint.CopyWithNewOpacity(0.74f)));
	}

	if (CostText)
	{
		CostText->SetFont(MakeBodyFont(15, 0));
	}

	bVisualTreeInitialized = true;
}

void UProjectSinfulAscensionExchangeAttributeRowWidget::RefreshVisuals()
{
	const float RowWidth = FMath::Max(CurrentDisplayData.RowWidth, 320.0f);
	const float RowHeight = FMath::Max(CurrentDisplayData.RowHeight, 70.0f);
	const FLinearColor AccentTint = CurrentDisplayData.AccentTint.A > KINDA_SMALL_NUMBER
		? CurrentDisplayData.AccentTint.GetClamped(0.0f, 1.0f)
		: ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::DefaultAccentTint;
	const bool bSelected = CurrentDisplayData.bSelected;

	if (bUsingNativeFallbackTree)
	{
		if (RootSizeBox)
		{
			RootSizeBox->SetWidthOverride(RowWidth);
			RootSizeBox->SetHeightOverride(RowHeight);
		}

		if (BackgroundBorder)
		{
			const FLinearColor FillTint = bSelected
				? ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::SelectedFillTint
				: ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::UnselectedFillTint;
			const FLinearColor OutlineTint = bSelected
				? AccentTint.CopyWithNewOpacity(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::SelectedOutlineTint.A)
				: ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::UnselectedOutlineTint;
			const FSlateRoundedBoxBrush BackgroundBrush(
				FillTint,
				ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::RowCornerRadius,
				FSlateColor(OutlineTint),
				ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::RowOutlineWidth,
				FVector2f(RowWidth, RowHeight));
			BackgroundBorder->SetBrush(BackgroundBrush);
		}

		if (FrameImage)
		{
			FrameImage->SetColorAndOpacity(AccentTint.CopyWithNewOpacity(bSelected ? 0.82f : 0.40f));
		}

		if (IconSizeBox)
		{
			IconSizeBox->SetWidthOverride(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconSize);
			IconSizeBox->SetHeightOverride(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconSize);
		}

		if (IconBackgroundBorder)
		{
			const FSlateRoundedBoxBrush IconBrush(
				ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPlateFillTint,
				10.0f,
				FSlateColor(AccentTint.CopyWithNewOpacity(bSelected ? 0.74f : 0.32f)),
				1.0f,
				FVector2f(
					ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconSize,
					ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconSize));
			IconBackgroundBorder->SetBrush(IconBrush);
		}
	}

	if (IconImage)
	{
		IconImage->SetBrushFromTexture(
			ResolveTexture(
				CurrentDisplayData.IconTexture.IsNull() ? DefaultIconTexture : CurrentDisplayData.IconTexture,
				ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::DefaultIconTexturePath),
			false);
		if (bUsingNativeFallbackTree)
		{
			IconImage->SetColorAndOpacity(AccentTint.CopyWithNewOpacity(bSelected ? 0.96f : 0.72f));
		}
	}

	if (NameText)
	{
		NameText->SetText(FText::FromString(CurrentDisplayData.DisplayLabel));
		if (bUsingNativeFallbackTree)
		{
			NameText->SetColorAndOpacity(FSlateColor(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::NameTint));
		}
	}

	if (LevelText)
	{
		LevelText->SetText(FText::Format(
			FText::FromString(TEXT("Lv {0}")),
			FText::AsNumber(CurrentDisplayData.Level)));
		if (bUsingNativeFallbackTree)
		{
			LevelText->SetColorAndOpacity(FSlateColor(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MetaTint));
		}
	}

	if (SeparatorText && bUsingNativeFallbackTree)
	{
		SeparatorText->SetColorAndOpacity(FSlateColor(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MetaTint.CopyWithNewOpacity(0.72f)));
	}

	if (CostText)
	{
		CostText->SetText(FText::Format(
			FText::FromString(TEXT("Cost {0}")),
			FText::AsNumber(CurrentDisplayData.Cost)));
		if (bUsingNativeFallbackTree)
		{
			CostText->SetColorAndOpacity(FSlateColor(
				(CurrentDisplayData.bAffordable || bSelected)
					? AccentTint.CopyWithNewOpacity(0.98f)
					: ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::WarningTint));
		}
	}

	if (SelectionGlyphImage)
	{
		if (bUsingNativeFallbackTree)
		{
			SelectionGlyphImage->SetColorAndOpacity(AccentTint.CopyWithNewOpacity(bSelected ? 0.98f : 0.22f));
		}
		SelectionGlyphImage->SetVisibility(bSelected ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	OnExchangeAttributeRowDataApplied(CurrentDisplayData);
	OnExchangeAttributeRowVisualStateChanged(CurrentDisplayData.AttributeName, CurrentDisplayData.bSelected, CurrentDisplayData.bAffordable);
}

FProjectSinfulAscensionExchangeAttributeRowDisplayData UProjectSinfulAscensionExchangeAttributeRowWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(
		TEXT("Willpower"),
		TEXT("Willpower"),
		1,
		204,
		FLinearColor(0.97f, 0.67f, 0.85f, 1.0f),
		ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Willpower")),
		true);
}

UTexture2D* UProjectSinfulAscensionExchangeAttributeRowWidget::ResolveTexture(
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

	return Cast<UTexture2D>(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::LoadObjectByPath(FallbackPath));
}

UObject* UProjectSinfulAscensionExchangeAttributeRowWidget::ResolveStyleAsset(
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

	return ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::LoadObjectByPath(FallbackPath);
}

FSlateFontInfo UProjectSinfulAscensionExchangeAttributeRowWidget::MakeTitleFont(const int32 Size, const int32 LetterSpacing) const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), Size);
	FontInfo.Size = Size;
	FontInfo.LetterSpacing = LetterSpacing;
	if (UObject* FontObject = ResolveStyleAsset(TitleFontAsset, ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::TitleFontPath))
	{
		FontInfo.FontObject = FontObject;
		FontInfo.TypefaceFontName = NAME_None;
	}

	return FontInfo;
}

FSlateFontInfo UProjectSinfulAscensionExchangeAttributeRowWidget::MakeBodyFont(const int32 Size, const int32 LetterSpacing) const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Size);
	FontInfo.Size = Size;
	FontInfo.LetterSpacing = LetterSpacing;
	if (UObject* FontObject = ResolveStyleAsset(BodyFontAsset, ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::BodyFontPath))
	{
		FontInfo.FontObject = FontObject;
		FontInfo.TypefaceFontName = NAME_None;
	}

	return FontInfo;
}

FProjectSinfulAscensionExchangeAttributeRowDisplayData UProjectSinfulAscensionExchangeAttributeRowGlobalWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(
		TEXT("Global"),
		TEXT("Attribute"),
		2,
		350,
		FLinearColor(0.96f, 0.53f, 0.80f, 1.0f),
		ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::DefaultIconTexturePath,
		true);
}

FProjectSinfulAscensionExchangeAttributeRowDisplayData UProjectSinfulAscensionExchangeWillpowerRowWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Willpower"), TEXT("Willpower"), 1, 204, FLinearColor(0.97f, 0.67f, 0.85f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Willpower")), true);
}

FProjectSinfulAscensionExchangeAttributeRowDisplayData UProjectSinfulAscensionExchangeSadismRowWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Sadism"), TEXT("Sadism"), 0, 80, FLinearColor(0.95f, 0.54f, 0.78f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Sadism")));
}

FProjectSinfulAscensionExchangeAttributeRowDisplayData UProjectSinfulAscensionExchangeMasochismRowWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Masochism"), TEXT("Masochism"), 0, 80, FLinearColor(0.96f, 0.45f, 0.74f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Masochism")));
}

FProjectSinfulAscensionExchangeAttributeRowDisplayData UProjectSinfulAscensionExchangeFaithRowWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Faith"), TEXT("Faith"), 0, 80, FLinearColor(0.92f, 0.59f, 0.84f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Faith")));
}

FProjectSinfulAscensionExchangeAttributeRowDisplayData UProjectSinfulAscensionExchangeCunningRowWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Cunning"), TEXT("Cunning"), 0, 80, FLinearColor(0.98f, 0.57f, 0.82f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Cunning")));
}

FProjectSinfulAscensionExchangeAttributeRowDisplayData UProjectSinfulAscensionExchangeCelerityRowWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Celerity"), TEXT("Celerity"), 0, 80, FLinearColor(0.88f, 0.63f, 0.88f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Celerity")));
}

FProjectSinfulAscensionExchangeAttributeRowDisplayData UProjectSinfulAscensionExchangeAllureRowWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Allure"), TEXT("Allure"), 0, 80, FLinearColor(0.99f, 0.63f, 0.86f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Allure")));
}

UProjectSinfulAscensionExchangeRowsGlobalWidget::UProjectSinfulAscensionExchangeRowsGlobalWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UProjectSinfulAscensionExchangeRowsGlobalWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
}

void UProjectSinfulAscensionExchangeRowsGlobalWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetTree();
}

bool UProjectSinfulAscensionExchangeRowsGlobalWidget::BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree)
	{
		return false;
	}

	WidgetTree = TargetWidgetTree;
	bUsingNativeFallbackTree = true;
	const bool bBuiltTree = BuildDefaultWidgetTree(TargetWidgetTree);
	if (bBuiltTree)
	{
		TArray<FProjectSinfulAscensionExchangeAttributeRowDisplayData> PreviewRows;
		PreviewRows.Add(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Willpower"), TEXT("Willpower"), 1, 204, FLinearColor(0.97f, 0.67f, 0.85f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Willpower")), true));
		PreviewRows.Add(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Sadism"), TEXT("Sadism"), 0, 80, FLinearColor(0.95f, 0.54f, 0.78f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Sadism"))));
		PreviewRows.Add(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Masochism"), TEXT("Masochism"), 0, 80, FLinearColor(0.96f, 0.45f, 0.74f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Masochism"))));
		PreviewRows.Add(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Faith"), TEXT("Faith"), 0, 80, FLinearColor(0.92f, 0.59f, 0.84f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Faith"))));
		PreviewRows.Add(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Cunning"), TEXT("Cunning"), 0, 80, FLinearColor(0.98f, 0.57f, 0.82f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Cunning"))));
		PreviewRows.Add(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Celerity"), TEXT("Celerity"), 0, 80, FLinearColor(0.88f, 0.63f, 0.88f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Celerity"))));
		PreviewRows.Add(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::MakePreviewData(TEXT("Allure"), TEXT("Allure"), 0, 80, FLinearColor(0.99f, 0.63f, 0.86f, 1.0f), ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::IconPathForAttribute(TEXT("Allure"))));
		ApplyRows(PreviewRows, UProjectSinfulAscensionExchangeAttributeRowWidget::StaticClass());
	}
	return bBuiltTree;
}

void UProjectSinfulAscensionExchangeRowsGlobalWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (RootSizeBox && WidgetTree->RootWidget == RootSizeBox)
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

bool UProjectSinfulAscensionExchangeRowsGlobalWidget::BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree || TargetWidgetTree->RootWidget)
	{
		return false;
	}

	RootSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::RowsGlobalWidth);
	RootSizeBox->SetHeightOverride(ProjectSinfulAscensionExchangeAttributeRowWidgetPrivate::RowsGlobalHeight);
	TargetWidgetTree->RootWidget = RootSizeBox;

	DesignerRootOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("DesignerRootOverlay"));
	RootSizeBox->AddChild(DesignerRootOverlay);

	RootOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
	if (UOverlaySlot* RootOverlaySlot = DesignerRootOverlay->AddChildToOverlay(RootOverlay))
	{
		RootOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
		RootOverlaySlot->SetVerticalAlignment(VAlign_Fill);
	}

	RowsScrollBox = TargetWidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("RowsScrollBox"));
	RowsScrollBox->SetAnimateWheelScrolling(false);
	RowsScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
	if (UOverlaySlot* ScrollSlot = RootOverlay->AddChildToOverlay(RowsScrollBox))
	{
		ScrollSlot->SetHorizontalAlignment(HAlign_Fill);
		ScrollSlot->SetVerticalAlignment(VAlign_Fill);
	}

	RowsLayout = TargetWidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RowsLayout"));
	RowsScrollBox->AddChild(RowsLayout);

	const TArray<TPair<FName, UClass*>> FixedRows = {
		{ FName(TEXT("Willpower")), UProjectSinfulAscensionExchangeWillpowerRowWidget::StaticClass() },
		{ FName(TEXT("Sadism")), UProjectSinfulAscensionExchangeSadismRowWidget::StaticClass() },
		{ FName(TEXT("Masochism")), UProjectSinfulAscensionExchangeMasochismRowWidget::StaticClass() },
		{ FName(TEXT("Faith")), UProjectSinfulAscensionExchangeFaithRowWidget::StaticClass() },
		{ FName(TEXT("Cunning")), UProjectSinfulAscensionExchangeCunningRowWidget::StaticClass() },
		{ FName(TEXT("Celerity")), UProjectSinfulAscensionExchangeCelerityRowWidget::StaticClass() },
		{ FName(TEXT("Allure")), UProjectSinfulAscensionExchangeAllureRowWidget::StaticClass() },
	};

	for (int32 Index = 0; Index < FixedRows.Num(); ++Index)
	{
		const TPair<FName, UClass*>& FixedRow = FixedRows[Index];
		UProjectSinfulAscensionExchangeAttributeRowWidget* RowWidget =
			TargetWidgetTree->ConstructWidget<UProjectSinfulAscensionExchangeAttributeRowWidget>(
				FixedRow.Value,
				*FString::Printf(TEXT("%sRow"), *FixedRow.Key.ToString()));
		if (!RowWidget)
		{
			continue;
		}

		if (UVerticalBoxSlot* RowSlot = RowsLayout->AddChildToVerticalBox(RowWidget))
		{
			RowSlot->SetPadding(FMargin(0.0f, Index == 0 ? 0.0f : 10.0f, 0.0f, 0.0f));
		}

		if (FixedRow.Key == FName(TEXT("Willpower")))
		{
			WillpowerRow = RowWidget;
		}
		else if (FixedRow.Key == FName(TEXT("Sadism")))
		{
			SadismRow = RowWidget;
		}
		else if (FixedRow.Key == FName(TEXT("Masochism")))
		{
			MasochismRow = RowWidget;
		}
		else if (FixedRow.Key == FName(TEXT("Faith")))
		{
			FaithRow = RowWidget;
		}
		else if (FixedRow.Key == FName(TEXT("Cunning")))
		{
			CunningRow = RowWidget;
		}
		else if (FixedRow.Key == FName(TEXT("Celerity")))
		{
			CelerityRow = RowWidget;
		}
		else if (FixedRow.Key == FName(TEXT("Allure")))
		{
			AllureRow = RowWidget;
		}
	}

	ExtraRowsLayout = TargetWidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ExtraRowsLayout"));
	if (UVerticalBoxSlot* ExtraSlot = RowsLayout->AddChildToVerticalBox(ExtraRowsLayout))
	{
		ExtraSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	}

	return true;
}

int32 UProjectSinfulAscensionExchangeRowsGlobalWidget::ApplyRows(
	const TArray<FProjectSinfulAscensionExchangeAttributeRowDisplayData>& InRowData,
	TSubclassOf<UProjectSinfulAscensionExchangeAttributeRowWidget> FallbackRowWidgetClass)
{
	BuildWidgetTree();
	RuntimeRowsByName.Empty();
	ClearFallbackRows();

	TSet<FName> VisibleFixedRowNames;
	int32 SelectedIndex = INDEX_NONE;
	VisibleRowCount = 0;

	for (int32 Index = 0; Index < InRowData.Num(); ++Index)
	{
		const FProjectSinfulAscensionExchangeAttributeRowDisplayData& RowData = InRowData[Index];
		UProjectSinfulAscensionExchangeAttributeRowWidget* RowWidget = GetFixedRowForAttribute(RowData.AttributeName);
		if (RowWidget)
		{
			VisibleFixedRowNames.Add(RowData.AttributeName);
		}
		else
		{
			RowWidget = GetOrCreateFallbackRow(RowData.AttributeName, FallbackRowWidgetClass);
		}

		if (!RowWidget)
		{
			continue;
		}

		RowWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		RowWidget->ApplyDisplayData(RowData);
		RuntimeRowsByName.Add(RowData.AttributeName, RowWidget);
		++VisibleRowCount;

		if (RowData.bSelected)
		{
			SelectedIndex = Index;
		}
	}

	HideUnusedFixedRows(VisibleFixedRowNames);
	OnExchangeRowsApplied(VisibleRowCount, SelectedIndex);
	return VisibleRowCount;
}

UProjectSinfulAscensionExchangeAttributeRowWidget* UProjectSinfulAscensionExchangeRowsGlobalWidget::FindRowWidgetByAttribute(
	const FName AttributeName) const
{
	if (const TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget>* FoundWidget = RuntimeRowsByName.Find(AttributeName))
	{
		return FoundWidget->Get();
	}
	return GetFixedRowForAttribute(AttributeName);
}

void UProjectSinfulAscensionExchangeRowsGlobalWidget::ScrollRowWidgetIntoView(UProjectSinfulAscensionExchangeAttributeRowWidget* RowWidget)
{
	if (RowsScrollBox && RowWidget)
	{
		RowsScrollBox->ScrollWidgetIntoView(RowWidget, true, EDescendantScrollDestination::Center, 24.0f);
	}
}

UProjectSinfulAscensionExchangeAttributeRowWidget* UProjectSinfulAscensionExchangeRowsGlobalWidget::GetFixedRowForAttribute(
	const FName AttributeName) const
{
	if (AttributeName == FName(TEXT("Willpower")))
	{
		return WillpowerRow.Get();
	}
	if (AttributeName == FName(TEXT("Sadism")))
	{
		return SadismRow.Get();
	}
	if (AttributeName == FName(TEXT("Masochism")))
	{
		return MasochismRow.Get();
	}
	if (AttributeName == FName(TEXT("Faith")))
	{
		return FaithRow.Get();
	}
	if (AttributeName == FName(TEXT("Cunning")))
	{
		return CunningRow.Get();
	}
	if (AttributeName == FName(TEXT("Celerity")))
	{
		return CelerityRow.Get();
	}
	if (AttributeName == FName(TEXT("Allure")))
	{
		return AllureRow.Get();
	}
	return nullptr;
}

UProjectSinfulAscensionExchangeAttributeRowWidget* UProjectSinfulAscensionExchangeRowsGlobalWidget::GetOrCreateFallbackRow(
	const FName AttributeName,
	TSubclassOf<UProjectSinfulAscensionExchangeAttributeRowWidget> FallbackRowWidgetClass)
{
	if (!ExtraRowsLayout || !FallbackRowWidgetClass)
	{
		return nullptr;
	}

	if (TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget>* ExistingRow = FallbackRowsByName.Find(AttributeName))
	{
		return ExistingRow->Get();
	}

	UProjectSinfulAscensionExchangeAttributeRowWidget* FallbackRow =
		CreateWidget<UProjectSinfulAscensionExchangeAttributeRowWidget>(this, FallbackRowWidgetClass.Get());
	if (!FallbackRow)
	{
		return nullptr;
	}

	if (UVerticalBoxSlot* RowSlot = ExtraRowsLayout->AddChildToVerticalBox(FallbackRow))
	{
		RowSlot->SetPadding(FMargin(0.0f, FallbackRowsByName.IsEmpty() ? 0.0f : 10.0f, 0.0f, 0.0f));
	}

	FallbackRowsByName.Add(AttributeName, FallbackRow);
	return FallbackRow;
}

void UProjectSinfulAscensionExchangeRowsGlobalWidget::HideUnusedFixedRows(const TSet<FName>& VisibleFixedRowNames)
{
	const auto HideIfUnused = [&VisibleFixedRowNames](const FName AttributeName, UProjectSinfulAscensionExchangeAttributeRowWidget* RowWidget)
	{
		if (RowWidget && !VisibleFixedRowNames.Contains(AttributeName))
		{
			RowWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	};

	HideIfUnused(FName(TEXT("Willpower")), WillpowerRow.Get());
	HideIfUnused(FName(TEXT("Sadism")), SadismRow.Get());
	HideIfUnused(FName(TEXT("Masochism")), MasochismRow.Get());
	HideIfUnused(FName(TEXT("Faith")), FaithRow.Get());
	HideIfUnused(FName(TEXT("Cunning")), CunningRow.Get());
	HideIfUnused(FName(TEXT("Celerity")), CelerityRow.Get());
	HideIfUnused(FName(TEXT("Allure")), AllureRow.Get());
}

void UProjectSinfulAscensionExchangeRowsGlobalWidget::ClearFallbackRows()
{
	if (ExtraRowsLayout)
	{
		ExtraRowsLayout->ClearChildren();
	}
	FallbackRowsByName.Empty();
}
