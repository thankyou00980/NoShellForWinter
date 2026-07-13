#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/ProjectEnemyCombatStatTypes.h"
#include "ProjectTargetStatsWidget.generated.h"

class SProjectTargetStatsPanel;

UCLASS()
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectTargetStatsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UProjectTargetStatsWidget(const FObjectInitializer& ObjectInitializer);

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void NativeConstruct() override;

	void SetCombatStatSnapshot(const FProjectEnemyCombatStatSnapshot& InSnapshot);
	void SetScreenPosition(const FVector2D& InScreenPosition);
	void SetOverlayVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "Project|Target")
	bool IsOverlayVisible() const;

private:
	void ApplyCachedSnapshot();
	void ApplyFixedViewportPlacement();

private:
	FProjectEnemyCombatStatSnapshot CachedSnapshot;

	bool bOverlayVisible = false;

	TSharedPtr<SProjectTargetStatsPanel> StatsPanel;
};
