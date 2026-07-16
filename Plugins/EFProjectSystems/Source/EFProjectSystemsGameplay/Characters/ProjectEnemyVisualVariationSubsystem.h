#pragma once

#include "CoreMinimal.h"
#include "EFCharacterCreationTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/ObjectKey.h"
#include "ProjectEnemyVisualVariationSubsystem.generated.h"

class AActor;
class APawn;
class UEFCharacterCustomizationComponent;
class UProjectCombatAttributeComponent;

struct FProjectEnemyArousalMorphState
{
	TWeakObjectPtr<APawn> Pawn;
	TWeakObjectPtr<UEFCharacterCustomizationComponent> CustomizationComponent;
	TWeakObjectPtr<UProjectCombatAttributeComponent> CombatComponent;
	TArray<FMorphSliderEntry> FlaccidEntries;
	TArray<FMorphSliderEntry> ErectionEntries;
	float CurrentArousalAlpha = 0.0f;
	bool bShouldBeAroused = false;
};

UCLASS()
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectEnemyVisualVariationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void ProcessExistingPawns();
	void HandleActorSpawned(AActor* SpawnedActor);
	void QueueVariationApplication(APawn* Pawn, int32 AttemptIndex);
	void TryApplyVariation(TWeakObjectPtr<APawn> PawnPtr, int32 AttemptIndex);
	bool ShouldProcessPawn(const APawn* Pawn) const;
	bool IsTargetEnemyClass(const UClass* ActorClass) const;
	bool IsMaleMorphTargetClass(const UClass* ActorClass) const;
	UEFCharacterCustomizationComponent* FindOrCreateCustomizationComponent(APawn* Pawn) const;
	bool GatherAllowedMorphEntries(const UEFCharacterCustomizationComponent* CustomizationComponent, TArray<FMorphSliderEntry>& OutEntries) const;
	bool GatherMorphEntriesForConfiguredName(
		const UEFCharacterCustomizationComponent* CustomizationComponent,
		FName MorphName,
		TArray<FMorphSliderEntry>& OutEntries) const;
	void ApplyConfiguredMaleMorphGroups(APawn* Pawn, UEFCharacterCustomizationComponent* CustomizationComponent) const;
	int32 ResolveEnemyLevel(const APawn* Pawn) const;
	bool ApplyMorphEntriesWithValue(
		UEFCharacterCustomizationComponent* CustomizationComponent,
		const TArray<FMorphSliderEntry>& Entries,
		float Value) const;
	void InitializeArousalMorphState(APawn* Pawn, UEFCharacterCustomizationComponent* CustomizationComponent);
	void UpdateSightTriggeredArousal(float CurrentTimeSeconds);
	void AdvanceArousalMorphs(float DeltaTime);
	bool IsPawnDead(const APawn* Pawn, const UProjectCombatAttributeComponent* CombatComponent) const;
	bool CanPawnSeeTrackedPlayer(const APawn* EnemyPawn, const APawn* PlayerPawn) const;
	void ApplyArousalMorphState(FProjectEnemyArousalMorphState& MorphState, float ArousalAlpha) const;
	void CleanupTrackedArousalStates();
	void MarkActorProcessed(const AActor* Actor);
	bool IsActorProcessed(const AActor* Actor) const;
	bool IsActorPending(const AActor* Actor) const;
	void MarkActorPending(const AActor* Actor);
	void ClearActorPending(const AActor* Actor);

private:
	FDelegateHandle ActorSpawnedHandle;
	TArray<TSubclassOf<APawn>> TargetEnemyClasses;
	TArray<TSubclassOf<APawn>> MaleMorphTargetEnemyClasses;
	TSet<FName> AllowedMorphNameSet;
	TSet<TObjectKey<UObject>> ProcessedActors;
	TSet<TObjectKey<UObject>> PendingActors;
	TMap<TObjectKey<APawn>, FProjectEnemyArousalMorphState> TrackedArousalStates;
	bool bInitialPawnScanPending = true;
	double LastSightPollTimeSeconds = -1.0;
};
