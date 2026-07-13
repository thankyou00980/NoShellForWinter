#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EFProjectAscensionSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "EF Project Ascension"))
class EFPROJECTSYSTEMSCORE_API UEFProjectAscensionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UEFProjectAscensionSettings();

	static const UEFProjectAscensionSettings* Get();

	virtual FName GetCategoryName() const override;

	UPROPERTY(EditAnywhere, Config, Category = "Progression")
	bool bEnableEternalSinModeByDefault;

	UPROPERTY(EditAnywhere, Config, Category = "Progression", meta = (ClampMin = "0"))
	int32 DefaultMetaSxpGrant;
};
