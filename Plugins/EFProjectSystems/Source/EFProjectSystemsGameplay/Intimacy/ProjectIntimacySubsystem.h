#pragma once

#include "CoreMinimal.h"
#include "Characters/ProjectEnemyCombatStatTypes.h"
#include "InputCoreTypes.h"
#include "Intimacy/ProjectIntimacyTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "ProjectIntimacySubsystem.generated.h"

class APlayerController;
class APawn;
class UDataTable;
class UInputComponent;
class UProjectEmoteComponent;
class UProjectIntimacyHudWidget;
class UProjectIntimacyPartnerComponent;
class UProjectIntimacySaveGame;
class UProjectSinfulAscensionComponent;
class UProjectSurvivalNeedsComponent;
class UProjectSurvivalStatusComponent;
class UProjectTargetingFixComponent;
class USkeletalMeshComponent;

struct FProjectIntimacyResolvedOption
{
	FName OptionId = NAME_None;
	FText Label;
	EProjectIntimacyTalkAction TalkAction = EProjectIntimacyTalkAction::None;
	FName CategoryId = NAME_None;
	FGameplayTagContainer TalkTags;
	float LustDrain = 0.0f;
	int32 SxpReward = 0;
	int32 ControlDelta = 0;
	int32 AffectDelta = 0;
	float AnimationRate = 1.0f;
	bool bRequiresAllyEligibility = false;
	bool bCanBeCorrectTalkOption = true;
	bool bUsesTalkCooldown = true;
	bool bCanBeFlavorCorrectOption = true;
};

struct FProjectIntimacyRuntimeSession
{
	TWeakObjectPtr<AActor> PartnerActor;
	TWeakObjectPtr<UProjectIntimacyPartnerComponent> PartnerComponent;
	FString PartnerId;
	float CurrentLust = 0.0f;
	float MaxLust = 0.0f;
	float LustDrainPerSecond = 0.0f;
	float SessionTimeSeconds = 0.0f;
	float AnimationRate = 1.0f;
	float ClimaxProgress = 0.0f;
	float ClimaxThreshold = 1000.0f;
	float ClimaxMultiplier = 1.0f;
	float ClimaxRecoveryRemaining = 0.0f;
	EProjectIntimacyPersonality EffectivePersonality = EProjectIntimacyPersonality::Nice;
	int32 CurrentControlPoints = 0;
	float TalkCooldownRemaining = 0.0f;
	FName CorrectTalkOptionId = NAME_None;
	float OverpoweringPulseElapsedSeconds = 0.0f;
	EProjectIntimacyHudMode HudMode = EProjectIntimacyHudMode::Main;
	FName ActiveTalkCategoryId = NAME_None;
	FName ActiveItemCategoryId = NAME_None;
	int32 SelectedOptionIndex = 0;
	bool bHudVisible = false;
	bool bSatisfied = false;
	bool bPleaseActive = false;
	int32 PleaseAttemptIndex = 0;
	int32 PleaseSuccessCount = 0;
	float PleaseElapsedSeconds = 0.0f;
	float PleasePulsePeriod = 1.0f;
	float PleaseCursorValue = 0.0f;
	float PleaseTargetCenter = 0.5f;
	float PleaseTargetHalfRange = 0.08f;
	FText StatusText;
};

UCLASS()
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectIntimacySubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static const FName FirstIntimacyHeartChestTattooRewardId;
	static const FName TestTattooIntimacyRewardId;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	bool IsIntimacySessionActive() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	bool IsHudVisible() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	EProjectIntimacyHudMode GetHudMode() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	bool IsPleaseActive() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	FString GetActivePartnerId() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	float GetCurrentEncounterLust() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	int32 GetCurrentControlPoints() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	EProjectIntimacyControlState GetCurrentControlState() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	float GetTalkCooldownRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy|Progress")
	int32 GetTotalIntimacyEncounterCount() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy|Progress")
	bool HasAnyIntimacyEncounter() const;

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy|Progress")
	bool IsAutomaticTattooRewardUnlocked(FName TattooRewardId) const;

	UFUNCTION(BlueprintCallable, Category = "Project|Intimacy|Progress")
	bool UnlockAutomaticTattooReward(FName TattooRewardId);

	UFUNCTION(BlueprintCallable, Category = "Project|Intimacy|Automation")
	bool GrantFirstIntimacyEncounterForAutomation();

	UFUNCTION(BlueprintCallable, Category = "Project|Intimacy|Automation")
	bool ForcePartnerClimaxForAutomation();

	UFUNCTION(BlueprintCallable, Category = "Project|Intimacy")
	void RequestToggleHud();

	UFUNCTION(BlueprintCallable, Category = "Project|Intimacy")
	bool RequestQuickStartIntimacy();

	UFUNCTION(BlueprintCallable, Category = "Project|Intimacy")
	void RequestNavigate(int32 Direction);

	UFUNCTION(BlueprintCallable, Category = "Project|Intimacy")
	void RequestBack();

	UFUNCTION(BlueprintCallable, Category = "Project|Intimacy")
	void RequestConfirm();

	UFUNCTION(BlueprintCallable, Category = "Project|Intimacy")
	void RequestCancelIntimacy();

	UFUNCTION(BlueprintPure, Category = "Project|Intimacy")
	FProjectIntimacySessionSnapshot BuildSnapshot() const;

#if WITH_DEV_AUTOMATION_TESTS
	bool AutomationRunMenuAndPleaseSmoke(
		FString& OutFailureReason,
		int32& OutStepCount,
		bool& bOutPleaseCompleted,
		bool& bOutClimaxTriggered);
#endif

	bool TryGetPartnerProfile(AActor* PartnerActor, FProjectIntimacyPartnerProfile& OutProfile) const;
	bool BuildTargetSocialCardSnapshot(AActor* PartnerActor, FProjectSocialCardSnapshot& OutSnapshot);
	void AppendTargetIntimacyRows(AActor* PartnerActor, FProjectEnemyCombatStatSnapshot& InOutSnapshot);

private:
	void TryResolveRuntimeContext();
	void AttachToPlayerController(APlayerController* PlayerController);
	void DetachFromTrackedPlayerController();
	void BindInputToTrackedPlayerController();
	void UnbindInputFromTrackedPlayerController();
	bool ResolveActiveIntimacyPartner(AActor*& OutPartnerActor, UProjectIntimacyPartnerComponent*& OutPartnerComponent) const;
	void StartSession(AActor* PartnerActor, UProjectIntimacyPartnerComponent* PartnerComponent);
	void UpdateActiveSession(float DeltaTime);
	void EndSession(bool bCancelled);
	void RefreshActiveIntimacyCombatShield();
	void FinishSatisfiedSession();
	void CancelActiveSession();
	void EnsureHudWidget();
	void RefreshHudWidget();
	void RefreshResolvedOptions();
	void ChooseCorrectTalkOption();
	void SetHudMode(EProjectIntimacyHudMode NewMode);
	void HandleMainOption(const FName OptionId);
	void HandleTalkOption(const FProjectIntimacyResolvedOption& Option);
	void HandleItemsOption(const FProjectIntimacyResolvedOption& Option);
	void ExecuteTalkOption(const FProjectIntimacyResolvedOption& Option);
	void TriggerMediaCueForTalkOption(const FProjectIntimacyResolvedOption& Option);
	bool TryResolveMediaCueForTalkOption(const FProjectIntimacyResolvedOption& Option, FProjectIntimacyMediaCueRow& OutCue) const;
	void TriggerMediaCueForEvent(FName EventId);
	bool TryResolveMediaCueForEvent(FName EventId, FProjectIntimacyMediaCueRow& OutCue) const;
	void StartPlease();
	void StartNextPleaseAttempt();
	void ResolvePleasePress();
	void ApplyControlDelta(int32 Delta, const FText& ReasonText);
	void ApplyLustDrain(float Amount, const FText& ReasonText, bool bApplyControlResistance = false);
	void UpdateClimaxIntensity(float DeltaTime);
	void TriggerPartnerClimax(int32 ClimaxCount);
	void GrantPlayerLustFromClimax(int32 ClimaxCount);
	float ComputeEffectiveAnimationRate() const;
	void UpdateOverpoweringPulse(float DeltaTime);
	void ApplyAnimationRate(float NewRate);
	void RestoreAnimationRates();
	void CacheAndSetMeshRate(USkeletalMeshComponent* MeshComponent, float NewRate);
	void CollectSessionMeshes(TArray<USkeletalMeshComponent*>& OutMeshes) const;
	bool TryRecruitPartner();
	void GrantSatisfiedSxp();
	void AddChronicleDialogue(const FText& Message);
	void AddChronicleSystem(const FText& Message);
	int32 ResolveAllureLevel() const;
	int32 ResolvePartnerLevel() const;
	EProjectIntimacyPersonality ResolveEffectivePersonality(const FProjectIntimacyPartnerProfile& Profile, const UProjectIntimacyPartnerComponent* PartnerComponent) const;
	void NormalizeProfile(UProjectIntimacyPartnerComponent* PartnerComponent, FProjectIntimacyPartnerProfile& Profile) const;
	void RefreshRelationshipTags(FProjectIntimacyPartnerProfile& Profile) const;
	bool RelationshipForcesChill(const FGameplayTagContainer& RelationshipTags) const;
	FGameplayTag GetBaseRelationshipTag(int32 Encounters) const;
	bool HasRelationshipTag(const FProjectIntimacyPartnerProfile& Profile, const TCHAR* TagName) const;
	void GrantTalkSxp(int32 Amount);
	FProjectIntimacyPartnerProfile& GetMutableProfile(UProjectIntimacyPartnerComponent* PartnerComponent);
	void LoadPersistentState();
	void SavePersistentState() const;
	UProjectSinfulAscensionComponent* EnsureSinfulAscensionComponent() const;
	UProjectSurvivalNeedsComponent* EnsureSurvivalNeedsComponent() const;
	UProjectSurvivalStatusComponent* EnsureStatusComponent() const;
	UDataTable* LoadTable(const FSoftObjectPath& TablePath) const;
	bool IsAllyEligible(const FProjectIntimacyPartnerProfile& Profile) const;

	void HandleToggleHudPressed();
	void HandleNavigateUpPressed();
	void HandleNavigateDownPressed();
	void HandleNavigateLeftPressed();
	void HandleNavigateRightPressed();
	void HandleConfirmPressed();

private:
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> TrackedPlayerController;

	UPROPERTY(Transient)
	TObjectPtr<APawn> TrackedPlayerPawn;

	UPROPERTY(Transient)
	TObjectPtr<UProjectEmoteComponent> TrackedEmoteComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectTargetingFixComponent> TrackedTargetingFixComponent;

	UPROPERTY(Transient)
	TObjectPtr<UInputComponent> IntimacyInputComponent;

	UPROPERTY(Transient)
	TObjectPtr<UProjectIntimacyHudWidget> HudWidget;

	UPROPERTY(Transient)
	TObjectPtr<UProjectIntimacySaveGame> IntimacySaveGame;

	FProjectIntimacyRuntimeSession ActiveSession;
	TSet<FName> RuntimeUnlockedAutomaticTattooIds;
	TArray<FProjectIntimacyResolvedOption> ResolvedOptions;
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, float> CachedMeshRates;
	FRandomStream RandomStream;
	bool bSessionActive = false;
	bool bSuppressStartUntilSceneEnds = false;
};
