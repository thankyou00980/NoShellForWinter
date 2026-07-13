#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ProjectSinfulAscensionEditorLibrary.generated.h"

class UBlueprint;

UCLASS()
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Editor")
	static bool ConfigureBlueprintAsSxpAltar(UBlueprint* Blueprint);
};
