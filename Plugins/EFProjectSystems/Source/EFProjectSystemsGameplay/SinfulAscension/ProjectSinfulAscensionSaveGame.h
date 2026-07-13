#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ProjectSinfulAscensionSaveGame.generated.h"

UCLASS()
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Sinful Ascension")
	int32 MetaBankSxp = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Sinful Ascension")
	bool bEternalSinMode = false;
};
