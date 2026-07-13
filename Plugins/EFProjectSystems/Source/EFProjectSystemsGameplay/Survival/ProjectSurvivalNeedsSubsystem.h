#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "SinfulAscension/ProjectSinfulAscensionTypes.h"
#include "Survival/ProjectRealtimeSnapshotComponent.h"
#include "ProjectSurvivalNeedsSubsystem.generated.h"

UCLASS()
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSurvivalNeedsSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override;
	virtual bool IsTickableWhenPaused() const override;

	UFUNCTION(BlueprintCallable, Category = "Survival|UI")
	bool SetNeedsHudVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Survival|UI")
	bool ToggleNeedsHudVisibility();

	UFUNCTION(BlueprintPure, Category = "Survival|UI")
	bool IsNeedsHudVisible() const;

	UFUNCTION(BlueprintPure, Category = "Survival|UI")
	class UProjectSinfulAscensionWidget* GetTrackedSinfulAscensionWidget() const;

protected:
	UFUNCTION()
	void HandleTrackedPawnChanged(class APawn* OldPawn, class APawn* NewPawn);

	UFUNCTION()
	void HandleSurvivalValueChanged(FName EntryName, float OldValue, float NewValue, float MaxValue, bool bIsSensation);

	UFUNCTION()
	void HandlePenaltyMultiplierChanged(float OldPenaltyMultiplier, float NewPenaltyMultiplier);

	UFUNCTION()
	void HandleStatusChanged(FName StatusName, bool bActive);

	UFUNCTION()
	void HandleBlackoutChanged(bool bBlackoutActive);

	UFUNCTION()
	void HandleSinfulAscensionSxpChanged(int32 OldCurrentRunSxp, int32 NewCurrentRunSxp, int32 OldMetaBankSxp, int32 NewMetaBankSxp);

	UFUNCTION()
	void HandleSinAttributeLevelChanged(EProjectSinAttribute Attribute, int32 OldLevel, int32 NewLevel, int32 NextLevelCost);

	UFUNCTION()
	void HandleSinMilestoneTriggered(FName AbilityId, EProjectSinAttribute Attribute, int32 Level);

	UFUNCTION()
	void HandleSinSensoryOverloadChanged(bool bOverloadActive, float ProgressSeconds);

	UFUNCTION()
	void HandleUnifiedRuntimeSnapshotChanged(const FProjectUnifiedRuntimeSnapshot& Snapshot);

	virtual void TryResolveRuntimeContext();
	virtual void AttachToPlayerController(class APlayerController* PlayerController);
	virtual void DetachFromTrackedPlayerController(bool bRemoveWidgets);
	virtual void BindInputToTrackedPlayerController();
	virtual void UnbindInputFromTrackedPlayerController();
	virtual void BindToTrackedComponents();
	virtual void UnbindFromTrackedComponents();
	virtual void RefreshNeedsWidget(bool bForceVisibleRefresh);
	virtual void RefreshStatusWidget(bool bForceVisibleRefresh);
	virtual void RefreshSinfulAscensionWidget(bool bForceVisibleRefresh);
	virtual void HandleNeedsHudTogglePressed();
	virtual void HandleDebugStatusCyclePressed();
	virtual void MarkMaintenanceRequired();
	virtual void EnsureNeedsComponentOnPlayerPawn();
	virtual void EnsureNeedsHudWidget(class APlayerController* PlayerController);
	virtual void EnsureStatusHudWidget(class APlayerController* PlayerController);
	virtual void EnsureSinfulAscensionHudWidget(class APlayerController* PlayerController);

private:
	UPROPERTY(Transient)
	TObjectPtr<class UProjectSurvivalNeedsComponent> TrackedNeedsComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectCombatAttributeComponent> TrackedCombatAttributeComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectSurvivalAttributeBridgeComponent> TrackedAttributeBridgeComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectRealtimeSnapshotComponent> TrackedRealtimeSnapshotComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectSurvivalNeedsWidget> TrackedNeedsWidget;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectSurvivalStatusComponent> TrackedStatusComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectSurvivalStatusWidget> TrackedStatusWidget;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectSinfulAscensionComponent> TrackedSinfulAscensionComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectSinfulAscensionWidget> TrackedSinfulAscensionWidget;

	UPROPERTY(Transient)
	TObjectPtr<class APlayerController> TrackedPlayerController;

	UPROPERTY(Transient)
	TObjectPtr<class UInputComponent> TrackedInputComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectSurvivalNeedsComponent> BoundNeedsComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectSurvivalStatusComponent> BoundStatusComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectSinfulAscensionComponent> BoundSinfulAscensionComponent;

	UPROPERTY(Transient)
	TObjectPtr<class UProjectRealtimeSnapshotComponent> BoundRealtimeSnapshotComponent;

	bool bNeedsHudVisible = false;
	bool bNeedsMaintenanceTick = true;
};
