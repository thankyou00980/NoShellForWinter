#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodeWidgetDesignerTreeProvider.h"
#include "ProjectSinfulAscensionAttributeCardWidget.generated.h"

class UBorder;
class UHorizontalBox;
class UImage;
class UOverlay;
class UScaleBox;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UWidget;
class UWidgetTree;
class UObject;
class UVerticalBox;
class UWrapBox;

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectSinfulAscensionAttributeCardDisplayData
{
	GENERATED_BODY()

	FProjectSinfulAscensionAttributeCardDisplayData()
		: AttributeName(NAME_None)
		, DisplayLabel()
		, ShortLabel()
		, IconTexture(nullptr)
		, AccentTint(FLinearColor(0.92f, 0.40f, 0.72f, 1.0f))
		, Level(0)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FName AttributeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FString DisplayLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FString ShortLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	TSoftObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FLinearColor AccentTint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	int32 Level;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionAttributeCardWidget : public UUserWidget, public ICodeWidgetDesignerTreeProvider
{
	GENERATED_BODY()

public:
	UProjectSinfulAscensionAttributeCardWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual bool BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree) override;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	void ApplyDisplayData(const FProjectSinfulAscensionAttributeCardDisplayData& InDisplayData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Sinful Ascension|UI")
	void OnSinfulAttributeCardDataApplied(const FProjectSinfulAscensionAttributeCardDisplayData& DisplayData);

protected:
	void BuildWidgetTree();
	bool BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree);
	void InitializeVisualTree();
	void RefreshVisuals();
	void RefreshCardBrush(const FLinearColor& AccentTint);
	virtual FProjectSinfulAscensionAttributeCardDisplayData MakeDesignerPreviewData() const;
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
	TSoftObjectPtr<UTexture2D> CardFrameTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Textures")
	TSoftObjectPtr<UTexture2D> DefaultIconTexture;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> RootSizeBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> RootOverlay;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> BaseBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> FrameImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> ContentBorder;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ContentBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> IconSizeBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ShortLabelText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UScaleBox> ValueScaleBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ValueText;

private:
	FProjectSinfulAscensionAttributeCardDisplayData CurrentDisplayData;
	bool bVisualTreeInitialized = false;
	bool bUsingNativeFallbackTree = false;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionAttributeCardGlobalWidget : public UProjectSinfulAscensionAttributeCardWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionAttributeCardDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionAttributeCardsGlobalWidget : public UUserWidget, public ICodeWidgetDesignerTreeProvider
{
	GENERATED_BODY()

public:
	UProjectSinfulAscensionAttributeCardsGlobalWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual bool BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree) override;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	int32 ApplyCards(
		const TArray<FProjectSinfulAscensionAttributeCardDisplayData>& InCardData,
		TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> FallbackCardWidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	UProjectSinfulAscensionAttributeCardWidget* FindCardWidgetByAttribute(FName AttributeName) const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI")
	int32 GetVisibleCardCount() const { return VisibleCardCount; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Sinful Ascension|UI")
	void OnSinfulAttributeCardsGlobalApplied(int32 InVisibleCardCount);

protected:
	void BuildWidgetTree();
	bool BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree);
	UProjectSinfulAscensionAttributeCardWidget* GetFixedCardForAttribute(FName AttributeName) const;
	UProjectSinfulAscensionAttributeCardWidget* GetOrCreateFallbackCard(
		FName AttributeName,
		TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> FallbackCardWidgetClass);
	void HideUnusedFixedCards(const TSet<FName>& VisibleFixedCardNames);
	void ClearFallbackCards();

protected:
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> RootSizeBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> CardsBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionAttributeCardWidget> WillpowerCard;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionAttributeCardWidget> SadismCard;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionAttributeCardWidget> MasochismCard;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionAttributeCardWidget> FaithCard;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionAttributeCardWidget> CunningCard;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionAttributeCardWidget> CelerityCard;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionAttributeCardWidget> AllureCard;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWrapBox> ExtraCardsWrapBox;

private:
	TMap<FName, TObjectPtr<UProjectSinfulAscensionAttributeCardWidget>> RuntimeCardsByName;
	TMap<FName, TObjectPtr<UProjectSinfulAscensionAttributeCardWidget>> FallbackCardsByName;
	int32 VisibleCardCount = 0;
	bool bUsingNativeFallbackTree = false;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionWillpowerCardWidget : public UProjectSinfulAscensionAttributeCardWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionAttributeCardDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionSadismCardWidget : public UProjectSinfulAscensionAttributeCardWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionAttributeCardDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionMasochismCardWidget : public UProjectSinfulAscensionAttributeCardWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionAttributeCardDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionFaithCardWidget : public UProjectSinfulAscensionAttributeCardWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionAttributeCardDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionCunningCardWidget : public UProjectSinfulAscensionAttributeCardWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionAttributeCardDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionCelerityCardWidget : public UProjectSinfulAscensionAttributeCardWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionAttributeCardDisplayData MakeDesignerPreviewData() const override;
};

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionAllureCardWidget : public UProjectSinfulAscensionAttributeCardWidget
{
	GENERATED_BODY()

protected:
	virtual FProjectSinfulAscensionAttributeCardDisplayData MakeDesignerPreviewData() const override;
};
