#include "SinfulAscension/ProjectSinfulAscensionBlueprintLibrary.h"

#include "Engine/World.h"
#include "SinfulAscension/ProjectSinfulAscensionComponent.h"
#include "SinfulAscension/ProjectSinfulAscensionMenuSubsystem.h"

UProjectSinfulAscensionComponent* UProjectSinfulAscensionBlueprintLibrary::FindSinfulAscensionComponent(AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UProjectSinfulAscensionComponent>() : nullptr;
}

int32 UProjectSinfulAscensionBlueprintLibrary::GrantSinfulSxp(AActor* Actor, const FName ReasonId, const int32 Amount, const bool bSinfulEvent)
{
	UProjectSinfulAscensionComponent* Component = FindSinfulAscensionComponent(Actor);
	return Component ? Component->GrantSxp(ReasonId, Amount, bSinfulEvent) : 0;
}

bool UProjectSinfulAscensionBlueprintLibrary::SpendSinfulSxpOnAttribute(AActor* Actor, const EProjectSinAttribute Attribute)
{
	UProjectSinfulAscensionComponent* Component = FindSinfulAscensionComponent(Actor);
	return Component ? Component->SpendSxpOnAttribute(Attribute) : false;
}

int32 UProjectSinfulAscensionBlueprintLibrary::WithdrawSinfulMetaSxp(AActor* Actor, const int32 RequestedAmount)
{
	UProjectSinfulAscensionComponent* Component = FindSinfulAscensionComponent(Actor);
	return Component ? Component->WithdrawMetaSxp(RequestedAmount) : 0;
}

void UProjectSinfulAscensionBlueprintLibrary::NotifySinfulInteraction(AActor* Actor, const FName InteractionId)
{
	if (UProjectSinfulAscensionComponent* Component = FindSinfulAscensionComponent(Actor))
	{
		Component->NotifySinfulInteraction(InteractionId);
	}
}

bool UProjectSinfulAscensionBlueprintLibrary::OpenSinfulSxpExchangeMenu(AActor* Actor)
{
	UWorld* World = Actor ? Actor->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	if (UProjectSinfulAscensionMenuSubsystem* MenuSubsystem = World->GetSubsystem<UProjectSinfulAscensionMenuSubsystem>())
	{
		return MenuSubsystem->OpenMenuForActor(Actor);
	}

	return false;
}

void UProjectSinfulAscensionBlueprintLibrary::CloseSinfulSxpExchangeMenu(AActor* Actor)
{
	UWorld* World = Actor ? Actor->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	if (UProjectSinfulAscensionMenuSubsystem* MenuSubsystem = World->GetSubsystem<UProjectSinfulAscensionMenuSubsystem>())
	{
		MenuSubsystem->CloseMenu();
	}
}

bool UProjectSinfulAscensionBlueprintLibrary::IsSinfulSxpExchangeMenuOpen(AActor* Actor)
{
	UWorld* World = Actor ? Actor->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	if (const UProjectSinfulAscensionMenuSubsystem* MenuSubsystem = World->GetSubsystem<UProjectSinfulAscensionMenuSubsystem>())
	{
		return MenuSubsystem->IsMenuOpen();
	}

	return false;
}
