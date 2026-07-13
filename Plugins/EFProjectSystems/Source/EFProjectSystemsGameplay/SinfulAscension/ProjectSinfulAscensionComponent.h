#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/ProjectCombatTypes.h"
#include "SinfulAscension/ProjectSinfulAscensionTypes.h"
#include "Survival/ProjectRealtimeSnapshotComponent.h"
#include "TimerManager.h"
#include "ProjectSinfulAscensionComponent.generated.h"

class UACFDamageHandlerComponent;
class UACFEffectsManagerComponent;
class UACMCollisionManagerComponent;
class UProjectCombatAttributeComponent;
class UProjectDirtyPawnEffectsBridgeComponent;
class UProjectDefeatFlowComponent;
class UProjectEmoteComponent;
class UProjectLocomotionOverrideComponent;
class UProjectRealtimeSnapshotComponent;
class UProjectSinfulAscensionSettings;
class UProjectSurvivalNeedsComponent;
class UProjectSurvivalStatusComponent;
struct FACFDamageEvent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FProjectSinfulAscensionSxpChangedSignature, int32, OldCurrentRunSxp, int32, NewCurrentRunSxp, int32, OldMetaBankSxp, int32, NewMetaBankSxp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FProjectSinAttributeLevelChangedSignature, EProjectSinAttribute, Attribute, int32, OldLevel, int32, NewLevel, int32, NextLevelCost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FProjectSinMilestoneTriggeredSignature, FName, AbilityId, EProjectSinAttribute, Attribute, int32, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FProjectSinSensoryOverloadChangedSignature, bool, bOverloadActive, float, ProgressSeconds);

UCLASS(ClassGroup = (SinfulAscension), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProjectSinfulAscensionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension")
	int32 GetCurrentRunSxp() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension")
	int32 GetMetaBankSxp() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension")
	bool IsEternalSinModeEnabled() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension")
	int32 GetAttributeLevel(EProjectSinAttribute Attribute) const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension")
	int32 GetUpgradeCost(EProjectSinAttribute Attribute) const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension")
	FProjectSinfulAscensionSnapshot BuildSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|Movement")
	float GetEffectiveNormalMoveSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|Movement")
	float GetEffectiveMoveSpeedBonus() const;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	int32 GrantSxp(FName ReasonId, int32 Amount, bool bSinfulEvent);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	int32 GrantSxpWithAttributeAffinity(FName ReasonId, int32 Amount, bool bSinfulEvent, const TArray<EProjectSinAttribute>& Affinities);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	bool SpendSxpOnAttribute(EProjectSinAttribute Attribute);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	bool ApplyFreeAttributeLevels(EProjectSinAttribute Attribute, int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	void ResetRunProgressForBackgroundChange();

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	void SetSXPGainMultipliers(const TMap<EProjectSinAttribute, float>& InMultipliers);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	void ClearSXPGainMultipliers();

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|External Effects")
	void SetExternalSXPGainMultiplier(FName SourceId, float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|External Effects")
	void ClearExternalSXPGainMultiplier(FName SourceId);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|External Effects")
	void SetExternalFlatDamageBonus(FName SourceId, float FlatDamageBonus);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|External Effects")
	void ClearExternalFlatDamageBonus(FName SourceId);

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension")
	float GetSXPGainMultiplierForAttribute(EProjectSinAttribute Attribute) const;

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	int32 WithdrawMetaSxp(int32 RequestedAmount);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	void HandleRunDeath();

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	void SetEternalSinMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension")
	bool ActivateMilestoneAbility(FName AbilityId);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Sensations")
	void GrantSensation(FName ReasonId, float MadnessDelta, float LustDelta, float PainDelta);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Hooks")
	void NotifyNudityStateChanged(bool bIsNude);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Hooks")
	void NotifySinfulInteraction(FName InteractionId);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Hooks")
	void NotifyCorruptObject(FName ObjectId);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Hooks")
	void NotifyForbiddenFood(FName ItemId);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Hooks")
	void NotifyTaggedConsumable(FName ItemId);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Hooks")
	bool NotifyDungeonFloorCompleted(FName FloorTransitionId = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Hooks")
	bool NotifySleepCompleted(FName SleepSourceId = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Sinful Ascension|Masochism")
	bool TryActivateSurrender();

	UFUNCTION(BlueprintPure, Category = "Sinful Ascension|Masochism")
	bool IsMasochismPainRaptureActive() const;

#if WITH_DEV_AUTOMATION_TESTS
	int32 AutomationGrantYMenuActionSxp(FName ReasonId, int32 Amount, bool bSinfulEvent);
	void AutomationApplyFaithMadnessRecovery(float DeltaTime);
	void AutomationQueueMasochismPainReflection(float PainAppliedDelta);
	void AutomationFlushMasochismPainReflection();
	float AutomationGetPendingMasochismPainReflectionLust() const;
#endif

	float ModifyIncomingHealthDamage(AActor* SourceActor, const FName DamageType, float PostArmorDamage, float& OutFlatNegatedDamage, float& OutRaptureAbsorbedDamage);
	bool TryDeferPainKnockout();
	bool TryPreventDeathFromDamage(const FProjectIncomingHitContext& HitContext, float CurrentHealth, float& OutSurvivingHealth);
	void ModifyOutgoingDamageSpec(AActor* TargetActor, FProjectCombatDamageSpec& InOutDamageSpec);
	void NotifyDamageDealt(AActor* TargetActor, const FProjectCombatDamageResult& DamageResult);
	void NotifyDamageReceived(const FProjectIncomingHitContext& HitContext);

	UPROPERTY(BlueprintAssignable, Category = "Sinful Ascension")
	FProjectSinfulAscensionSxpChangedSignature OnSxpChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sinful Ascension")
	FProjectSinAttributeLevelChangedSignature OnAttributeLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sinful Ascension")
	FProjectSinMilestoneTriggeredSignature OnMilestoneTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Sinful Ascension")
	FProjectSinSensoryOverloadChangedSignature OnSensoryOverloadChanged;

private:
	enum class ESxpGrantSource : uint8
	{
		Standard,
		YMenuAction
	};

	UFUNCTION()
	void HandleBlackoutChanged(bool bBlackoutActive);

	UFUNCTION()
	void HandleRealtimeCombatImpact(const FProjectRealtimeCombatImpact& Impact);

	UFUNCTION()
	void HandleMasochismRaptureAcfActorDamaged(AActor* DamageReceiver);

	UFUNCTION()
	void HandleMasochismRaptureAcfCollisionDetected(const FHitResult& HitResult);

	void RefreshCachedComponents();
	void EnsureDirtyPawnEffectsBridge();
	void InitializeAttributeStorage();
	void LoadPersistentState();
	void SavePersistentState() const;
	int32 GrantSxpInternal(FName ReasonId, int32 Amount, bool bSinfulEvent, ESxpGrantSource Source);
	int32 GrantSxpWithAttributeAffinityInternal(FName ReasonId, int32 Amount, bool bSinfulEvent, const TArray<EProjectSinAttribute>& Affinities, ESxpGrantSource Source);
	float ResolveSxpSourceMultiplier(ESxpGrantSource Source, const UProjectSinfulAscensionSettings* Settings) const;
	float ResolveExternalSxpGainMultiplier() const;
	float ResolveExternalFlatDamageBonus() const;
	int32 CalculateCelerityFlatSxpBonus(const UProjectSinfulAscensionSettings* Settings) const;
	void BroadcastSxpChanged(int32 OldCurrentRunSxp, int32 OldMetaBankSxp);
	void ResetRunAttributes();
	void ApplyPassiveAttributeEffects();
	void ApplyFaithPassiveAttributeEffects();
	void UpdateFaithMadnessRecovery(float DeltaTime);
	void UpdateActiveEmoteEffects(float DeltaTime);
	void UpdateSensationDecay();
	void UpdateSensationConsequences(float DeltaTime);
	void UpdateSensationHoldState();
	void StartMaxSensationHold(FName SensationName, float MaxValue);
	void RefreshMaxSensationHold(FName SensationName);
	void EndSensationHold(FName SensationName);
	void UpdateCombatState();
	void MarkCombatImpact();
	bool TryStartOrRefreshMasochismPainRapture(bool bRefreshHold);
	void MaintainMasochismPainRapture();
	void CompleteMasochismPainRapture();
	void EndMasochismPainRaptureState();
	float ResolveCurrentOwnerHealthForMasochismRapture();
	void RestoreMasochismRaptureHealthFloor();
	void ApplyMasochismRaptureAcfImmortality(bool bEnabled);
	void RegisterMasochismRaptureHitFeedback();
	void RefreshMasochismRaptureHitFeedbackBindings(bool bForce = false);
	void BindMasochismRaptureCollisionManagersOnActor(AActor* Actor);
	void BindMasochismRaptureCollisionManager(UACMCollisionManagerComponent* CollisionManager);
	void UnbindMasochismRaptureHitFeedback();
	void HandleMasochismRaptureActorSpawned(AActor* SpawnedActor);
	void TriggerMasochismRaptureHitFeedback(AActor* SourceActor, const FHitResult* HitResult);
	void PlayMasochismRaptureHitSound(const FACFDamageEvent& DamageEvent);
	void QueueMasochismRaptureDamageTextSuppression();
	void SuppressMasochismRaptureDamageTextWidgets();
	TSubclassOf<class UACFDamageType> ResolveMasochismRaptureFeedbackDamageClass(AActor* SourceActor) const;
	bool TryApplyMasochismPainOverflow();
	void QueueMasochismPainReflection(float PainAppliedDelta);
	void FlushMasochismPainReflection();
	void ApplyWillpowerStatusImmunities();
	bool ApplyWillpowerSecondBreathRecovery();
	float CalculateDesiredMoveSpeedBonus() const;
	float ResolveBaseMoveSpeedForBonuses() const;
	void UpdateMovementBonus();
	void ApplyDynamicMaximums();
	void ApplyDynamicNeedMaximum(FName NeedName, float NewMaxValue);
	void ApplyDynamicSensationMaximum(FName SensationName, float NewMaxValue);
	float CalculateDynamicMaximum(FName EntryName, bool bIsSensation, float BaseMax) const;
	float ResolveBaseNeedMax(FName NeedName) const;
	float ResolveBaseSensationMax(FName SensationName) const;
	void SetForcedStatus(FName StatusName, bool bActive);
	float ApplyResourceDelta(FName ResourceName, float DeltaAmount);
	void ApplyHealthDelta(float DeltaAmount);
	void AddSensationGainTimestamp(FName SensationName, float AppliedDelta);
	void AddMilestoneSnapshot(TArray<FProjectSinMilestoneState>& OutMilestones, FName AbilityId, EProjectSinAttribute Attribute, int32 RequiredLevel) const;
	void BroadcastMilestonesForLevelRange(EProjectSinAttribute Attribute, int32 OldLevel, int32 NewLevel);
	float ResolveBestSXPGainMultiplier(const TArray<EProjectSinAttribute>& Affinities) const;
	void BindToRealtimeSnapshotComponent();
	void UnbindFromRealtimeSnapshotComponent();
	void RewardEnemyKill(AActor* TargetActor);
	void GrantSadismLustFromDamage(AActor* TargetActor, const FProjectCombatDamageResult& DamageResult);
	void GrantSadismExecuteLust(AActor* TargetActor, const FProjectCombatDamageResult& DamageResult);
	void GrantMasochismLustFromQualifiedHit(const FProjectIncomingHitContext& HitContext);
	void TryApplyQualifiedPainFallback(const FProjectRealtimeCombatImpact& Impact);
	bool IsAttributeIndexValid(EProjectSinAttribute Attribute) const;
	bool IsRelevantEnemyActor(const AActor* Actor) const;
	bool IsQualifiedMaleEnemyActor(const AActor* Actor) const;
	bool IsActorMatchingClassHints(const AActor* Actor, const TArray<FString>& Hints) const;
	FText GetAttributeDisplayName(EProjectSinAttribute Attribute) const;
	float GetSensationCurrent(FName SensationName) const;
	float GetSensationMax(FName SensationName) const;
	void SetSensationCurrent(FName SensationName, float NewValue);
	float GetNormalizedSensation(FName SensationName) const;
	bool IsOrgasmStateActive() const;
	bool IsSensationMaxHoldActive(FName SensationName) const;
	bool HasMilestone(EProjectSinAttribute Attribute, int32 Level) const;

	UPROPERTY(Transient)
	TObjectPtr<UProjectSurvivalNeedsComponent> NeedsComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectSurvivalStatusComponent> StatusComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectCombatAttributeComponent> CombatAttributeComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectRealtimeSnapshotComponent> RealtimeSnapshotComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectDefeatFlowComponent> DefeatFlowComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectLocomotionOverrideComponent> LocomotionOverrideComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectEmoteComponent> EmoteComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectDirtyPawnEffectsBridgeComponent> DirtyPawnEffectsBridgeComponent;

	UPROPERTY(Transient)
	TObjectPtr<UACFDamageHandlerComponent> AcfDamageHandlerComponent;

	UPROPERTY(Transient)
	TObjectPtr<UACFEffectsManagerComponent> AcfEffectsManagerComponent;

	UPROPERTY(Transient)
	TArray<int32> AttributeLevels;

	TMap<EProjectSinAttribute, float> SXPGainMultipliersByAttribute;
	TMap<FName, float> ExternalSxpGainMultipliersBySource;
	TMap<FName, float> ExternalFlatDamageBonusesBySource;

	UPROPERTY(Transient)
	int32 CurrentRunSxp = 0;

	UPROPERTY(Transient)
	int32 MetaBankSxp = 0;

	UPROPERTY(Transient)
	bool bEternalSinMode = false;

	TMap<FName, float> LastSensationGainTimeSeconds;
	TMap<FName, float> LastSensationDecayTimeSeconds;
	TMap<FName, float> MaxSensationHoldEndTimeSeconds;
	TMap<FName, float> LastEnemyKillRewardTimesByActorName;
	TArray<TWeakObjectPtr<UACMCollisionManagerComponent>> MasochismRaptureCollisionManagers;
	TMap<FName, float> RuntimeBaseNeedMaxByName;
	TMap<FName, float> RuntimeBaseSensationMaxByName;
	TSet<FName> AppliedSecondBreathTransitionIds;
	FDelegateHandle MasochismRaptureActorSpawnedHandle;
	FTimerHandle MasochismPainReflectionTimerHandle;
	float AccumulatedTickSeconds = 0.f;
	float SensoryOverloadProgressSeconds = 0.f;
	float NudityTickAccumulatorSeconds = 0.f;
	float CachedBaseMaxWalkSpeed = 0.f;
	float AppliedMoveSpeedBonus = 0.f;
	float LastCombatImpactTimeSeconds = -FLT_MAX;
	float LastQualifiedPainGainTimeSeconds = -FLT_MAX;
	float LastMasochismRaptureHitFeedbackTimeSeconds = -FLT_MAX;
	float NextMasochismRaptureFeedbackBindingRefreshTimeSeconds = -FLT_MAX;
	float MasochismRaptureCapturedHealth = 0.f;
	float PendingMasochismPainReflectionLust = 0.f;
	float AppliedFaithSpellDefenseBonus = 0.f;
	bool bHasCachedBaseMaxWalkSpeed = false;
	bool bOverloadBlackoutInProgress = false;
	bool bOverloadProgressBroadcastState = false;
	bool bFrenzyForced = false;
	bool bPainForcedCrawl = false;
	bool bPainForcedWalkWasEnabled = false;
	bool bIsNude = false;
	bool bAllureMarkedInteraction = false;
	bool bCombatStateActive = false;
	bool bMasochismPainRaptureActive = false;
	bool bMasochismRaptureAcfImmortalityApplied = false;
	bool bMasochismRapturePreviousAcfImmortal = false;
};
