#pragma once

#include "Blueprint/UserWidget.h"
#include "ProjectTargetPointWidget.generated.h"

UCLASS()
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectTargetPointWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UProjectTargetPointWidget(const FObjectInitializer& ObjectInitializer);

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void NativeConstruct() override;

	void SetOverlayVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "Project|Target")
	bool IsOverlayVisible() const;

private:
	bool bOverlayVisible = false;
};
