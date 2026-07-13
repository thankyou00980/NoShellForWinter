#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SinfulAscension/ProjectSinfulAscensionTypes.h"
#include "ProjectSinfulAscensionBlueprintLibrary.generated.h"

class UProjectSinfulAscensionComponent;

UCLASS()
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension", meta = (DefaultToSelf = "Actor"))
	static UProjectSinfulAscensionComponent* FindSinfulAscensionComponent(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension", meta = (DefaultToSelf = "Actor"))
	static int32 GrantSinfulSxp(AActor* Actor, FName ReasonId, int32 Amount, bool bSinfulEvent);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension", meta = (DefaultToSelf = "Actor"))
	static bool SpendSinfulSxpOnAttribute(AActor* Actor, EProjectSinAttribute Attribute);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension", meta = (DefaultToSelf = "Actor"))
	static int32 WithdrawSinfulMetaSxp(AActor* Actor, int32 RequestedAmount);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension", meta = (DefaultToSelf = "Actor"))
	static void NotifySinfulInteraction(AActor* Actor, FName InteractionId);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI", meta = (DefaultToSelf = "Actor"))
	static bool OpenSinfulSxpExchangeMenu(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI", meta = (DefaultToSelf = "Actor"))
	static void CloseSinfulSxpExchangeMenu(AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI", meta = (DefaultToSelf = "Actor"))
	static bool IsSinfulSxpExchangeMenuOpen(AActor* Actor);
};
