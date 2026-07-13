#include "SinfulAscension/ProjectSinfulAscensionAltar.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "SinfulAscension/ProjectSinfulAscensionBlueprintLibrary.h"
#include "SinfulAscension/ProjectSinfulAscensionComponent.h"

AProjectSinfulAscensionAltar::AProjectSinfulAscensionAltar()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AProjectSinfulAscensionAltar::SpendSxpOnAttribute(AActor* InteractingActor, const EProjectSinAttribute Attribute) const
{
	UProjectSinfulAscensionComponent* Component = ResolveSinfulAscensionComponent(InteractingActor);
	return Component ? Component->SpendSxpOnAttribute(Attribute) : false;
}

int32 AProjectSinfulAscensionAltar::WithdrawMetaSxp(AActor* InteractingActor, const int32 RequestedAmount) const
{
	UProjectSinfulAscensionComponent* Component = ResolveSinfulAscensionComponent(InteractingActor);
	return Component ? Component->WithdrawMetaSxp(RequestedAmount) : 0;
}

bool AProjectSinfulAscensionAltar::SetEternalSinMode(AActor* InteractingActor, const bool bEnabled) const
{
	UProjectSinfulAscensionComponent* Component = ResolveSinfulAscensionComponent(InteractingActor);
	if (!Component)
	{
		return false;
	}

	Component->SetEternalSinMode(bEnabled);
	return true;
}

UProjectSinfulAscensionComponent* AProjectSinfulAscensionAltar::ResolveSinfulAscensionComponent(AActor* InteractingActor) const
{
	if (!InteractingActor)
	{
		return nullptr;
	}

	if (UProjectSinfulAscensionComponent* DirectComponent = InteractingActor->FindComponentByClass<UProjectSinfulAscensionComponent>())
	{
		return DirectComponent;
	}

	if (const AController* Controller = Cast<AController>(InteractingActor))
	{
		if (APawn* Pawn = Controller->GetPawn())
		{
			return Pawn->FindComponentByClass<UProjectSinfulAscensionComponent>();
		}
	}

	if (const APawn* Pawn = Cast<APawn>(InteractingActor))
	{
		if (AController* Controller = Pawn->GetController())
		{
			return Controller->FindComponentByClass<UProjectSinfulAscensionComponent>();
		}
	}

	return nullptr;
}

void AProjectSinfulAscensionAltar::OnInteractedByPawn_Implementation(APawn* Pawn)
{
	OpenExchangeMenu(Pawn);
}

bool AProjectSinfulAscensionAltar::OpenExchangeMenu(AActor* InteractingActor) const
{
	return UProjectSinfulAscensionBlueprintLibrary::OpenSinfulSxpExchangeMenu(InteractingActor);
}
