#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeWidgetDesignerTreeProvider.h"
#include "ProjectSinfulAscensionExchangeAttributeRowWidget.generated.h"

class UBorder;
class UHorizontalBox;
class UImage;
class UOverlay;
class UScrollBox;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UVerticalBox;
class UWidgetTree;
class UObject;

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectSinfulAscensionExchangeAttributeRowDisplayData
{
	GENERATED_BODY()

	FProjectSinfulAscensionExchangeAttributeRowDisplayData()
		: AttributeName(NAME_None)
		, DisplayLabel()
		, IconTexture(nullptr)
		, AccentTint(FLinearColor(0.92f, 0.40f, 0.72f, 1.0f))
		, Level(0)
		, Cost(0)
		, RowWidth(560.0f)
		, RowHeight(88.0f)
		, bSelected(false)
		, bAffordable(false)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FName AttributeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FString DisplayLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	TSoftObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	FLinearColor AccentTint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	int32 Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	int32 Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	float RowWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	float RowHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	bool bSelected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension|UI")
	bool bAffordable;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeAttributeRowWidget : public UUserWidget, public ICodeWidgetDesignerTreeProvider
{
	GENERATED_BODY()

public:
	UProjectSinfulAscensionExchangeAttributeRowWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual bool BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree) override;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	void ApplyDisplayData(const FProjectSinfulAscensionExchangeAttributeRowDisplayData& InDisplayData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Sinful Ascension|UI")
	void OnExchangeAttributeRowDataApplied(const FProjectSinfulAscensionExchangeAttributeRowDisplayData& DisplayData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Sinful Ascension|UI")
	void OnExchangeAttributeRowVisualStateChanged(FName AttributeName, bool bSelected, bool bAffordable);

protected:
	void BuildWidgetTree();
	bool BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree);
	void InitializeVisualTree();
	void RefreshVisuals();
	virtual FProjectSinfulAscensionExchangeAttributeRowDisplayData MakeDesignerPreviewData() const;
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
	TSoftObjectPtr<UTexture2D> RowFrameTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> DefaultIconTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> SelectionGlyphTexture;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> RootSizeBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> DesignerRootOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> RootOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> BackgroundBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> FrameImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> ContentBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> ContentBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> IconSizeBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> IconBackgroundBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> TextColumn;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> MetaRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SeparatorText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CostText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SelectionGlyphSizeBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> SelectionGlyphImage;

private:
	FProjectSinfulAscensionExchangeAttributeRowDisplayData CurrentDisplayData;
	bool bVisualTreeInitialized = false;
	bool bUsingNativeFallbackTree = false;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeAttributeRowGlobalWidget : public UProjectSinfulAscensionExchangeAttributeRowWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionExchangeAttributeRowDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeWillpowerRowWidget : public UProjectSinfulAscensionExchangeAttributeRowWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionExchangeAttributeRowDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeSadismRowWidget : public UProjectSinfulAscensionExchangeAttributeRowWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionExchangeAttributeRowDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeMasochismRowWidget : public UProjectSinfulAscensionExchangeAttributeRowWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionExchangeAttributeRowDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeFaithRowWidget : public UProjectSinfulAscensionExchangeAttributeRowWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionExchangeAttributeRowDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeCunningRowWidget : public UProjectSinfulAscensionExchangeAttributeRowWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionExchangeAttributeRowDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeCelerityRowWidget : public UProjectSinfulAscensionExchangeAttributeRowWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionExchangeAttributeRowDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeAllureRowWidget : public UProjectSinfulAscensionExchangeAttributeRowWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionExchangeAttributeRowDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionExchangeRowsGlobalWidget : public UUserWidget, public ICodeWidgetDesignerTreeProvider
{
	GENERATED_BODY()

public:
	UProjectSinfulAscensionExchangeRowsGlobalWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual bool BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree) override;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	int32 ApplyRows(
		const TArray<FProjectSinfulAscensionExchangeAttributeRowDisplayData>& InRowData,
		TSubclassOf<UProjectSinfulAscensionExchangeAttributeRowWidget> FallbackRowWidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	UProjectSinfulAscensionExchangeAttributeRowWidget* FindRowWidgetByAttribute(FName AttributeName) const;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	void ScrollRowWidgetIntoView(UProjectSinfulAscensionExchangeAttributeRowWidget* RowWidget);

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI")
	int32 GetVisibleRowCount() const { return VisibleRowCount; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Sinful Ascension|UI")
	void OnExchangeRowsApplied(int32 InVisibleRowCount, int32 SelectedIndex);

protected:
	void BuildWidgetTree();
	bool BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree);
	UProjectSinfulAscensionExchangeAttributeRowWidget* GetFixedRowForAttribute(FName AttributeName) const;
	UProjectSinfulAscensionExchangeAttributeRowWidget* GetOrCreateFallbackRow(
		FName AttributeName,
		TSubclassOf<UProjectSinfulAscensionExchangeAttributeRowWidget> FallbackRowWidgetClass);
	void HideUnusedFixedRows(const TSet<FName>& VisibleFixedRowNames);
	void ClearFallbackRows();

protected:
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> RootSizeBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> DesignerRootOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> RootOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> RowsScrollBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> RowsLayout;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget> WillpowerRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget> SadismRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget> MasochismRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget> FaithRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget> CunningRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget> CelerityRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget> AllureRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ExtraRowsLayout;

private:
	TMap<FName, TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget>> RuntimeRowsByName;
	TMap<FName, TObjectPtr<UProjectSinfulAscensionExchangeAttributeRowWidget>> FallbackRowsByName;
	int32 VisibleRowCount = 0;
	bool bUsingNativeFallbackTree = false;
};
