#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SinfulAscension/ProjectSinfulAscensionTypes.h"
#include "ProjectSinfulAscensionMenuSubsystem.generated.h"

class AActor;
class APlayerController;
class APawn;
class UProjectSinfulAscensionComponent;
class UProjectSinfulAscensionExchangeMenuWidget;

struct FProjectSinfulExchangeMenuStateSnapshot
{
	bool bHasSavedControllerState = false;
	bool bWasMoveInputIgnored = false;
	bool bWasLookInputIgnored = false;
	bool bAppliedMoveInputIgnore = false;
	bool bAppliedLookInputIgnore = false;
	bool bHasSavedMovementState = false;
	TEnumAsByte<EMovementMode> PreviousMovementMode = MOVE_Walking;
	uint8 PreviousCustomMovementMode = 0;
	bool bPawnInputSuspended = false;
	bool bHasSavedProjectHudVisibility = false;
	bool bWasProjectHudVisible = false;
	bool bHasSavedPlayerHudVisibility = false;
	bool bWasPlayerHudVisible = true;
	bool bAppliedAcfHudDisable = false;
};

UCLASS()
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionMenuSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	bool OpenMenuForActor(AActor* InteractingActor);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|UI")
	void CloseMenu();

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI")
	bool IsMenuOpen() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI|Debug")
	bool HasTrackedExchangeMenuWidget() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI|Debug")
	bool IsTrackedExchangeMenuWidgetInViewport() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI|Debug")
	FString GetTrackedExchangeMenuWidgetClassName() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI|Debug")
	int32 GetTrackedExchangeMenuSelectedIndex() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|UI|Debug")
	FProjectSinfulAscensionSnapshot GetTrackedExchangeMenuSnapshot() const;

protected:
	UFUNCTION()
	void HandleTrackedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void HandleSxpChanged(int32 OldCurrentRunSxp, int32 NewCurrentRunSxp, int32 OldMetaBankSxp, int32 NewMetaBankSxp);

	UFUNCTION()
	void HandleAttributeLevelChanged(EProjectSinAttribute Attribute, int32 OldLevel, int32 NewLevel, int32 NextLevelCost);

	UFUNCTION()
	void HandleMilestoneTriggered(FName AbilityId, EProjectSinAttribute Attribute, int32 Level);

	UFUNCTION()
	void HandleSensoryOverloadChanged(bool bOverloadActive, float ProgressSeconds);

private:
	void TryResolveRuntimeContext();
	void AttachToPlayerController(APlayerController* PlayerController);
	void DetachFromTrackedPlayerController(bool bRemoveWidget);
	void EnsureSinfulAscensionComponent(APawn* Pawn);
	void EnsureExchangeMenuWidget(APlayerController* PlayerController);
	TSubclassOf<UProjectSinfulAscensionExchangeMenuWidget> ResolveExchangeMenuWidgetClass() const;
	void BindToTrackedComponent();
	void UnbindFromTrackedComponent();
	void ApplyMenuInputCapture();
	void RestoreMenuInputCapture();
	void ApplyMenuHudSuppression();
	void RestoreMenuHudSuppression();
	bool TrySetReflectedHudEnabled(class AHUD* HudActor, bool bEnabled) const;
	void RefreshMenuFocusNextTick();
	void RefreshMenuDisplay();
	bool ResolveLocalInteractionContext(AActor* InteractingActor);
	bool IsGamePauseBlockingMenu() const;
	void HandlePurchaseRequested(EProjectSinAttribute Attribute);
	void HandleWithdrawRequested();
	void HandleCloseRequested();

private:
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> TrackedPlayerController;

	UPROPERTY(Transient)
	TObjectPtr<APawn> TrackedPlayerPawn;

	UPROPERTY(Transient)
	TObjectPtr<UProjectSinfulAscensionComponent> TrackedSinfulAscensionComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectSinfulAscensionComponent> BoundSinfulAscensionComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectSinfulAscensionExchangeMenuWidget> TrackedExchangeMenuWidget;

	FProjectSinfulExchangeMenuStateSnapshot MenuStateSnapshot;
	bool bMenuOpen = false;
};
