#include "Characters/ProjectPlayerMorphPhysicsSubsystem.h"

#include "EFMorphPhysicsConstraintComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogProjectPlayerMorphPhysics, Log, All);

void UProjectPlayerMorphPhysicsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TrackedPlayerController = nullptr;
	TrackedPawn = nullptr;
	TrackedMorphPhysicsComponent = nullptr;
	bNeedsMaintenanceTick = true;
}

void UProjectPlayerMorphPhysicsSubsystem::Deinitialize()
{
	DetachFromTrackedPlayerController();
	bNeedsMaintenanceTick = false;

	Super::Deinitialize();
}

void UProjectPlayerMorphPhysicsSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	(void)DeltaTime;

	if (!bNeedsMaintenanceTick)
	{
		return;
	}

	bNeedsMaintenanceTick = !TryResolveRuntimeContext();
}

TStatId UProjectPlayerMorphPhysicsSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UProjectPlayerMorphPhysicsSubsystem, STATGROUP_Tickables);
}

bool UProjectPlayerMorphPhysicsSubsystem::IsTickable() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->IsGameWorld() && bNeedsMaintenanceTick;
}

bool UProjectPlayerMorphPhysicsSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UProjectPlayerMorphPhysicsSubsystem::TryResolveRuntimeContext()
{
	UWorld* World = GetWorld();
	if (!World || World->IsNetMode(NM_DedicatedServer))
	{
		DetachFromTrackedPlayerController();
		return false;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		DetachFromTrackedPlayerController();
		return false;
	}

	AttachToPlayerController(PlayerController);
	EnsureMorphPhysicsComponent(PlayerController->GetPawn());

	return TrackedPlayerController != nullptr
		&& TrackedPawn != nullptr
		&& TrackedMorphPhysicsComponent != nullptr;
}

void UProjectPlayerMorphPhysicsSubsystem::AttachToPlayerController(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	if (TrackedPlayerController == PlayerController)
	{
		return;
	}

	DetachFromTrackedPlayerController();
	TrackedPlayerController = PlayerController;
	TrackedPawn = PlayerController->GetPawn();
	TrackedPlayerController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::HandlePossessedPawnChanged);
}

void UProjectPlayerMorphPhysicsSubsystem::DetachFromTrackedPlayerController()
{
	if (TrackedPlayerController)
	{
		TrackedPlayerController->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandlePossessedPawnChanged);
	}

	TrackedMorphPhysicsComponent = nullptr;
	TrackedPawn = nullptr;
	TrackedPlayerController = nullptr;
}

void UProjectPlayerMorphPhysicsSubsystem::EnsureMorphPhysicsComponent(APawn* Pawn)
{
	if (!Pawn)
	{
		TrackedPawn = nullptr;
		TrackedMorphPhysicsComponent = nullptr;
		return;
	}

	TrackedPawn = Pawn;
	UEFMorphPhysicsConstraintComponent* MorphPhysicsComponent = Pawn->FindComponentByClass<UEFMorphPhysicsConstraintComponent>();
	if (!MorphPhysicsComponent)
	{
		MorphPhysicsComponent = NewObject<UEFMorphPhysicsConstraintComponent>(
			Pawn,
			UEFMorphPhysicsConstraintComponent::StaticClass(),
			TEXT("ProjectMorphPhysicsConstraintComponent"));
		if (MorphPhysicsComponent)
		{
			Pawn->AddInstanceComponent(MorphPhysicsComponent);
			MorphPhysicsComponent->OnComponentCreated();
			MorphPhysicsComponent->RegisterComponent();
			MorphPhysicsComponent->Activate(true);

			UE_LOG(
				LogProjectPlayerMorphPhysics,
				Log,
				TEXT("Attached morph physics driver to pawn %s."),
				*GetNameSafe(Pawn));
		}
	}
	else
	{
		if (!MorphPhysicsComponent->IsRegistered())
		{
			MorphPhysicsComponent->RegisterComponent();
		}

		if (!MorphPhysicsComponent->IsActive())
		{
			MorphPhysicsComponent->Activate(true);
		}
	}

	TrackedMorphPhysicsComponent = MorphPhysicsComponent;
	if (TrackedMorphPhysicsComponent)
	{
		TrackedMorphPhysicsComponent->RefreshFromCurrentMorphState();
	}
	else
	{
		UE_LOG(
			LogProjectPlayerMorphPhysics,
			Warning,
			TEXT("Could not attach morph physics driver to pawn %s."),
			*GetNameSafe(Pawn));
	}
}

void UProjectPlayerMorphPhysicsSubsystem::MarkMaintenanceRequired()
{
	bNeedsMaintenanceTick = true;
}

void UProjectPlayerMorphPhysicsSubsystem::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	(void)OldPawn;

	TrackedPawn = NewPawn;
	TrackedMorphPhysicsComponent = nullptr;
	MarkMaintenanceRequired();
}
