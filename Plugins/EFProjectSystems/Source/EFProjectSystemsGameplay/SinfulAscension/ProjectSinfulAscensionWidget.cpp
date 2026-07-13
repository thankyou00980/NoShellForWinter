#include "SinfulAscension/ProjectSinfulAscensionWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "EFProjectUISettings.h"
#include "SinfulAscension/ProjectSinfulAscensionAttributesPanelWidget.h"
#include "SinfulAscension/ProjectSinfulAscensionComponent.h"
#include "UI/ProjectWidgetClassResolver.h"

namespace ProjectSinfulAscensionWidgetPrivate
{
	constexpr float PanelWidth = 448.0f;
	constexpr float BasePanelHeight = 206.0f;
	constexpr float AdditionalCardRowHeight = 76.0f;
	constexpr int32 MaxCardsPerRow = 7;
	constexpr int32 RuntimeFallbackSortBase = 1000;

	const FLinearColor DefaultAccentTint(0.92f, 0.40f, 0.72f, 1.0f);

	UCanvasPanelSlot* AddCanvasChild(
		UCanvasPanel* Canvas,
		UWidget* Child,
		const FAnchors& Anchors,
		const FVector2D& Alignment,
		const FVector2D& Position,
		const FVector2D& Size,
		const int32 ZOrder)
	{
		if (!Canvas || !Child)
		{
			return nullptr;
		}

		UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
		if (!Slot)
		{
			return nullptr;
		}

		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(Alignment);
		Slot->SetPosition(Position);
		Slot->SetZOrder(ZOrder);
		Slot->SetSize(Size);
		return Slot;
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

	FString NormalizeDisplayLabel(const FString& Source)
	{
		FString Result = Source;
		Result.TrimStartAndEndInline();
		if (Result.IsEmpty())
		{
			Result = TEXT("UNKNOWN");
		}
		Result.ToUpperInline();
		return Result;
	}

	FString BuildShortLabel(const FString& DisplayLabel)
	{
		FString Result;
		Result.Reserve(3);

		for (const TCHAR Character : DisplayLabel)
		{
			if (!FChar::IsAlpha(Character))
			{
				continue;
			}

			Result.AppendChar(FChar::ToUpper(Character));
			if (Result.Len() >= 3)
			{
				break;
			}
		}

		if (Result.IsEmpty())
		{
			return TEXT("???");
		}

		while (Result.Len() < 3)
		{
			Result.AppendChar(Result[Result.Len() - 1]);
		}

		return Result;
	}

	struct FResolvedSinfulAscensionCard
	{
		FProjectSinfulAscensionAttributeCardDisplayData CardData;
		int32 SortOrder = 0;
	};
}

UProjectSinfulAscensionWidget::UProjectSinfulAscensionWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SinfulAscensionComponent(nullptr)
{
	ZOrder = 120;
	CurrentPanelHeight = ProjectSinfulAscensionWidgetPrivate::BasePanelHeight;
}

void UProjectSinfulAscensionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	EnsureAttributesPanelWidget();
	RefreshDisplay();
}

void UProjectSinfulAscensionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildWidgetTree();
	EnsureAttributesPanelWidget();
	ApplyHudVisibility();
	RefreshDisplay();
}

void UProjectSinfulAscensionWidget::NativeDestruct()
{
	CardWidgetsByName.Empty();
	CachedCardOrder.Reset();
	AttributesCardsGlobalWidget = nullptr;
	AttributesWrapBox = nullptr;
	AttributesPanelWidget = nullptr;
	SinfulAscensionComponent = nullptr;
	Super::NativeDestruct();
}

bool UProjectSinfulAscensionWidget::BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree)
	{
		return false;
	}

	WidgetTree = TargetWidgetTree;
	bUsingNativeFallbackTree = true;
	CurrentPanelHeight = ProjectSinfulAscensionWidgetPrivate::BasePanelHeight;
	return BuildDefaultWidgetTree(TargetWidgetTree);
}

void UProjectSinfulAscensionWidget::GatherCodeWidgetDesignerChildWidgetSpecs(TArray<FCodeWidgetDesignerChildWidgetSpec>& OutWidgetSpecs) const
{
	const auto AddSpec = [&OutWidgetSpecs](
		TSubclassOf<UUserWidget> WidgetClass,
		const FString& RelativeFolder,
		const FString& AssetNameOverride,
		const ECodeWidgetDesignerAssetRole Role,
		const int32 PriorityRank,
		const bool bRuntimeDefault = false,
		const bool bRequiresStableRootWrapper = false,
		TArray<FName> ExpectedWidgetNames = TArray<FName>())
	{
		FCodeWidgetDesignerChildWidgetSpec Spec;
		Spec.WidgetClass = WidgetClass;
		Spec.RelativeFolder = RelativeFolder;
		Spec.AssetNameOverride = AssetNameOverride;
		Spec.Role = Role;
		Spec.PriorityGroup = FName(TEXT("Attributes"));
		Spec.PriorityRank = PriorityRank;
		Spec.bRuntimeDefault = bRuntimeDefault;
		Spec.bRequiresStableRootWrapper = bRequiresStableRootWrapper;
		Spec.ExpectedWidgetNames = MoveTemp(ExpectedWidgetNames);
		OutWidgetSpecs.Add(Spec);
	};

	const TArray<FName> CardWidgetNames = {
		TEXT("DesignerRootOverlay"),
		TEXT("RootSizeBox"),
		TEXT("RootOverlay"),
		TEXT("ShortLabelText"),
		TEXT("ValueText")
	};

	AddSpec(UProjectSinfulAscensionAttributesPanelWidget::StaticClass(), TEXT("Globals"), TEXT("WBP_ProjectSinfulAttributesGlobal"), ECodeWidgetDesignerAssetRole::GlobalPanel, 9000, true, false, { TEXT("AttributeCardsGlobal") });
	AddSpec(UProjectSinfulAscensionAttributeCardGlobalWidget::StaticClass(), TEXT("Globals"), TEXT("WBP_ProjectSinfulAttributeCardGlobal"), ECodeWidgetDesignerAssetRole::GlobalTemplate, 10000, true, true, CardWidgetNames);
	AddSpec(UProjectSinfulAscensionAttributeCardsGlobalWidget::StaticClass(), TEXT("Globals"), TEXT("WBP_ProjectSinfulAttributesCardsGlobal"), ECodeWidgetDesignerAssetRole::GlobalPanel, 8000, false, false, { TEXT("WillpowerCard"), TEXT("SadismCard"), TEXT("MasochismCard"), TEXT("FaithCard"), TEXT("CunningCard"), TEXT("CelerityCard"), TEXT("AllureCard") });
	AddSpec(UProjectSinfulAscensionAttributeCardWidget::StaticClass(), TEXT("Main"), TEXT("WBP_ProjectSinfulAscensionAttributeCard"), ECodeWidgetDesignerAssetRole::MainBase, 1000, false, true, CardWidgetNames);
	AddSpec(UProjectSinfulAscensionWillpowerCardWidget::StaticClass(), TEXT("Cards"), TEXT("WBP_ProjectSinfulWillpowerCard"), ECodeWidgetDesignerAssetRole::Individual, 500, false, true, CardWidgetNames);
	AddSpec(UProjectSinfulAscensionSadismCardWidget::StaticClass(), TEXT("Cards"), TEXT("WBP_ProjectSinfulSadismCard"), ECodeWidgetDesignerAssetRole::Individual, 500, false, true, CardWidgetNames);
	AddSpec(UProjectSinfulAscensionMasochismCardWidget::StaticClass(), TEXT("Cards"), TEXT("WBP_ProjectSinfulMasochismCard"), ECodeWidgetDesignerAssetRole::Individual, 500, false, true, CardWidgetNames);
	AddSpec(UProjectSinfulAscensionFaithCardWidget::StaticClass(), TEXT("Cards"), TEXT("WBP_ProjectSinfulFaithCard"), ECodeWidgetDesignerAssetRole::Individual, 500, false, true, CardWidgetNames);
	AddSpec(UProjectSinfulAscensionCunningCardWidget::StaticClass(), TEXT("Cards"), TEXT("WBP_ProjectSinfulCunningCard"), ECodeWidgetDesignerAssetRole::Individual, 500, false, true, CardWidgetNames);
	AddSpec(UProjectSinfulAscensionCelerityCardWidget::StaticClass(), TEXT("Cards"), TEXT("WBP_ProjectSinfulCelerityCard"), ECodeWidgetDesignerAssetRole::Individual, 500, false, true, CardWidgetNames);
	AddSpec(UProjectSinfulAscensionAllureCardWidget::StaticClass(), TEXT("Cards"), TEXT("WBP_ProjectSinfulAllureCard"), ECodeWidgetDesignerAssetRole::Individual, 500, false, true, CardWidgetNames);
}

void UProjectSinfulAscensionWidget::SetSinfulAscensionComponent(UProjectSinfulAscensionComponent* InComponent)
{
	if (SinfulAscensionComponent == InComponent)
	{
		return;
	}

	SinfulAscensionComponent = InComponent;
	RefreshDisplay();
}

void UProjectSinfulAscensionWidget::RefreshDisplay()
{
	BuildWidgetTree();
	EnsureAttributesPanelWidget();
	SyncAttributesContainer();

	CachedSnapshot = SinfulAscensionComponent
		? SinfulAscensionComponent->BuildSnapshot()
		: FProjectSinfulAscensionSnapshot();

	const TArray<FProjectSinfulAscensionAttributeCardDisplayData> ResolvedCards = BuildResolvedCardData();
	if (DoesCardLayoutNeedRebuild(ResolvedCards))
	{
		RebuildAttributeCards(ResolvedCards);
	}

	for (const FProjectSinfulAscensionAttributeCardDisplayData& CardData : ResolvedCards)
	{
		if (TObjectPtr<UProjectSinfulAscensionAttributeCardWidget>* ExistingCard = CardWidgetsByName.Find(CardData.AttributeName))
		{
			if (UProjectSinfulAscensionAttributeCardWidget* CardWidget = ExistingCard->Get())
			{
				CardWidget->ApplyDisplayData(CardData);
			}
		}
	}

	ApplyPanelState(ResolvedCards.Num());
}

void UProjectSinfulAscensionWidget::SetHudVisible(const bool bVisible)
{
	if (bHudVisible == bVisible)
	{
		return;
	}

	bHudVisible = bVisible;
	ApplyHudVisibility();
	if (bHudVisible)
	{
		RefreshDisplay();
	}
}

bool UProjectSinfulAscensionWidget::IsHudVisible() const
{
	return bHudVisible;
}

FProjectSinfulAscensionSnapshot UProjectSinfulAscensionWidget::GetCachedSnapshot() const
{
	return CachedSnapshot;
}

int32 UProjectSinfulAscensionWidget::GetRenderedAttributeCardCount() const
{
	if (AttributesCardsGlobalWidget)
	{
		return AttributesCardsGlobalWidget->GetVisibleCardCount();
	}
	return AttributesWrapBox ? AttributesWrapBox->GetChildrenCount() : 0;
}

void UProjectSinfulAscensionWidget::ApplyHudVisibility()
{
	SetVisibility(bHudVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UProjectSinfulAscensionWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (RootCanvas || WidgetTree->RootWidget)
	{
		bUsingNativeFallbackTree = false;
		if (PanelHost && !PanelHostSlot)
		{
			PanelHostSlot = Cast<UCanvasPanelSlot>(PanelHost->Slot);
		}

		if (!PanelHost && RootCanvas)
		{
			PanelHost = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("PanelHost"));
			PanelHostSlot = ProjectSinfulAscensionWidgetPrivate::AddCanvasChild(
				RootCanvas,
				PanelHost,
				FAnchors(0.0f, 1.0f),
				FVector2D(0.0f, 1.0f),
				FVector2D(10.0f, -320.0f),
				FVector2D(ProjectSinfulAscensionWidgetPrivate::PanelWidth, CurrentPanelHeight),
				ZOrder);
		}
		return;
	}

	bUsingNativeFallbackTree = true;
	BuildDefaultWidgetTree(WidgetTree);
}

bool UProjectSinfulAscensionWidget::BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree)
{
	if (!TargetWidgetTree || TargetWidgetTree->RootWidget)
	{
		return false;
	}

	RootCanvas = TargetWidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	TargetWidgetTree->RootWidget = RootCanvas;

	PanelHost = TargetWidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("PanelHost"));
	PanelHostSlot = ProjectSinfulAscensionWidgetPrivate::AddCanvasChild(
		RootCanvas,
		PanelHost,
		FAnchors(0.0f, 1.0f),
		FVector2D(0.0f, 1.0f),
		FVector2D(10.0f, -320.0f),
		FVector2D(ProjectSinfulAscensionWidgetPrivate::PanelWidth, CurrentPanelHeight),
		ZOrder);

	AttributesPanelWidget = TargetWidgetTree->ConstructWidget<UProjectSinfulAscensionAttributesPanelWidget>(
		UProjectSinfulAscensionAttributesPanelWidget::StaticClass(),
		TEXT("AttributesPanelWidget"));
	if (AttributesPanelWidget)
	{
		if (UOverlaySlot* PanelSlot = PanelHost->AddChildToOverlay(AttributesPanelWidget))
		{
			PanelSlot->SetHorizontalAlignment(HAlign_Fill);
			PanelSlot->SetVerticalAlignment(VAlign_Fill);
		}
		AttributesPanelWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	return true;
}

void UProjectSinfulAscensionWidget::EnsureAttributesPanelWidget()
{
	if (!PanelHost)
	{
		return;
	}

	if (!AttributesPanelWidget)
	{
		const TSubclassOf<UProjectSinfulAscensionAttributesPanelWidget> PanelClass = ResolveAttributesPanelWidgetClass();
		AttributesPanelWidget = CreateWidget<UProjectSinfulAscensionAttributesPanelWidget>(
			this,
			PanelClass ? PanelClass.Get() : UProjectSinfulAscensionAttributesPanelWidget::StaticClass());
		if (AttributesPanelWidget)
		{
			if (UOverlaySlot* PanelSlot = PanelHost->AddChildToOverlay(AttributesPanelWidget))
			{
				PanelSlot->SetHorizontalAlignment(HAlign_Fill);
				PanelSlot->SetVerticalAlignment(VAlign_Fill);
			}
			AttributesPanelWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	SyncAttributesContainer();
}

void UProjectSinfulAscensionWidget::SyncAttributesContainer()
{
	AttributesCardsGlobalWidget = AttributesPanelWidget ? AttributesPanelWidget->GetAttributeCardsGlobalWidget() : nullptr;
	AttributesWrapBox = AttributesPanelWidget ? AttributesPanelWidget->GetAttributesWrapBox() : nullptr;
}

void UProjectSinfulAscensionWidget::ApplyPanelState(const int32 CardCount)
{
	CurrentPanelHeight = CalculatePanelHeight(CardCount);

	if (PanelHostSlot && bUsingNativeFallbackTree)
	{
		PanelHostSlot->SetSize(FVector2D(ProjectSinfulAscensionWidgetPrivate::PanelWidth, CurrentPanelHeight));
	}

	if (AttributesPanelWidget)
	{
		AttributesPanelWidget->ApplyPanelState(
			CachedSnapshot.bEternalSinMode,
			CachedSnapshot.CurrentRunSxp,
			CachedSnapshot.MetaBankSxp,
			CurrentPanelHeight,
			CardCount);
	}
}

float UProjectSinfulAscensionWidget::CalculatePanelHeight(const int32 CardCount) const
{
	const int32 SafeCardCount = FMath::Max(1, CardCount);
	const int32 RowCount = FMath::Max(1, FMath::DivideAndRoundUp(SafeCardCount, ProjectSinfulAscensionWidgetPrivate::MaxCardsPerRow));
	return ProjectSinfulAscensionWidgetPrivate::BasePanelHeight
		+ static_cast<float>(RowCount - 1) * ProjectSinfulAscensionWidgetPrivate::AdditionalCardRowHeight;
}

TArray<FProjectSinfulAscensionAttributeCardDisplayData> UProjectSinfulAscensionWidget::BuildResolvedCardData() const
{
	TMap<FName, const FProjectSinAttributeState*> RuntimeDataByName;
	for (const FProjectSinAttributeState& AttributeState : CachedSnapshot.Attributes)
	{
		FName AttributeName = ProjectSinfulAscensionWidgetPrivate::BuildAttributeName(AttributeState.Attribute);
		if (AttributeName.IsNone())
		{
			AttributeName = FName(*AttributeState.DisplayName.ToString().Replace(TEXT(" "), TEXT("")));
		}

		if (!AttributeName.IsNone())
		{
			RuntimeDataByName.Add(AttributeName, &AttributeState);
		}
	}

	TArray<ProjectSinfulAscensionWidgetPrivate::FResolvedSinfulAscensionCard> ResolvedCards;
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

		ProjectSinfulAscensionWidgetPrivate::FResolvedSinfulAscensionCard ResolvedCard;
		ResolvedCard.SortOrder = Definition.SortOrder;
		ResolvedCard.CardData.AttributeName = Definition.AttributeName;
		ResolvedCard.CardData.DisplayLabel = Definition.DisplayLabel.IsEmpty()
			? ProjectSinfulAscensionWidgetPrivate::NormalizeDisplayLabel((*RuntimeState)->DisplayName.ToString())
			: Definition.DisplayLabel;
		ResolvedCard.CardData.ShortLabel = Definition.ShortLabel.IsEmpty()
			? ProjectSinfulAscensionWidgetPrivate::BuildShortLabel(ResolvedCard.CardData.DisplayLabel)
			: Definition.ShortLabel;
		ResolvedCard.CardData.IconTexture = Definition.IconTexture;
		ResolvedCard.CardData.AccentTint = Definition.AccentTint;
		ResolvedCard.CardData.Level = (*RuntimeState)->Level;
		ResolvedCards.Add(ResolvedCard);
		RuntimeDataByName.Remove(Definition.AttributeName);
	}

	int32 FallbackSortOffset = 0;
	for (const TPair<FName, const FProjectSinAttributeState*>& Pair : RuntimeDataByName)
	{
		if (!Pair.Value)
		{
			continue;
		}

		ProjectSinfulAscensionWidgetPrivate::FResolvedSinfulAscensionCard ResolvedCard;
		ResolvedCard.SortOrder = ProjectSinfulAscensionWidgetPrivate::RuntimeFallbackSortBase + FallbackSortOffset++;
		ResolvedCard.CardData.AttributeName = Pair.Key;
		ResolvedCard.CardData.DisplayLabel = ProjectSinfulAscensionWidgetPrivate::NormalizeDisplayLabel(Pair.Value->DisplayName.ToString());
		ResolvedCard.CardData.ShortLabel = ProjectSinfulAscensionWidgetPrivate::BuildShortLabel(ResolvedCard.CardData.DisplayLabel);
		ResolvedCard.CardData.IconTexture.Reset();
		ResolvedCard.CardData.AccentTint = ProjectSinfulAscensionWidgetPrivate::DefaultAccentTint;
		ResolvedCard.CardData.Level = Pair.Value->Level;
		ResolvedCards.Add(ResolvedCard);
	}

	ResolvedCards.Sort([](
		const ProjectSinfulAscensionWidgetPrivate::FResolvedSinfulAscensionCard& Left,
		const ProjectSinfulAscensionWidgetPrivate::FResolvedSinfulAscensionCard& Right)
	{
		if (Left.SortOrder != Right.SortOrder)
		{
			return Left.SortOrder < Right.SortOrder;
		}

		return Left.CardData.AttributeName.LexicalLess(Right.CardData.AttributeName);
	});

	TArray<FProjectSinfulAscensionAttributeCardDisplayData> FinalCards;
	FinalCards.Reserve(ResolvedCards.Num());
	for (const ProjectSinfulAscensionWidgetPrivate::FResolvedSinfulAscensionCard& ResolvedCard : ResolvedCards)
	{
		FinalCards.Add(ResolvedCard.CardData);
	}

	return FinalCards;
}

bool UProjectSinfulAscensionWidget::DoesCardLayoutNeedRebuild(const TArray<FProjectSinfulAscensionAttributeCardDisplayData>& InCardData) const
{
	if (CachedCardOrder.Num() != InCardData.Num())
	{
		return true;
	}

	for (int32 Index = 0; Index < InCardData.Num(); ++Index)
	{
		if (!CachedCardOrder.IsValidIndex(Index) || CachedCardOrder[Index] != InCardData[Index].AttributeName)
		{
			return true;
		}
	}

	return false;
}

void UProjectSinfulAscensionWidget::RebuildAttributeCards(const TArray<FProjectSinfulAscensionAttributeCardDisplayData>& InCardData)
{
	SyncAttributesContainer();
	if (AttributesCardsGlobalWidget)
	{
		CardWidgetsByName.Empty();
		CachedCardOrder.Reset();

		const int32 VisibleCardCount = AttributesCardsGlobalWidget->ApplyCards(
			InCardData,
			ResolveGlobalAttributeCardWidgetClass());

		for (const FProjectSinfulAscensionAttributeCardDisplayData& CardData : InCardData)
		{
			if (UProjectSinfulAscensionAttributeCardWidget* CardWidget = AttributesCardsGlobalWidget->FindCardWidgetByAttribute(CardData.AttributeName))
			{
				CardWidgetsByName.Add(CardData.AttributeName, CardWidget);
			}
			CachedCardOrder.Add(CardData.AttributeName);
		}

		if (AttributesWrapBox)
		{
			AttributesWrapBox->ClearChildren();
			AttributesWrapBox->SetVisibility(ESlateVisibility::Collapsed);
		}

		OnSinfulAttributesRebuilt(VisibleCardCount);
		return;
	}

	if (!AttributesWrapBox)
	{
		return;
	}

	AttributesWrapBox->ClearChildren();
	CardWidgetsByName.Empty();
	CachedCardOrder.Reset();

	for (const FProjectSinfulAscensionAttributeCardDisplayData& CardData : InCardData)
	{
		const TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> ResolvedCardWidgetClass =
			ResolveAttributeCardWidgetClassForData(CardData);
		if (!ResolvedCardWidgetClass)
		{
			continue;
		}

		UProjectSinfulAscensionAttributeCardWidget* CardWidget = CreateWidget<UProjectSinfulAscensionAttributeCardWidget>(
			this,
			ResolvedCardWidgetClass.Get());
		if (!CardWidget)
		{
			continue;
		}

		CardWidget->ApplyDisplayData(CardData);
		if (UWrapBoxSlot* CardSlot = AttributesWrapBox->AddChildToWrapBox(CardWidget))
		{
			CardSlot->SetPadding(FMargin(0.0f));
			CardSlot->SetHorizontalAlignment(HAlign_Left);
			CardSlot->SetVerticalAlignment(VAlign_Center);
			CardSlot->SetFillEmptySpace(false);
			CardSlot->SetFillSpanWhenLessThan(0.0f);
		}

		CardWidgetsByName.Add(CardData.AttributeName, CardWidget);
		CachedCardOrder.Add(CardData.AttributeName);
	}

	OnSinfulAttributesRebuilt(CardWidgetsByName.Num());
}

TSubclassOf<UProjectSinfulAscensionAttributesPanelWidget> UProjectSinfulAscensionWidget::ResolveAttributesPanelWidgetClass() const
{
	if (UClass* DiscoveredClass = ProjectWidgetClassResolver::ResolveWidgetClass(
		FSoftClassPath(),
		UProjectSinfulAscensionAttributesPanelWidget::StaticClass(),
		TEXT("ProjectSinfulAttributesGlobal")))
	{
		return DiscoveredClass;
	}

	return UProjectSinfulAscensionAttributesPanelWidget::StaticClass();
}

TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> UProjectSinfulAscensionWidget::ResolveAttributeCardWidgetClass() const
{
	if (!AttributeCardWidgetClass.IsNull())
	{
		if (UClass* LoadedClass = AttributeCardWidgetClass.LoadSynchronous())
		{
			return LoadedClass;
		}
	}

	return ProjectWidgetClassResolver::DiscoverWidgetClass<UProjectSinfulAscensionAttributeCardWidget>(TEXT("ProjectSinfulAscensionAttributeCard"));
}

TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> UProjectSinfulAscensionWidget::ResolveGlobalAttributeCardWidgetClass() const
{
	if (UClass* GlobalCardClass = ProjectWidgetClassResolver::ResolveWidgetClass(
		FSoftClassPath(),
		UProjectSinfulAscensionAttributeCardGlobalWidget::StaticClass(),
		TEXT("ProjectSinfulAttributeCardGlobal")))
	{
		return GlobalCardClass;
	}

	return UProjectSinfulAscensionAttributeCardWidget::StaticClass();
}

TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> UProjectSinfulAscensionWidget::ResolveAttributeCardWidgetClassForData(
	const FProjectSinfulAscensionAttributeCardDisplayData& CardData) const
{
	const TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> GlobalCardClass = ResolveGlobalAttributeCardWidgetClass();
	if (GlobalCardClass)
	{
		return GlobalCardClass;
	}

	UClass* NativeCardClass = ResolveNativeCardClassForAttribute(CardData.AttributeName);
	if (UClass* DiscoveredClass = ProjectWidgetClassResolver::ResolveWidgetClass(
		FSoftClassPath(),
		NativeCardClass,
		TEXT("ProjectSinfulAttributeCard")))
	{
		if (DiscoveredClass != NativeCardClass)
		{
			return DiscoveredClass;
		}
	}

	if (!AttributeCardWidgetClass.IsNull())
	{
		return ResolveAttributeCardWidgetClass();
	}

	return ResolveAttributeCardWidgetClass();
}

UClass* UProjectSinfulAscensionWidget::ResolveNativeCardClassForAttribute(const FName AttributeName) const
{
	if (AttributeName == FName(TEXT("Willpower")))
	{
		return UProjectSinfulAscensionWillpowerCardWidget::StaticClass();
	}
	if (AttributeName == FName(TEXT("Sadism")))
	{
		return UProjectSinfulAscensionSadismCardWidget::StaticClass();
	}
	if (AttributeName == FName(TEXT("Masochism")))
	{
		return UProjectSinfulAscensionMasochismCardWidget::StaticClass();
	}
	if (AttributeName == FName(TEXT("Faith")))
	{
		return UProjectSinfulAscensionFaithCardWidget::StaticClass();
	}
	if (AttributeName == FName(TEXT("Cunning")))
	{
		return UProjectSinfulAscensionCunningCardWidget::StaticClass();
	}
	if (AttributeName == FName(TEXT("Celerity")))
	{
		return UProjectSinfulAscensionCelerityCardWidget::StaticClass();
	}
	if (AttributeName == FName(TEXT("Allure")))
	{
		return UProjectSinfulAscensionAllureCardWidget::StaticClass();
	}

	return UProjectSinfulAscensionAttributeCardWidget::StaticClass();
}
