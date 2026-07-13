#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/ProjectEnemyCombatStatTypes.h"
#include "ProjectSocialCardWidget.generated.h"

class SProjectSocialCardPanel;

UCLASS()
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSocialCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UProjectSocialCardWidget(const FObjectInitializer& ObjectInitializer);

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void NativeConstruct() override;

	void SetSocialCardSnapshot(const FProjectSocialCardSnapshot& InSnapshot);
	void SetHudVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "Project|SocialCard")
	bool IsHudVisible() const;

private:
	void ApplyCachedSnapshot();
	void ApplyFixedViewportPlacement();

private:
	FProjectSocialCardSnapshot CachedSnapshot;

	bool bHudVisible = false;

	TSharedPtr<SProjectSocialCardPanel> SocialCardPanel;
};
