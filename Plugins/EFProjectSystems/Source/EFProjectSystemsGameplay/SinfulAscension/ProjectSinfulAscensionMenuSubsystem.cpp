#include "SinfulAscension/ProjectSinfulAscensionMenuSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "EFProjectUISettings.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SinfulAscension/ProjectSinfulAscensionComponent.h"
#include "SinfulAscension/ProjectSinfulAscensionExchangeMenuWidget.h"
#include "SinfulAscension/ProjectSinfulAscensionSettings.h"
#include "Survival/ProjectSurvivalNeedsSubsystem.h"
#include "TimerManager.h"
#include "UI/ProjectWidgetClassResolver.h"

DEFINE_LOG_CATEGORY_STATIC(LogProjectSinfulAscensionMenu, Log, All);

namespace ProjectSinfulAscensionMenuSubsystemPrivate
{
	constexpr int32 DefaultMenuZOrder = 285;

	struct FSetHudEnabledParams
	{
		bool bEnabled = false;
	};
}

void UProjectSinfulAscensionMenuSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TrackedPlayerController = nullptr;
	TrackedPlayerPawn = nullptr;
	TrackedSinfulAscensionComponent = nullptr;
	BoundSinfulAscensionComponent = nullptr;
	TrackedExchangeMenuWidget = nullptr;
	MenuStateSnapshot = FProjectSinfulExchangeMenuStateSnapshot();
	bMenuOpen = false;
}

void UProjectSinfulAscensionMenuSubsystem::Deinitialize()
{
	CloseMenu();
	UnbindFromTrackedComponent();
	DetachFromTrackedPlayerController(true);
	Super::Deinitialize();
}

void UProjectSinfulAscensionMenuSubsystem::Tick(float DeltaTime)
{
	TryResolveRuntimeContext();

	if (IsGamePauseBlockingMenu() && bMenuOpen)
	{
		CloseMenu();
	}
}

TStatId UProjectSinfulAscensionMenuSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UProjectSinfulAscensionMenuSubsystem, STATGROUP_Tickables);
}

bool UProjectSinfulAscensionMenuSubsystem::IsTickable() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->IsGameWorld();
}

bool UProjectSinfulAscensionMenuSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UProjectSinfulAscensionMenuSubsystem::OpenMenuForActor(AActor* InteractingActor)
{
	if (IsGamePauseBlockingMenu())
	{
		return false;
	}

	TryResolveRuntimeContext();
	if (!ResolveLocalInteractionContext(InteractingActor))
	{
		return false;
	}

	EnsureSinfulAscensionComponent(TrackedPlayerPawn);
	BindToTrackedComponent();
	EnsureExchangeMenuWidget(TrackedPlayerController);
	if (!TrackedPlayerController || !TrackedExchangeMenuWidget || !TrackedSinfulAscensionComponent)
	{
		return false;
	}

	if (!TrackedExchangeMenuWidget->IsInViewport())
	{
		const int32 MenuZOrder = UProjectSinfulAscensionSettings::Get()
			? UProjectSinfulAscensionSettings::Get()->SxpExchangeMenuZOrder
			: ProjectSinfulAscensionMenuSubsystemPrivate::DefaultMenuZOrder;
		if (!TrackedExchangeMenuWidget->AddToPlayerScreen(MenuZOrder))
		{
			TrackedExchangeMenuWidget->AddToViewport(MenuZOrder);
		}
	}

	TrackedExchangeMenuWidget->SetSinfulAscensionComponent(TrackedSinfulAscensionComponent);
	TrackedExchangeMenuWidget->RefreshDisplay();

	if (!bMenuOpen)
	{
		ApplyMenuInputCapture();
		bMenuOpen = true;
	}

	RefreshMenuFocusNextTick();
	return true;
}

void UProjectSinfulAscensionMenuSubsystem::CloseMenu()
{
	if (TrackedExchangeMenuWidget)
	{
		TrackedExchangeMenuWidget->RemoveFromParent();
	}

	if (bMenuOpen)
	{
		RestoreMenuInputCapture();
	}

	bMenuOpen = false;
}

bool UProjectSinfulAscensionMenuSubsystem::IsMenuOpen() const
{
	return bMenuOpen;
}

bool UProjectSinfulAscensionMenuSubsystem::HasTrackedExchangeMenuWidget() const
{
	return TrackedExchangeMenuWidget != nullptr;
}

bool UProjectSinfulAscensionMenuSubsystem::IsTrackedExchangeMenuWidgetInViewport() const
{
	return TrackedExchangeMenuWidget != nullptr && TrackedExchangeMenuWidget->IsInViewport();
}

FString UProjectSinfulAscensionMenuSubsystem::GetTrackedExchangeMenuWidgetClassName() const
{
	return TrackedExchangeMenuWidget ? GetNameSafe(TrackedExchangeMenuWidget->GetClass()) : FString();
}

int32 UProjectSinfulAscensionMenuSubsystem::GetTrackedExchangeMenuSelectedIndex() const
{
	return TrackedExchangeMenuWidget ? TrackedExchangeMenuWidget->GetSelectedIndex() : INDEX_NONE;
}

FProjectSinfulAscensionSnapshot UProjectSinfulAscensionMenuSubsystem::GetTrackedExchangeMenuSnapshot() const
{
	return TrackedExchangeMenuWidget ? TrackedExchangeMenuWidget->GetCachedSnapshot() : FProjectSinfulAscensionSnapshot();
}

void UProjectSinfulAscensionMenuSubsystem::HandleTrackedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (bMenuOpen)
	{
		CloseMenu();
	}

	TrackedPlayerPawn = NewPawn;
	TrackedSinfulAscensionComponent = nullptr;
	BindToTrackedComponent();
	EnsureSinfulAscensionComponent(NewPawn);
}

void UProjectSinfulAscensionMenuSubsystem::HandleSxpChanged(int32 OldCurrentRunSxp, int32 NewCurrentRunSxp, int32 OldMetaBankSxp, int32 NewMetaBankSxp)
{
	RefreshMenuDisplay();
}

void UProjectSinfulAscensionMenuSubsystem::HandleAttributeLevelChanged(EProjectSinAttribute Attribute, int32 OldLevel, int32 NewLevel, int32 NextLevelCost)
{
	RefreshMenuDisplay();
}

void UProjectSinfulAscensionMenuSubsystem::HandleMilestoneTriggered(FName AbilityId, EProjectSinAttribute Attribute, int32 Level)
{
	RefreshMenuDisplay();
}

void UProjectSinfulAscensionMenuSubsystem::HandleSensoryOverloadChanged(bool bOverloadActive, float ProgressSeconds)
{
	RefreshMenuDisplay();
}

void UProjectSinfulAscensionMenuSubsystem::TryResolveRuntimeContext()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		DetachFromTrackedPlayerController(true);
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		DetachFromTrackedPlayerController(true);
		return;
	}

	AttachToPlayerController(PlayerController);
	EnsureSinfulAscensionComponent(PlayerController->GetPawn());
	BindToTrackedComponent();
}

void UProjectSinfulAscensionMenuSubsystem::AttachToPlayerController(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	if (TrackedPlayerController == PlayerController)
	{
		if (TrackedPlayerPawn != PlayerController->GetPawn())
		{
			HandleTrackedPawnChanged(TrackedPlayerPawn, PlayerController->GetPawn());
		}
		return;
	}

	DetachFromTrackedPlayerController(true);

	TrackedPlayerController = PlayerController;
	TrackedPlayerPawn = PlayerController->GetPawn();
	TrackedPlayerController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::HandleTrackedPawnChanged);
}

void UProjectSinfulAscensionMenuSubsystem::DetachFromTrackedPlayerController(const bool bRemoveWidget)
{
	if (bMenuOpen)
	{
		RestoreMenuInputCapture();
		bMenuOpen = false;
	}

	if (TrackedPlayerController)
	{
		TrackedPlayerController->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandleTrackedPawnChanged);
	}

	UnbindFromTrackedComponent();

	if (bRemoveWidget && TrackedExchangeMenuWidget)
	{
		TrackedExchangeMenuWidget->RemoveFromParent();
		TrackedExchangeMenuWidget = nullptr;
	}

	TrackedPlayerController = nullptr;
	TrackedPlayerPawn = nullptr;
	TrackedSinfulAscensionComponent = nullptr;
	MenuStateSnapshot = FProjectSinfulExchangeMenuStateSnapshot();
}

void UProjectSinfulAscensionMenuSubsystem::EnsureSinfulAscensionComponent(APawn* Pawn)
{
	if (!Pawn)
	{
		TrackedPlayerPawn = nullptr;
		TrackedSinfulAscensionComponent = nullptr;
		BindToTrackedComponent();
		return;
	}

	TrackedPlayerPawn = Pawn;
	UProjectSinfulAscensionComponent* Component = Pawn->FindComponentByClass<UProjectSinfulAscensionComponent>();
	if (!Component)
	{
		Component = NewObject<UProjectSinfulAscensionComponent>(Pawn, UProjectSinfulAscensionComponent::StaticClass(), TEXT("ProjectSinfulAscensionComponent"));
		if (Component)
		{
			Pawn->AddInstanceComponent(Component);
			Component->OnComponentCreated();
			Component->RegisterComponent();
			Component->Activate(true);
		}
	}

	TrackedSinfulAscensionComponent = Component;
	BindToTrackedComponent();
}

void UProjectSinfulAscensionMenuSubsystem::EnsureExchangeMenuWidget(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	if (TrackedExchangeMenuWidget)
	{
		return;
	}

	const TSubclassOf<UProjectSinfulAscensionExchangeMenuWidget> ExchangeMenuWidgetClass = ResolveExchangeMenuWidgetClass();
	TrackedExchangeMenuWidget = CreateWidget<UProjectSinfulAscensionExchangeMenuWidget>(
		PlayerController,
		ExchangeMenuWidgetClass);
	if (!TrackedExchangeMenuWidget)
	{
		UE_LOG(LogProjectSinfulAscensionMenu, Warning, TEXT("[ProjectSinfulAscensionMenu] Failed to create exchange widget for %s"), *GetNameSafe(PlayerController));
		return;
	}

	TrackedExchangeMenuWidget->OnPurchaseRequested.AddUObject(this, &ThisClass::HandlePurchaseRequested);
	TrackedExchangeMenuWidget->OnWithdrawRequested.AddUObject(this, &ThisClass::HandleWithdrawRequested);
	TrackedExchangeMenuWidget->OnCloseRequested.AddUObject(this, &ThisClass::HandleCloseRequested);
}

TSubclassOf<UProjectSinfulAscensionExchangeMenuWidget> UProjectSinfulAscensionMenuSubsystem::ResolveExchangeMenuWidgetClass() const
{
	FSoftClassPath ConfiguredWidgetClass;
	if (const UEFProjectUISettings* UISettings = UEFProjectUISettings::Get())
	{
		ConfiguredWidgetClass = UISettings->SinfulAscensionExchangeMenuWidgetClass;
	}

	if (UClass* ResolvedClass = ProjectWidgetClassResolver::ResolveWidgetClassWithPriority(
		ConfiguredWidgetClass,
		UProjectSinfulAscensionExchangeMenuWidget::StaticClass(),
		TEXT("ProjectSinfulAscensionAltarExchangeMenu"),
		TEXT("SinfulAscensionAltar")))
	{
		if (ResolvedClass->IsChildOf(UProjectSinfulAscensionExchangeMenuWidget::StaticClass()))
		{
			return ResolvedClass;
		}
	}

	return UProjectSinfulAscensionExchangeMenuWidget::StaticClass();
}

void UProjectSinfulAscensionMenuSubsystem::BindToTrackedComponent()
{
	if (BoundSinfulAscensionComponent == TrackedSinfulAscensionComponent)
	{
		return;
	}

	UnbindFromTrackedComponent();

	BoundSinfulAscensionComponent = TrackedSinfulAscensionComponent;
	if (!BoundSinfulAscensionComponent)
	{
		return;
	}

	BoundSinfulAscensionComponent->OnSxpChanged.AddUniqueDynamic(this, &ThisClass::HandleSxpChanged);
	BoundSinfulAscensionComponent->OnAttributeLevelChanged.AddUniqueDynamic(this, &ThisClass::HandleAttributeLevelChanged);
	BoundSinfulAscensionComponent->OnMilestoneTriggered.AddUniqueDynamic(this, &ThisClass::HandleMilestoneTriggered);
	BoundSinfulAscensionComponent->OnSensoryOverloadChanged.AddUniqueDynamic(this, &ThisClass::HandleSensoryOverloadChanged);

	if (TrackedExchangeMenuWidget)
	{
		TrackedExchangeMenuWidget->SetSinfulAscensionComponent(BoundSinfulAscensionComponent);
	}
}

void UProjectSinfulAscensionMenuSubsystem::UnbindFromTrackedComponent()
{
	if (!BoundSinfulAscensionComponent)
	{
		return;
	}

	BoundSinfulAscensionComponent->OnSxpChanged.RemoveDynamic(this, &ThisClass::HandleSxpChanged);
	BoundSinfulAscensionComponent->OnAttributeLevelChanged.RemoveDynamic(this, &ThisClass::HandleAttributeLevelChanged);
	BoundSinfulAscensionComponent->OnMilestoneTriggered.RemoveDynamic(this, &ThisClass::HandleMilestoneTriggered);
	BoundSinfulAscensionComponent->OnSensoryOverloadChanged.RemoveDynamic(this, &ThisClass::HandleSensoryOverloadChanged);
	BoundSinfulAscensionComponent = nullptr;
}

void UProjectSinfulAscensionMenuSubsystem::ApplyMenuInputCapture()
{
	MenuStateSnapshot = FProjectSinfulExchangeMenuStateSnapshot();
	ApplyMenuHudSuppression();

	if (TrackedPlayerController)
	{
		MenuStateSnapshot.bHasSavedControllerState = true;
		MenuStateSnapshot.bWasMoveInputIgnored = TrackedPlayerController->IsMoveInputIgnored();
		MenuStateSnapshot.bWasLookInputIgnored = TrackedPlayerController->IsLookInputIgnored();

		TrackedPlayerController->FlushPressedKeys();

		if (!MenuStateSnapshot.bWasMoveInputIgnored)
		{
			TrackedPlayerController->SetIgnoreMoveInput(true);
			MenuStateSnapshot.bAppliedMoveInputIgnore = true;
		}

		if (!MenuStateSnapshot.bWasLookInputIgnored)
		{
			TrackedPlayerController->SetIgnoreLookInput(true);
			MenuStateSnapshot.bAppliedLookInputIgnore = true;
		}

		TrackedPlayerController->DisableInput(TrackedPlayerController);
		TrackedPlayerController->bShowMouseCursor = false;
		TrackedPlayerController->bEnableClickEvents = false;
		TrackedPlayerController->bEnableMouseOverEvents = false;

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TrackedExchangeMenuWidget ? TrackedExchangeMenuWidget->TakeWidget() : TSharedPtr<SWidget>());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		TrackedPlayerController->SetInputMode(InputMode);
	}

	if (TrackedPlayerPawn)
	{
		if (ACharacter* Character = Cast<ACharacter>(TrackedPlayerPawn))
		{
			Character->StopJumping();
			if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
			{
				MenuStateSnapshot.bHasSavedMovementState = true;
				MenuStateSnapshot.PreviousMovementMode = CharacterMovement->MovementMode;
				MenuStateSnapshot.PreviousCustomMovementMode = CharacterMovement->CustomMovementMode;
				CharacterMovement->StopMovementImmediately();
				CharacterMovement->DisableMovement();
			}
		}

		if (TrackedPlayerController)
		{
			TrackedPlayerPawn->DisableInput(TrackedPlayerController);
			MenuStateSnapshot.bPawnInputSuspended = true;
		}
	}
}

void UProjectSinfulAscensionMenuSubsystem::RestoreMenuInputCapture()
{
	RestoreMenuHudSuppression();

	if (TrackedPlayerPawn && MenuStateSnapshot.bPawnInputSuspended)
	{
		if (APlayerController* OwningController = Cast<APlayerController>(TrackedPlayerPawn->GetController()))
		{
			TrackedPlayerPawn->EnableInput(OwningController);
		}
		else if (TrackedPlayerController)
		{
			TrackedPlayerPawn->EnableInput(TrackedPlayerController);
		}
	}

	if (TrackedPlayerPawn && MenuStateSnapshot.bHasSavedMovementState)
	{
		if (ACharacter* Character = Cast<ACharacter>(TrackedPlayerPawn))
		{
			if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
			{
				CharacterMovement->SetMovementMode(MenuStateSnapshot.PreviousMovementMode, MenuStateSnapshot.PreviousCustomMovementMode);
			}
		}
	}

	if (TrackedPlayerController && MenuStateSnapshot.bHasSavedControllerState)
	{
		TrackedPlayerController->EnableInput(TrackedPlayerController);

		if (MenuStateSnapshot.bAppliedMoveInputIgnore)
		{
			TrackedPlayerController->SetIgnoreMoveInput(false);
		}

		if (MenuStateSnapshot.bAppliedLookInputIgnore)
		{
			TrackedPlayerController->SetIgnoreLookInput(false);
		}

		TrackedPlayerController->bShowMouseCursor = false;
		TrackedPlayerController->bEnableClickEvents = false;
		TrackedPlayerController->bEnableMouseOverEvents = false;

		FInputModeGameOnly InputMode;
		TrackedPlayerController->SetInputMode(InputMode);
		TrackedPlayerController->FlushPressedKeys();
	}

	MenuStateSnapshot = FProjectSinfulExchangeMenuStateSnapshot();
}

void UProjectSinfulAscensionMenuSubsystem::ApplyMenuHudSuppression()
{
	if (UWorld* World = GetWorld())
	{
		if (UProjectSurvivalNeedsSubsystem* NeedsSubsystem = World->GetSubsystem<UProjectSurvivalNeedsSubsystem>())
		{
			MenuStateSnapshot.bHasSavedProjectHudVisibility = true;
			MenuStateSnapshot.bWasProjectHudVisible = NeedsSubsystem->IsNeedsHudVisible();
			NeedsSubsystem->SetNeedsHudVisible(false);
		}
	}

	if (!TrackedPlayerController)
	{
		return;
	}

	if (AHUD* HudActor = TrackedPlayerController->GetHUD())
	{
		MenuStateSnapshot.bHasSavedPlayerHudVisibility = true;
		MenuStateSnapshot.bWasPlayerHudVisible = HudActor->bShowHUD;
		HudActor->bShowHUD = false;
		MenuStateSnapshot.bAppliedAcfHudDisable = TrySetReflectedHudEnabled(HudActor, false);
	}
}

void UProjectSinfulAscensionMenuSubsystem::RestoreMenuHudSuppression()
{
	if (TrackedPlayerController && MenuStateSnapshot.bHasSavedPlayerHudVisibility)
	{
		if (AHUD* HudActor = TrackedPlayerController->GetHUD())
		{
			HudActor->bShowHUD = MenuStateSnapshot.bWasPlayerHudVisible;

			if (MenuStateSnapshot.bAppliedAcfHudDisable)
			{
				TrySetReflectedHudEnabled(HudActor, MenuStateSnapshot.bWasPlayerHudVisible);
			}
		}
	}

	if (MenuStateSnapshot.bHasSavedProjectHudVisibility)
	{
		if (UWorld* World = GetWorld())
		{
			if (UProjectSurvivalNeedsSubsystem* NeedsSubsystem = World->GetSubsystem<UProjectSurvivalNeedsSubsystem>())
			{
				NeedsSubsystem->SetNeedsHudVisible(MenuStateSnapshot.bWasProjectHudVisible);
			}
		}
	}
}

bool UProjectSinfulAscensionMenuSubsystem::TrySetReflectedHudEnabled(AHUD* HudActor, const bool bEnabled) const
{
	if (!HudActor)
	{
		return false;
	}

	UFunction* SetHudEnabledFunction = HudActor->FindFunction(TEXT("SetHudEnabled"));
	if (!SetHudEnabledFunction)
	{
		return false;
	}

	ProjectSinfulAscensionMenuSubsystemPrivate::FSetHudEnabledParams Parameters;
	Parameters.bEnabled = bEnabled;
	HudActor->ProcessEvent(SetHudEnabledFunction, &Parameters);
	return true;
}

void UProjectSinfulAscensionMenuSubsystem::RefreshMenuFocusNextTick()
{
	if (!TrackedExchangeMenuWidget)
	{
		return;
	}

	TrackedExchangeMenuWidget->FocusMenuWidget();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (bMenuOpen && TrackedExchangeMenuWidget)
			{
				TrackedExchangeMenuWidget->FocusMenuWidget();
			}
		}));
	}
}

void UProjectSinfulAscensionMenuSubsystem::RefreshMenuDisplay()
{
	if (!TrackedExchangeMenuWidget)
	{
		return;
	}

	TrackedExchangeMenuWidget->SetSinfulAscensionComponent(TrackedSinfulAscensionComponent);
	TrackedExchangeMenuWidget->RefreshDisplay();
}

bool UProjectSinfulAscensionMenuSubsystem::ResolveLocalInteractionContext(AActor* InteractingActor)
{
	if (!InteractingActor)
	{
		return TrackedPlayerController != nullptr && TrackedPlayerPawn != nullptr;
	}

	APawn* CandidatePawn = Cast<APawn>(InteractingActor);
	if (!CandidatePawn)
	{
		if (const AController* CandidateController = Cast<AController>(InteractingActor))
		{
			CandidatePawn = CandidateController->GetPawn();
		}
	}

	if (!CandidatePawn)
	{
		return TrackedPlayerController != nullptr && TrackedPlayerPawn != nullptr;
	}

	if (CandidatePawn == TrackedPlayerPawn)
	{
		return true;
	}

	if (APlayerController* CandidatePlayerController = Cast<APlayerController>(CandidatePawn->GetController()))
	{
		if (CandidatePlayerController->IsLocalController())
		{
			AttachToPlayerController(CandidatePlayerController);
			EnsureSinfulAscensionComponent(CandidatePawn);
			return true;
		}
	}

	return false;
}

bool UProjectSinfulAscensionMenuSubsystem::IsGamePauseBlockingMenu() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->IsGameWorld() && UGameplayStatics::IsGamePaused(World);
}

void UProjectSinfulAscensionMenuSubsystem::HandlePurchaseRequested(const EProjectSinAttribute Attribute)
{
	if (!TrackedSinfulAscensionComponent)
	{
		return;
	}

	TrackedSinfulAscensionComponent->SpendSxpOnAttribute(Attribute);
	RefreshMenuDisplay();
}

void UProjectSinfulAscensionMenuSubsystem::HandleWithdrawRequested()
{
	if (!TrackedSinfulAscensionComponent)
	{
		return;
	}

	TrackedSinfulAscensionComponent->WithdrawMetaSxp(TrackedSinfulAscensionComponent->GetMetaBankSxp());
	RefreshMenuDisplay();
}

void UProjectSinfulAscensionMenuSubsystem::HandleCloseRequested()
{
	CloseMenu();
}
