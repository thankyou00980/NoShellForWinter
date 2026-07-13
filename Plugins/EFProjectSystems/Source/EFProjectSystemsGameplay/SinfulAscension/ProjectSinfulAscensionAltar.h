#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SinfulAscension/ProjectSinfulAscensionTypes.h"
#include "ProjectSinfulAscensionAltar.generated.h"

class UProjectSinfulAscensionComponent;
class APawn;

UCLASS(BlueprintType, Blueprintable)
class EFPROJECTSYSTEMSGAMEPLAY_API AProjectSinfulAscensionAltar : public AActor
{
	GENERATED_BODY()

public:
	AProjectSinfulAscensionAltar();

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Altar")
	bool SpendSxpOnAttribute(AActor* InteractingActor, EProjectSinAttribute Attribute) const;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Altar")
	int32 WithdrawMetaSxp(AActor* InteractingActor, int32 RequestedAmount) const;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Altar")
	bool SetEternalSinMode(AActor* InteractingActor, bool bEnabled) const;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Altar")
	UProjectSinfulAscensionComponent* ResolveSinfulAscensionComponent(AActor* InteractingActor) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Sinful Ascension|Altar")
	void OnInteractedByPawn(APawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Altar")
	bool OpenExchangeMenu(AActor* InteractingActor) const;
};
