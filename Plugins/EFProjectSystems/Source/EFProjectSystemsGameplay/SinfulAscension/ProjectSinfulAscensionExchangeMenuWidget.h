#pragma once

#include "Blueprint/UserWidget.h"
#include "CodeWidgetDesignerTreeProvider.h"
#include "InputCoreTypes.h"
#include "SinfulAscension/ProjectSinfulAscensionExchangeAttributeRowWidget.h"
#include "SinfulAscension/ProjectSinfulAscensionTypes.h"
#include "ProjectSinfulAscensionExchangeMenuWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UHorizontalBox;
class UImage;
class UOverlay;
class UProjectSinfulAscensionComponent;
class UProjectSinfulAscensionExchangeAttributeRowWidget;
class UProjectSinfulAscensionExchangeDetailPanelWidget;
class UProjectSinfulAscensionExchangeRowsGlobalWidget;
class UScaleBox;
class UScrollBox;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UVerticalBox;
class UWidgetTree;
class UObject;

DECLARE_MULTICAST_DELEGATE_OneParam(FProjectSinfulExchangePurchaseRequestedEvent, EProjectSinAttribute);
DECLARE_MULTICAST_DELEGATE(FProjectSinfulExchangeWithdrawRequestedEvent);
DECLARE_MULTICAST_DELEGATE(FProjectSinfulExchangeCloseRequestedEvent);

struct FProjectSinfulAscensionExchangeResolvedEntry
{
	FName AttributeName = NAME_None;
	EProjectSinAttribute Attribute = EProjectSinAttribute::Willpower;
	FProjectSinfulAscensionExchangeAttributeRowDisplayData RowData;
	FText DetailName;
	FText DetailDescription;
	FText MilestoneFiveLabel;
	FText MilestoneTenLabel;
	TSoftObjectPtr<UTexture2D> DetailWatermarkTexture;
	int32 SortOrder = 0;
	bool bMilestoneFiveUnlocked = false;
	bool bMilestoneTenUnlocked = false;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectSinfulAscensionExchangeDetailDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FName AttributeName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FText DetailName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FText LevelText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FText CostText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FText MilestoneFiveText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FText MilestoneTenText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FText FlavorText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	TSoftObjectPtr<UTexture2D> WatermarkTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FLinearColor AccentTint = FLinearColor(0.97f, 0.54f, 0.82f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	bool bHasSelection = false;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeMenuWidget : public UUserWidget, public ICodeWidgetDesignerTreeProvider
{
	GENERATED_BODY()

public:
	UProjectSinfulAscensionExchangeMenuWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree) override;
	virtual bool GatherCodeWidgetDesignerConversionManifest(FCodeWidgetDesignerConversionManifest& OutManifest) const override;

	void SetSinfulAscensionComponent(UProjectSinfulAscensionComponent* InComponent);
	void RefreshDisplay();
	void FocusMenuWidget();

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI|Debug")
	int32 GetSelectedIndex() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI|Debug")
	FProjectSinfulAscensionSnapshot GetCachedSnapshot() const;

	FProjectSinfulExchangePurchaseRequestedEvent OnPurchaseRequested;
	FProjectSinfulExchangeWithdrawRequestedEvent OnWithdrawRequested;
	FProjectSinfulExchangeCloseRequestedEvent OnCloseRequested;

	UFUNCTION(BlueprintImplementableEvent, Category = "Sinful Ascension|UI")
	void OnExchangeMenuStateApplied(const FProjectSinfulAscensionSnapshot& Snapshot, int32 InSelectedIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Sinful Ascension|UI")
	void OnExchangeMenuSelectionChanged(int32 InSelectedIndex, FName SelectedAttributeName);

protected:
	void BuildWidgetTree();
	void InitializeVisualTree();
	void RefreshVisualState();
	TArray<FProjectSinfulAscensionExchangeResolvedEntry> BuildResolvedEntries() const;
	TArray<FProjectSinfulAscensionExchangeResolvedEntry> BuildDesignerPreviewEntries() const;
	bool DoesRowLayoutNeedRebuild(const TArray<FProjectSinfulAscensionExchangeResolvedEntry>& InEntries) const;
	void RebuildAttributeRows(const TArray<FProjectSinfulAscensionExchangeResolvedEntry>& InEntries);
	void RefreshAttributeRows();
	void RefreshDetailPanel();
	void RefreshFooterState();
	void NavigateSelectionByDirection(int32 Direction);
	void ConfirmSelection();
	bool HandleMenuKey(const FKey& Key);
	TSubclassOf<UProjectSinfulAscensionExchangeAttributeRowWidget> ResolveRowWidgetClass() const;
	UTexture2D* ResolveTexture(const TSoftObjectPtr<UTexture2D>& AssetPtr, const TCHAR* FallbackPath) const;
	UObject* ResolveStyleAsset(const TSoftObjectPtr<UObject>& AssetPtr, const TCHAR* FallbackPath) const;
	FSlateFontInfo MakeTitleFont(int32 Size, int32 LetterSpacing = 0) const;
	FSlateFontInfo MakeBodyFont(int32 Size, int32 LetterSpacing = 0) const;
	FText BuildAttributeFlavor(EProjectSinAttribute Attribute) const;
	FText BuildMilestoneStatusText(const FText& LabelText, bool bUnlocked) const;
	FProjectSinfulAscensionExchangeDetailDisplayData BuildDetailDisplayData(const FProjectSinfulAscensionExchangeResolvedEntry* SelectedEntry) const;
	const FProjectSinfulAscensionExchangeResolvedEntry* GetSelectedEntry() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Style|Fonts")
	TSoftObjectPtr<UObject> TitleFontAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Fonts")
	TSoftObjectPtr<UObject> BodyFontAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> FrameTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> HazeTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> DividerTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> StatBoxTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> FooterOrnamentTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> ModeGlyphTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> DefaultIconTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> DefaultWatermarkTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Widgets")
	TSoftClassPtr<UProjectSinfulAscensionExchangeAttributeRowWidget> AttributeRowWidgetClass;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> BackdropBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> FrameOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> BackgroundBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> HazeImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> FrameTextureImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> ContentBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ContentBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HeaderRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> ModeBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> ModeGlyphImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ModeText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> TopDividerImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> ResourceRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> RunSxpBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RunSxpLabelText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UScaleBox> RunSxpValueScaleBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RunSxpValueText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> MetaSxpBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MetaSxpLabelText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UScaleBox> MetaSxpValueScaleBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MetaSxpValueText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> BodyRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> AttributeListBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionExchangeRowsGlobalWidget> RowsGlobalWidget;

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> AttributesScrollBox;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> AttributesLayout;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> DetailBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> DetailOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> DetailWatermarkImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> DetailContentBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> DetailContentBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailNameText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailLevelText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailCostText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> DetailMilestoneDividerImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailMilestoneFiveText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailMilestoneTenText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailFlavorText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> FooterDividerImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> FooterRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FooterStatusText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FooterControlsText;

	TMap<FName, TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget>> RowWidgetsByName;
	TArray<FName> CachedRowOrder;
	TArray<FProjectSinfulAscensionExchangeResolvedEntry> ResolvedEntries;
	TWeakObjectPtr<UProjectSinfulAscensionComponent> SinfulAscensionComponent;
	FProjectSinfulAscensionSnapshot CachedSnapshot;
	int32 SelectedIndex = INDEX_NONE;
	bool bVisualTreeInitialized = false;
	bool bUsingNativeFallbackTree = false;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeMenuGlobalWidget : public UProjectSinfulAscensionExchangeMenuWidget
{
	GENERATED_BODY()
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeDetailPanelWidget : public UUserWidget, public ICodeWidgetDesignerTreeProvider
{
	GENERATED_BODY()

public:
	UProjectSinfulAscensionExchangeDetailPanelWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual bool BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree) override;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	void ApplyDetailData(const FProjectSinfulAscensionExchangeDetailDisplayData& InDisplayData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Sinful Ascension|UI")
	void OnExchangeDetailPanelDataApplied(const FProjectSinfulAscensionExchangeDetailDisplayData& DisplayData);

protected:
	void BuildWidgetTree();
	bool BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree);
	void InitializeVisualTree();
	void RefreshVisuals();
	UTexture2D* ResolveTexture(const TSoftObjectPtr<UTexture2D>& AssetPtr, const TCHAR* FallbackPath) const;
	UObject* ResolveStyleAsset(const TSoftObjectPtr<UObject>& AssetPtr, const TCHAR* FallbackPath) const;
	FSlateFontInfo MakeTitleFont(int32 Size, int32 LetterSpacing = 0) const;
	FSlateFontInfo MakeBodyFont(int32 Size, int32 LetterSpacing = 0) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Style|Fonts")
	TSoftObjectPtr<UObject> TitleFontAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Fonts")
	TSoftObjectPtr<UObject> BodyFontAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> DividerTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> DefaultWatermarkTexture;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> DetailBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> DetailOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> DetailWatermarkImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> DetailContentBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> DetailContentBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailNameText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailLevelText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailCostText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> DetailMilestoneDividerImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailMilestoneFiveText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailMilestoneTenText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailFlavorText;

private:
	FProjectSinfulAscensionExchangeDetailDisplayData CurrentDisplayData;
	bool bVisualTreeInitialized = false;
	bool bUsingNativeFallbackTree = false;
};
