#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EFProjectEnemySettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "EF Project Enemies"))
class EFPROJECTSYSTEMSCORE_API UEFProjectEnemySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UEFProjectEnemySettings();

	static const UEFProjectEnemySettings* Get();

	virtual FName GetCategoryName() const override;

	UPROPERTY(EditAnywhere, Config, Category = "Enemy Classes")
	TArray<FSoftClassPath> RuntimeEnemyClasses;

	UPROPERTY(EditAnywhere, Config, Category = "Target HUD")
	bool bEnableExtendedTargetStatsWhenNeedsHudVisible;
};
