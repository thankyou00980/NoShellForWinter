#include "SinfulAscension/ProjectSinfulAscensionAttributeCardWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScaleBox.h"
#include "Components/ScaleBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"

namespace ProjectSinfulAscensionAttributeCardWidgetPrivate
{
	constexpr float CardWidth = 56.0f;
	constexpr float CardHeight = 70.0f;
	constexpr float CardCornerRadius = 9.0f;
	constexpr float CardOutlineWidth = 1.0f;
	constexpr float ValueHeight = 18.0f;
	constexpr float CardsGlobalWidth = 416.0f;
	constexpr float CardsGlobalHeight = 70.0f;

	const FLinearColor BaseFillTint(0.012f, 0.0f, 0.011f, 0.88f);
	const FLinearColor BaseOutlineTint(0.84f, 0.37f, 0.65f, 0.36f);
	const FLinearColor LabelShadowTint(0.0f, 0.0f, 0.0f, 0.24f);
	const FLinearColor ValueShadowTint(0.0f, 0.0f, 0.0f, 0.30f);

	const TCHAR* CinzelFontPath = TEXT("/Game/_Game/Widgets/Attributes/Assets/Fonts/F_InnerState_Cinzel.F_InnerState_Cinzel");
	const TCHAR* CormorantFontPath = TEXT("/Game/_Game/Widgets/Attributes/Assets/Fonts/F_InnerState_Cormorant.F_InnerState_Cormorant");
	const TCHAR* CardFrameTexturePath = TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_CardFrame.T_SinfulAscension_CardFrame");
	const TCHAR* DefaultIconTexturePath = TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Default.T_SinfulAscension_Icon_Default");

	UObject* LoadObjectByPath(const TCHAR* AssetPath)
	{
		return AssetPath && AssetPath[0] != 0
			? StaticLoadObject(UObject::StaticClass(), nullptr, AssetPath)
			: nullptr;
	}

	FProjectSinfulAscensionAttributeCardDisplayData MakePreviewData(
		const FName AttributeName,
		const FString& DisplayLabel,
		const FString& ShortLabel,
		const int32 Level,
		const FLinearColor& AccentTint,
		const TCHAR* IconTexturePath)
	{
		FProjectSinfulAscensionAttributeCardDisplayData Data;
		Data.AttributeName = AttributeName;
		Data.DisplayLabel = DisplayLabel;
		Data.ShortLabel = ShortLabel;
		Data.Level = Level;
		Data.AccentTint = AccentTint;
		if (IconTexturePath && IconTexturePath[0] != 0)
		{
			Data.IconTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(IconTexturePath));
		}
		return Data;
	}
}

UProjectSinfulAscensionAttributeCardWidget::UProjectSinfulAscensionAttributeCardWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TitleFontAsset = FSoftObjectPath(ProjectSinfulAscensionAttributeCardWidgetPrivate::CinzelFontPath);
	BodyFontAsset = FSoftObjectPath(ProjectSinfulAscensionAttributeCardWidgetPrivate::CormorantFontPath);
	CardFrameTexture = FSoftObjectPath(ProjectSinfulAscensionAttributeCardWidgetPrivate::CardFrameTexturePath);
	DefaultIconTexture = FSoftObjectPath(ProjectSinfulAscensionAttributeCardWidgetPrivate::DefaultIconTexturePath);
}

void UProjectSinfulAscensionAttributeCardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshVisuals();
}

void UProjectSinfulAscensionAttributeCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshVisuals();
}

bool UProjectSinfulAscensionAttributeCardWidget::BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree)
	{
		return false;
	}

	CurrentDisplayData = MakeDesignerPreviewData();
	bVisualTreeInitialized = false;
	bUsingNativeFallbackTree = true;
	WidgetTree = TargetWidgetTree;

	const bool bBuiltTree = BuildDefaultWidgetTree(TargetWidgetTree);
	if (bBuiltTree)
	{
		InitializeVisualTree();
		RefreshVisuals();
	}

	return bBuiltTree;
}

void UProjectSinfulAscensionAttributeCardWidget::ApplyDisplayData(const FProjectSinfulAscensionAttributeCardDisplayData& InDisplayData)
{
	CurrentDisplayData = InDisplayData;
	BuildWidgetTree();
	InitializeVisualTree();
	RefreshVisuals();
	OnSinfulAttributeCardDataApplied(CurrentDisplayData);
}

void UProjectSinfulAscensionAttributeCardWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (RootSizeBox || WidgetTree->RootWidget)
	{
		bUsingNativeFallbackTree = false;
		return;
	}

	bUsingNativeFallbackTree = true;
	BuildDefaultWidgetTree(WidgetTree);
}

bool UProjectSinfulAscensionAttributeCardWidget::BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree || TargetWidgetTree->RootWidget)
	{
		return false;
	}

	UOverlay* DesignerRootOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("DesignerRootOverlay"));
	TargetWidgetTree->RootWidget = DesignerRootOverlay;

	RootSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(ProjectSinfulAscensionAttributeCardWidgetPrivate::CardWidth);
	RootSizeBox->SetHeightOverride(ProjectSinfulAscensionAttributeCardWidgetPrivate::CardHeight);
	if (UOverlaySlot* RootSizeSlot = DesignerRootOverlay->AddChildToOverlay(RootSizeBox))
	{
		RootSizeSlot->SetHorizontalAlignment(HAlign_Center);
		RootSizeSlot->SetVerticalAlignment(VAlign_Center);
	}

	RootOverlay = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
	RootSizeBox->AddChild(RootOverlay);

	BaseBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BaseBorder"));
	BaseBorder->SetPadding(FMargin(0.0f));
	BaseBorder->SetBrushColor(FLinearColor::White);
	if (UOverlaySlot* BaseSlot = RootOverlay->AddChildToOverlay(BaseBorder))
	{
		BaseSlot->SetHorizontalAlignment(HAlign_Fill);
		BaseSlot->SetVerticalAlignment(VAlign_Fill);
	}

	FrameImage = TargetWidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("FrameImage"));
	if (UOverlaySlot* FrameSlot = RootOverlay->AddChildToOverlay(FrameImage))
	{
		FrameSlot->SetHorizontalAlignment(HAlign_Fill);
		FrameSlot->SetVerticalAlignment(VAlign_Fill);
	}

	ContentBorder = TargetWidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ContentBorder"));
	ContentBorder->SetPadding(FMargin(4.0f, 13.0f, 4.0f, 8.0f));
	ContentBorder->SetBrushColor(FLinearColor::Transparent);
	if (UOverlaySlot* ContentSlot = RootOverlay->AddChildToOverlay(ContentBorder))
	{
		ContentSlot->SetHorizontalAlignment(HAlign_Fill);
		ContentSlot->SetVerticalAlignment(VAlign_Fill);
	}

	ContentBox = TargetWidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContentBox"));
	ContentBorder->SetContent(ContentBox);

	ShortLabelText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ShortLabelText"));
	if (UVerticalBoxSlot* LabelSlot = ContentBox->AddChildToVerticalBox(ShortLabelText))
	{
		LabelSlot->SetHorizontalAlignment(HAlign_Center);
		LabelSlot->SetVerticalAlignment(VAlign_Center);
		LabelSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 11.0f));
	}

	ValueScaleBox = TargetWidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("ValueScaleBox"));
	ValueScaleBox->SetStretch(EStretch::ScaleToFit);
	if (UVerticalBoxSlot* ValueScaleSlot = ContentBox->AddChildToVerticalBox(ValueScaleBox))
	{
		ValueScaleSlot->SetHorizontalAlignment(HAlign_Fill);
		ValueScaleSlot->SetVerticalAlignment(VAlign_Center);
		ValueScaleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	USizeBox* ValueSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ValueSizeBox"));
	ValueSizeBox->SetHeightOverride(ProjectSinfulAscensionAttributeCardWidgetPrivate::ValueHeight);
	ValueScaleBox->AddChild(ValueSizeBox);

	ValueText = TargetWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ValueText"));
	ValueSizeBox->AddChild(ValueText);

	return true;
}

void UProjectSinfulAscensionAttributeCardWidget::InitializeVisualTree()
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
		if (UTexture2D* FrameTexture = ResolveTexture(CardFrameTexture, ProjectSinfulAscensionAttributeCardWidgetPrivate::CardFrameTexturePath))
		{
			FrameImage->SetBrushFromTexture(FrameTexture, false);
		}
	}

	if (ShortLabelText)
	{
		ShortLabelText->SetFont(MakeTitleFont(10, 0));
		ShortLabelText->SetJustification(ETextJustify::Center);
		ShortLabelText->SetShadowOffset(FVector2D(0.0f, 0.25f));
		ShortLabelText->SetShadowColorAndOpacity(ProjectSinfulAscensionAttributeCardWidgetPrivate::LabelShadowTint);
	}

	if (ValueText)
	{
		ValueText->SetFont(MakeBodyFont(15, 0));
		ValueText->SetJustification(ETextJustify::Center);
		ValueText->SetShadowOffset(FVector2D(0.0f, 0.35f));
		ValueText->SetShadowColorAndOpacity(ProjectSinfulAscensionAttributeCardWidgetPrivate::ValueShadowTint);
	}

	bVisualTreeInitialized = true;
}

void UProjectSinfulAscensionAttributeCardWidget::RefreshVisuals()
{
	const FLinearColor AccentTint = CurrentDisplayData.AccentTint.GetClamped(0.0f, 1.0f);

	if (bUsingNativeFallbackTree)
	{
		if (RootSizeBox)
		{
			RootSizeBox->SetWidthOverride(ProjectSinfulAscensionAttributeCardWidgetPrivate::CardWidth);
			RootSizeBox->SetHeightOverride(ProjectSinfulAscensionAttributeCardWidgetPrivate::CardHeight);
		}

		RefreshCardBrush(AccentTint);

		if (FrameImage)
		{
			const FLinearColor FrameTint = FLinearColor(
				FMath::Max(ProjectSinfulAscensionAttributeCardWidgetPrivate::BaseOutlineTint.R, AccentTint.R * 0.78f),
				FMath::Max(ProjectSinfulAscensionAttributeCardWidgetPrivate::BaseOutlineTint.G, AccentTint.G * 0.62f),
				FMath::Max(ProjectSinfulAscensionAttributeCardWidgetPrivate::BaseOutlineTint.B, AccentTint.B * 0.88f),
				0.44f);
			FrameImage->SetColorAndOpacity(FrameTint);
		}

		if (IconImage)
		{
			if (UTexture2D* IconTexture = ResolveTexture(CurrentDisplayData.IconTexture, ProjectSinfulAscensionAttributeCardWidgetPrivate::DefaultIconTexturePath))
			{
				IconImage->SetBrushFromTexture(IconTexture, false);
			}
			IconImage->SetColorAndOpacity(AccentTint.CopyWithNewOpacity(0.94f));
		}
	}

	if (ShortLabelText)
	{
		ShortLabelText->SetText(FText::FromString(CurrentDisplayData.ShortLabel));
		if (bUsingNativeFallbackTree)
		{
			ShortLabelText->SetColorAndOpacity(FSlateColor(AccentTint.CopyWithNewOpacity(0.92f)));
		}
	}

	if (ValueText)
	{
		ValueText->SetText(FText::AsNumber(CurrentDisplayData.Level));
		if (bUsingNativeFallbackTree)
		{
			ValueText->SetColorAndOpacity(FSlateColor(AccentTint.CopyWithNewOpacity(0.98f)));
		}
	}
}

void UProjectSinfulAscensionAttributeCardWidget::RefreshCardBrush(const FLinearColor& AccentTint)
{
	if (!BaseBorder || !bUsingNativeFallbackTree)
	{
		return;
	}

	const FLinearColor SafeAccent = AccentTint.GetClamped(0.0f, 1.0f);
	const FLinearColor OutlineTint = FLinearColor(
		FMath::Max(ProjectSinfulAscensionAttributeCardWidgetPrivate::BaseOutlineTint.R, SafeAccent.R * 0.76f),
		FMath::Max(ProjectSinfulAscensionAttributeCardWidgetPrivate::BaseOutlineTint.G, SafeAccent.G * 0.60f),
		FMath::Max(ProjectSinfulAscensionAttributeCardWidgetPrivate::BaseOutlineTint.B, SafeAccent.B * 0.82f),
		0.58f);
	const FSlateRoundedBoxBrush CardBrush(
		ProjectSinfulAscensionAttributeCardWidgetPrivate::BaseFillTint,
		ProjectSinfulAscensionAttributeCardWidgetPrivate::CardCornerRadius,
		FSlateColor(OutlineTint),
		ProjectSinfulAscensionAttributeCardWidgetPrivate::CardOutlineWidth,
		FVector2f(
			ProjectSinfulAscensionAttributeCardWidgetPrivate::CardWidth,
			ProjectSinfulAscensionAttributeCardWidgetPrivate::CardHeight));
	BaseBorder->SetBrush(CardBrush);
}

FProjectSinfulAscensionAttributeCardDisplayData UProjectSinfulAscensionAttributeCardWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
		TEXT("Attribute"),
		TEXT("ATTRIBUTE"),
		TEXT("ATR"),
		0,
		FLinearColor(0.92f, 0.40f, 0.72f, 1.0f),
		ProjectSinfulAscensionAttributeCardWidgetPrivate::DefaultIconTexturePath);
}

FProjectSinfulAscensionAttributeCardDisplayData UProjectSinfulAscensionAttributeCardGlobalWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
		TEXT("GlobalCard"),
		TEXT("GLOBAL CARD"),
		TEXT("GLB"),
		8,
		FLinearColor(0.94f, 0.52f, 0.82f, 1.0f),
		ProjectSinfulAscensionAttributeCardWidgetPrivate::DefaultIconTexturePath);
}

UProjectSinfulAscensionAttributeCardsGlobalWidget::UProjectSinfulAscensionAttributeCardsGlobalWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UProjectSinfulAscensionAttributeCardsGlobalWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
}

void UProjectSinfulAscensionAttributeCardsGlobalWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetTree();
}

bool UProjectSinfulAscensionAttributeCardsGlobalWidget::BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree)
	{
		return false;
	}

	WidgetTree = TargetWidgetTree;
	bUsingNativeFallbackTree = true;
	return BuildDefaultWidgetTree(TargetWidgetTree);
}

void UProjectSinfulAscensionAttributeCardsGlobalWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (RootSizeBox || WidgetTree->RootWidget)
	{
		bUsingNativeFallbackTree = false;
		return;
	}

	bUsingNativeFallbackTree = true;
	BuildDefaultWidgetTree(WidgetTree);
}

bool UProjectSinfulAscensionAttributeCardsGlobalWidget::BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree || TargetWidgetTree->RootWidget)
	{
		return false;
	}

	RootSizeBox = TargetWidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(ProjectSinfulAscensionAttributeCardWidgetPrivate::CardsGlobalWidth);
	RootSizeBox->SetHeightOverride(ProjectSinfulAscensionAttributeCardWidgetPrivate::CardsGlobalHeight);
	TargetWidgetTree->RootWidget = RootSizeBox;

	UVerticalBox* RootBox = TargetWidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CardsRootBox"));
	RootSizeBox->AddChild(RootBox);

	CardsBox = TargetWidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CardsBox"));
	if (UVerticalBoxSlot* CardsSlot = RootBox->AddChildToVerticalBox(CardsBox))
	{
		CardsSlot->SetHorizontalAlignment(HAlign_Center);
		CardsSlot->SetVerticalAlignment(VAlign_Center);
		CardsSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	const auto AddPreviewCard = [this, TargetWidgetTree](
		UClass* CardClass,
		const TCHAR* WidgetName,
		const FProjectSinfulAscensionAttributeCardDisplayData& PreviewData) -> UProjectSinfulAscensionAttributeCardWidget*
	{
		if (!CardsBox || !CardClass || !WidgetName)
		{
			return nullptr;
		}

		UProjectSinfulAscensionAttributeCardWidget* CardWidget =
			TargetWidgetTree->ConstructWidget<UProjectSinfulAscensionAttributeCardWidget>(CardClass, FName(WidgetName));
		if (!CardWidget)
		{
			return nullptr;
		}

		CardWidget->ApplyDisplayData(PreviewData);
		if (UHorizontalBoxSlot* CardSlot = CardsBox->AddChildToHorizontalBox(CardWidget))
		{
			CardSlot->SetPadding(FMargin(0.0f));
			CardSlot->SetHorizontalAlignment(HAlign_Center);
			CardSlot->SetVerticalAlignment(VAlign_Center);
			CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		}
		return CardWidget;
	};

	WillpowerCard = AddPreviewCard(
		UProjectSinfulAscensionWillpowerCardWidget::StaticClass(),
		TEXT("WillpowerCard"),
		ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
			TEXT("Willpower"),
			TEXT("WILLPOWER"),
			TEXT("WIL"),
			3,
			FLinearColor(0.97f, 0.67f, 0.85f, 1.0f),
			TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Willpower.T_SinfulAscension_Icon_Willpower")));

	SadismCard = AddPreviewCard(
		UProjectSinfulAscensionSadismCardWidget::StaticClass(),
		TEXT("SadismCard"),
		ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
			TEXT("Sadism"),
			TEXT("SADISM"),
			TEXT("SAD"),
			2,
			FLinearColor(0.95f, 0.54f, 0.78f, 1.0f),
			TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Sadism.T_SinfulAscension_Icon_Sadism")));

	MasochismCard = AddPreviewCard(
		UProjectSinfulAscensionMasochismCardWidget::StaticClass(),
		TEXT("MasochismCard"),
		ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
			TEXT("Masochism"),
			TEXT("MASOCHISM"),
			TEXT("MAS"),
			4,
			FLinearColor(0.96f, 0.45f, 0.74f, 1.0f),
			TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Masochism.T_SinfulAscension_Icon_Masochism")));

	FaithCard = AddPreviewCard(
		UProjectSinfulAscensionFaithCardWidget::StaticClass(),
		TEXT("FaithCard"),
		ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
			TEXT("Faith"),
			TEXT("FAITH"),
			TEXT("FAI"),
			1,
			FLinearColor(0.92f, 0.59f, 0.84f, 1.0f),
			TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Faith.T_SinfulAscension_Icon_Faith")));

	CunningCard = AddPreviewCard(
		UProjectSinfulAscensionCunningCardWidget::StaticClass(),
		TEXT("CunningCard"),
		ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
			TEXT("Cunning"),
			TEXT("CUNNING"),
			TEXT("CUN"),
			5,
			FLinearColor(0.98f, 0.57f, 0.82f, 1.0f),
			TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Cunning.T_SinfulAscension_Icon_Cunning")));

	CelerityCard = AddPreviewCard(
		UProjectSinfulAscensionCelerityCardWidget::StaticClass(),
		TEXT("CelerityCard"),
		ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
			TEXT("Celerity"),
			TEXT("CELERITY"),
			TEXT("CEL"),
			2,
			FLinearColor(0.88f, 0.63f, 0.88f, 1.0f),
			TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Celerity.T_SinfulAscension_Icon_Celerity")));

	AllureCard = AddPreviewCard(
		UProjectSinfulAscensionAllureCardWidget::StaticClass(),
		TEXT("AllureCard"),
		ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
			TEXT("Allure"),
			TEXT("ALLURE"),
			TEXT("ALL"),
			6,
			FLinearColor(0.99f, 0.63f, 0.86f, 1.0f),
			TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Allure.T_SinfulAscension_Icon_Allure")));

	ExtraCardsWrapBox = TargetWidgetTree->ConstructWidget<UWrapBox>(UWrapBox::StaticClass(), TEXT("ExtraCardsWrapBox"));
	ExtraCardsWrapBox->SetVisibility(ESlateVisibility::Collapsed);
	if (UVerticalBoxSlot* ExtraSlot = RootBox->AddChildToVerticalBox(ExtraCardsWrapBox))
	{
		ExtraSlot->SetHorizontalAlignment(HAlign_Center);
		ExtraSlot->SetVerticalAlignment(VAlign_Center);
		ExtraSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	return true;
}

int32 UProjectSinfulAscensionAttributeCardsGlobalWidget::ApplyCards(
	const TArray<FProjectSinfulAscensionAttributeCardDisplayData>& InCardData,
	TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> FallbackCardWidgetClass)
{
	BuildWidgetTree();

	RuntimeCardsByName.Empty();
	TSet<FName> VisibleFixedCardNames;
	VisibleCardCount = 0;
	ClearFallbackCards();

	for (const FProjectSinfulAscensionAttributeCardDisplayData& CardData : InCardData)
	{
		UProjectSinfulAscensionAttributeCardWidget* CardWidget = GetFixedCardForAttribute(CardData.AttributeName);
		if (CardWidget)
		{
			VisibleFixedCardNames.Add(CardData.AttributeName);
		}
		else
		{
			CardWidget = GetOrCreateFallbackCard(CardData.AttributeName, FallbackCardWidgetClass);
		}

		if (!CardWidget)
		{
			continue;
		}

		CardWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CardWidget->ApplyDisplayData(CardData);
		RuntimeCardsByName.Add(CardData.AttributeName, CardWidget);
		++VisibleCardCount;
	}

	HideUnusedFixedCards(VisibleFixedCardNames);
	if (ExtraCardsWrapBox)
	{
		ExtraCardsWrapBox->SetVisibility(FallbackCardsByName.IsEmpty()
			? ESlateVisibility::Collapsed
			: ESlateVisibility::SelfHitTestInvisible);
	}

	OnSinfulAttributeCardsGlobalApplied(VisibleCardCount);
	return VisibleCardCount;
}

UProjectSinfulAscensionAttributeCardWidget* UProjectSinfulAscensionAttributeCardsGlobalWidget::FindCardWidgetByAttribute(const FName AttributeName) const
{
	if (const TObjectPtr<UProjectSinfulAscensionAttributeCardWidget>* FoundWidget = RuntimeCardsByName.Find(AttributeName))
	{
		return FoundWidget->Get();
	}

	return GetFixedCardForAttribute(AttributeName);
}

UProjectSinfulAscensionAttributeCardWidget* UProjectSinfulAscensionAttributeCardsGlobalWidget::GetFixedCardForAttribute(const FName AttributeName) const
{
	if (AttributeName == FName(TEXT("Willpower")))
	{
		return WillpowerCard.Get();
	}
	if (AttributeName == FName(TEXT("Sadism")))
	{
		return SadismCard.Get();
	}
	if (AttributeName == FName(TEXT("Masochism")))
	{
		return MasochismCard.Get();
	}
	if (AttributeName == FName(TEXT("Faith")))
	{
		return FaithCard.Get();
	}
	if (AttributeName == FName(TEXT("Cunning")))
	{
		return CunningCard.Get();
	}
	if (AttributeName == FName(TEXT("Celerity")))
	{
		return CelerityCard.Get();
	}
	if (AttributeName == FName(TEXT("Allure")))
	{
		return AllureCard.Get();
	}

	return nullptr;
}

UProjectSinfulAscensionAttributeCardWidget* UProjectSinfulAscensionAttributeCardsGlobalWidget::GetOrCreateFallbackCard(
	const FName AttributeName,
	TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> FallbackCardWidgetClass)
{
	if (!ExtraCardsWrapBox || !FallbackCardWidgetClass)
	{
		return nullptr;
	}

	if (TObjectPtr<UProjectSinfulAscensionAttributeCardWidget>* ExistingCard = FallbackCardsByName.Find(AttributeName))
	{
		return ExistingCard->Get();
	}

	UProjectSinfulAscensionAttributeCardWidget* FallbackCard = CreateWidget<UProjectSinfulAscensionAttributeCardWidget>(
		this,
		FallbackCardWidgetClass.Get());
	if (!FallbackCard)
	{
		return nullptr;
	}

	if (UWrapBoxSlot* FallbackSlot = ExtraCardsWrapBox->AddChildToWrapBox(FallbackCard))
	{
		FallbackSlot->SetPadding(FMargin(0.0f));
		FallbackSlot->SetHorizontalAlignment(HAlign_Left);
		FallbackSlot->SetVerticalAlignment(VAlign_Center);
		FallbackSlot->SetFillEmptySpace(false);
		FallbackSlot->SetFillSpanWhenLessThan(0.0f);
	}

	FallbackCardsByName.Add(AttributeName, FallbackCard);
	return FallbackCard;
}

void UProjectSinfulAscensionAttributeCardsGlobalWidget::HideUnusedFixedCards(const TSet<FName>& VisibleFixedCardNames)
{
	const auto HideIfUnused = [&VisibleFixedCardNames](const FName AttributeName, UProjectSinfulAscensionAttributeCardWidget* CardWidget)
	{
		if (CardWidget && !VisibleFixedCardNames.Contains(AttributeName))
		{
			CardWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	};

	HideIfUnused(FName(TEXT("Willpower")), WillpowerCard.Get());
	HideIfUnused(FName(TEXT("Sadism")), SadismCard.Get());
	HideIfUnused(FName(TEXT("Masochism")), MasochismCard.Get());
	HideIfUnused(FName(TEXT("Faith")), FaithCard.Get());
	HideIfUnused(FName(TEXT("Cunning")), CunningCard.Get());
	HideIfUnused(FName(TEXT("Celerity")), CelerityCard.Get());
	HideIfUnused(FName(TEXT("Allure")), AllureCard.Get());
}

void UProjectSinfulAscensionAttributeCardsGlobalWidget::ClearFallbackCards()
{
	if (ExtraCardsWrapBox)
	{
		ExtraCardsWrapBox->ClearChildren();
	}
	FallbackCardsByName.Empty();
}

FProjectSinfulAscensionAttributeCardDisplayData UProjectSinfulAscensionWillpowerCardWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
		TEXT("Willpower"),
		TEXT("WILLPOWER"),
		TEXT("WIL"),
		3,
		FLinearColor(0.97f, 0.67f, 0.85f, 1.0f),
		TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Willpower.T_SinfulAscension_Icon_Willpower"));
}

FProjectSinfulAscensionAttributeCardDisplayData UProjectSinfulAscensionSadismCardWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
		TEXT("Sadism"),
		TEXT("SADISM"),
		TEXT("SAD"),
		2,
		FLinearColor(0.95f, 0.54f, 0.78f, 1.0f),
		TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Sadism.T_SinfulAscension_Icon_Sadism"));
}

FProjectSinfulAscensionAttributeCardDisplayData UProjectSinfulAscensionMasochismCardWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
		TEXT("Masochism"),
		TEXT("MASOCHISM"),
		TEXT("MAS"),
		4,
		FLinearColor(0.96f, 0.45f, 0.74f, 1.0f),
		TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Masochism.T_SinfulAscension_Icon_Masochism"));
}

FProjectSinfulAscensionAttributeCardDisplayData UProjectSinfulAscensionFaithCardWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
		TEXT("Faith"),
		TEXT("FAITH"),
		TEXT("FAI"),
		1,
		FLinearColor(0.92f, 0.59f, 0.84f, 1.0f),
		TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Faith.T_SinfulAscension_Icon_Faith"));
}

FProjectSinfulAscensionAttributeCardDisplayData UProjectSinfulAscensionCunningCardWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
		TEXT("Cunning"),
		TEXT("CUNNING"),
		TEXT("CUN"),
		5,
		FLinearColor(0.98f, 0.57f, 0.82f, 1.0f),
		TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Cunning.T_SinfulAscension_Icon_Cunning"));
}

FProjectSinfulAscensionAttributeCardDisplayData UProjectSinfulAscensionCelerityCardWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
		TEXT("Celerity"),
		TEXT("CELERITY"),
		TEXT("CEL"),
		2,
		FLinearColor(0.88f, 0.63f, 0.88f, 1.0f),
		TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Celerity.T_SinfulAscension_Icon_Celerity"));
}

FProjectSinfulAscensionAttributeCardDisplayData UProjectSinfulAscensionAllureCardWidget::MakeDesignerPreviewData() const
{
	return ProjectSinfulAscensionAttributeCardWidgetPrivate::MakePreviewData(
		TEXT("Allure"),
		TEXT("ALLURE"),
		TEXT("ALL"),
		6,
		FLinearColor(0.99f, 0.63f, 0.86f, 1.0f),
		TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/T_SinfulAscension_Icon_Allure.T_SinfulAscension_Icon_Allure"));
}

UTexture2D* UProjectSinfulAscensionAttributeCardWidget::ResolveTexture(
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

	return Cast<UTexture2D>(ProjectSinfulAscensionAttributeCardWidgetPrivate::LoadObjectByPath(FallbackPath));
}

UObject* UProjectSinfulAscensionAttributeCardWidget::ResolveStyleAsset(
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

	return ProjectSinfulAscensionAttributeCardWidgetPrivate::LoadObjectByPath(FallbackPath);
}

FSlateFontInfo UProjectSinfulAscensionAttributeCardWidget::MakeTitleFont(const int32 Size, const int32 LetterSpacing) const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), Size);
	FontInfo.Size = Size;
	FontInfo.LetterSpacing = LetterSpacing;
	if (UObject* FontObject = ResolveStyleAsset(TitleFontAsset, ProjectSinfulAscensionAttributeCardWidgetPrivate::CinzelFontPath))
	{
		FontInfo.FontObject = FontObject;
		FontInfo.TypefaceFontName = NAME_None;
	}

	return FontInfo;
}

FSlateFontInfo UProjectSinfulAscensionAttributeCardWidget::MakeBodyFont(const int32 Size, const int32 LetterSpacing) const
{
	FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Size);
	FontInfo.Size = Size;
	FontInfo.LetterSpacing = LetterSpacing;
	if (UObject* FontObject = ResolveStyleAsset(BodyFontAsset, ProjectSinfulAscensionAttributeCardWidgetPrivate::CormorantFontPath))
	{
		FontInfo.FontObject = FontObject;
		FontInfo.TypefaceFontName = NAME_None;
	}

	return FontInfo;
}
