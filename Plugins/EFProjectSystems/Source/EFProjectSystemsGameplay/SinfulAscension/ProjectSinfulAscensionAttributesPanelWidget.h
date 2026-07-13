#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeWidgetDesignerTreeProvider.h"
#include "ProjectSinfulAscensionAttributesPanelWidget.generated.h"

class UBorder;
class UHorizontalBox;
class UImage;
class UOverlay;
class UScaleBox;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UVerticalBox;
class UWidgetTree;
class UWrapBox;
class UObject;
class UProjectSinfulAscensionAttributeCardsGlobalWidget;

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionAttributesPanelWidget : public UUserWidget, public ICodeWidgetDesignerTreeProvider
{
	GENERATED_BODY()

public:
	UProjectSinfulAscensionAttributesPanelWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual bool BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree) override;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	void ApplyPanelState(bool bEternalSinMode, int32 CurrentRunSxp, int32 MetaBankSxp, float PanelHeight, int32 CardCount);

	UWrapBox* GetAttributesWrapBox() const { return AttributesWrapBox; }
	UProjectSinfulAscensionAttributeCardsGlobalWidget* GetAttributeCardsGlobalWidget() const { return AttributeCardsGlobal; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Sinful Ascension|UI")
	void OnAttributesPanelStateApplied(bool bEternalSinMode, int32 CurrentRunSxp, int32 MetaBankSxp, float PanelHeight, int32 CardCount);

protected:
	void BuildWidgetTree();
	bool BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree);
	void InitializeVisualTree();
	void RefreshPanelBrush();
	void RefreshSummaryBrushes();
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
	TSoftObjectPtr<UTexture2D> FrameTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> HazeTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> DividerTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> HudPillTexture;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> RootSizeBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> FrameOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> FrameTextureImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> HazeImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> ContentBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ContentBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> HeaderOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> HudPillOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> HudPillImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ModeText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> TopDividerImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> SummaryRow;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> RunSummaryBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RunSummaryLabelText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UScaleBox> RunSummaryValueScaleBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RunSummaryValueText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> MetaSummaryBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MetaSummaryLabelText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UScaleBox> MetaSummaryValueScaleBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MetaSummaryValueText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWrapBox> AttributesWrapBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionAttributeCardsGlobalWidget> AttributeCardsGlobal;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> BottomDividerImage;

private:
	float CurrentPanelHeight = 206.0f;
	int32 CurrentCardCount = 0;
	bool bVisualTreeInitialized = false;
	bool bUsingNativeFallbackTree = false;
};
