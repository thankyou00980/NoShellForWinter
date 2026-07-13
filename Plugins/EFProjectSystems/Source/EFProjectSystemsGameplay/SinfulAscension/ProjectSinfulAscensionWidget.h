#pragma once

#include "Blueprint/UserWidget.h"
#include "CodeWidgetDesignerTreeProvider.h"
#include "SinfulAscension/ProjectSinfulAscensionAttributeCardWidget.h"
#include "SinfulAscension/ProjectSinfulAscensionTypes.h"
#include "ProjectSinfulAscensionWidget.generated.h"

class UCanvasPanel;
class UCanvasPanelSlot;
class UOverlay;
class UProjectSinfulAscensionAttributeCardsGlobalWidget;
class UProjectSinfulAscensionAttributesPanelWidget;
class UProjectSinfulAscensionComponent;
class UWidgetTree;
class UWrapBox;

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionWidget : public UUserWidget, public ICodeWidgetDesignerTreeProvider
{
	GENERATED_BODY()

public:
	UProjectSinfulAscensionWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual bool BuildCodeWidgetDesignerTree(UWidgetTree* TargetWidgetTree) override;
	virtual void GatherCodeWidgetDesignerChildWidgetSpecs(TArray<FCodeWidgetDesignerChildWidgetSpec>& OutWidgetSpecs) const override;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	void SetSinfulAscensionComponent(UProjectSinfulAscensionComponent* InComponent);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	void RefreshDisplay();

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	void SetHudVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI")
	bool IsHudVisible() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI")
	FProjectSinfulAscensionSnapshot GetCachedSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI")
	int32 GetRenderedAttributeCardCount() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Sinful Ascension|UI")
	void OnSinfulAttributesRebuilt(int32 VisibleCardCount);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sinful Ascension|UI")
	int32 ZOrder;

	UPROPERTY(EditDefaultsOnly, Category = "Style|Widgets")
	TSoftClassPtr<UProjectSinfulAscensionAttributeCardWidget> AttributeCardWidgetClass;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> PanelHost;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UProjectSinfulAscensionAttributesPanelWidget> AttributesPanelWidget;

	UPROPERTY(Transient)
	TObjectPtr<UProjectSinfulAscensionComponent> SinfulAscensionComponent;

	UPROPERTY(Transient)
	FProjectSinfulAscensionSnapshot CachedSnapshot;

protected:
	void ApplyHudVisibility();
	void BuildWidgetTree();
	bool BuildDefaultWidgetTree(UWidgetTree* TargetWidgetTree);
	void EnsureAttributesPanelWidget();
	void SyncAttributesContainer();
	void ApplyPanelState(int32 CardCount);
	float CalculatePanelHeight(int32 CardCount) const;
	TArray<FProjectSinfulAscensionAttributeCardDisplayData> BuildResolvedCardData() const;
	bool DoesCardLayoutNeedRebuild(const TArray<FProjectSinfulAscensionAttributeCardDisplayData>& InCardData) const;
	void RebuildAttributeCards(const TArray<FProjectSinfulAscensionAttributeCardDisplayData>& InCardData);
	TSubclassOf<UProjectSinfulAscensionAttributesPanelWidget> ResolveAttributesPanelWidgetClass() const;
	TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> ResolveAttributeCardWidgetClass() const;
	TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> ResolveGlobalAttributeCardWidgetClass() const;
	TSubclassOf<UProjectSinfulAscensionAttributeCardWidget> ResolveAttributeCardWidgetClassForData(const FProjectSinfulAscensionAttributeCardDisplayData& CardData) const;
	UClass* ResolveNativeCardClassForAttribute(FName AttributeName) const;

private:
	UCanvasPanelSlot* PanelHostSlot = nullptr;
	TObjectPtr<UProjectSinfulAscensionAttributeCardsGlobalWidget> AttributesCardsGlobalWidget;
	TObjectPtr<UWrapBox> AttributesWrapBox;
	TMap<FName, TObjectPtr<UProjectSinfulAscensionAttributeCardWidget>> CardWidgetsByName;
	TArray<FName> CachedCardOrder;
	float CurrentPanelHeight = 206.0f;
	bool bHudVisible = false;
	bool bUsingNativeFallbackTree = false;
};
