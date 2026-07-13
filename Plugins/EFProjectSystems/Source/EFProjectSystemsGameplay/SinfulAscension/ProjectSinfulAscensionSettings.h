#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SinfulAscension/ProjectSinfulAscensionTypes.h"
#include "ProjectSinfulAscensionSettings.generated.h"

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectSinfulAscensionDynamicMaxRule
{
	GENERATED_BODY()

	FProjectSinfulAscensionDynamicMaxRule() = default;

	FProjectSinfulAscensionDynamicMaxRule(
		FName InEntryName,
		bool bInIsSensation,
		EProjectSinAttribute InAttribute,
		float InFlatBonusPerLevel,
		float InPercentBonusPerLevel,
		bool bInFloorFlatContribution = false)
		: EntryName(InEntryName)
		, bIsSensation(bInIsSensation)
		, Attribute(InAttribute)
		, FlatBonusPerLevel(InFlatBonusPerLevel)
		, PercentBonusPerLevel(InPercentBonusPerLevel)
		, bFloorFlatContribution(bInFloorFlatContribution)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Maximums")
	FName EntryName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Maximums")
	bool bIsSensation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Maximums")
	EProjectSinAttribute Attribute = EProjectSinAttribute::Willpower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Maximums", meta = (ClampMin = "0.0"))
	float FlatBonusPerLevel = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Maximums", meta = (ClampMin = "0.0"))
	float PercentBonusPerLevel = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Maximums")
	bool bFloorFlatContribution = false;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Project Sinful Ascension"))
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectSinfulAscensionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UProjectSinfulAscensionSettings();

	static const UProjectSinfulAscensionSettings* Get();
	static int32 ComputeAttributeUpgradeCost(int32 CurrentLevel);
	static float ComputeMasochismFlatDamageNegation(int32 CurrentLevel, float FlatNegationPerLevel);
	static float ComputeMasochismPainOverflowLust(float LustMax, float OverflowPct);
	static float ComputeMasochismPainReflectionLust(float PainAppliedDelta, float ReflectionPct);
	static float ComputeFaithPassiveBonus(int32 CurrentLevel, float BonusPerLevel);
	static float ComputeFaithMadnessRecovery(float DeltaSeconds, float RecoveryPerSecond);
	static float ComputeFaithSleepMadnessRestore(float MadnessMax, float RestorePct);
	static float ComputeScaledResourceGain(float AppliedDamage, float GainPerDamagePct, float FlatCap);
	static float ComputeCunningPassiveRatio(int32 CurrentLevel, float Pivot);
	static float ComputeCunningLockpickSpeedMultiplier(int32 Difficulty, int32 CurrentLevel, float SpeedBaseMultiplier, float MaxSlowPct, float TimePivot);
	static float ComputeCunningLockpickTargetHalfRange(int32 Difficulty, int32 CurrentLevel, float ZonePivot);
	static float ComputeCunningStruggleSpeedMultiplier(int32 CurrentLevel, float SpeedBaseMultiplier, float MaxSlowPct, float TimePivot);
	static float ComputeCunningScaledStruggleSeconds(float BaseSeconds, float StruggleSpeedMultiplier, float MinSeconds, float MaxSeconds);
	static int32 ComputeCunningStruggleMaxMisses(int32 CurrentLevel, int32 DefaultMaxMisses, int32 MilestoneLevel, int32 MilestoneMaxMisses);
	static void AccumulateDynamicMaxRule(const FProjectSinfulAscensionDynamicMaxRule& Rule, int32 AttributeLevel, float& InOutFlatBonus, float& InOutPercentMultiplier);
	static float ComputeDynamicMaxValue(float BaseMax, float FlatBonusTotal, float PercentMultiplier);

	virtual FName GetCategoryName() const override;

	UPROPERTY(EditAnywhere, Config, Category = "SXP", meta = (ClampMin = "0"))
	int32 DanceSxpPerSecond = 200;

	UPROPERTY(EditAnywhere, Config, Category = "SXP", meta = (ClampMin = "0"))
	int32 KillSxp = 15;

	UPROPERTY(EditAnywhere, Config, Category = "SXP", meta = (ClampMin = "0"))
	int32 HitSxp = 3;

	UPROPERTY(EditAnywhere, Config, Category = "SXP", meta = (ClampMin = "0"))
	int32 SinfulInteractionSxp = 150;

	UPROPERTY(EditAnywhere, Config, Category = "SXP", meta = (ClampMin = "1"))
	int32 MaxSinAttributeLevel = 100;

	UPROPERTY(EditAnywhere, Config, Category = "SXP", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DeathRunLossRatio = 0.90f;

	UPROPERTY(EditAnywhere, Config, Category = "SXP")
	FString SaveSlotName = TEXT("ProjectSinfulAscension");

	UPROPERTY(EditAnywhere, Config, Category = "SXP", meta = (ClampMin = "0"))
	int32 SaveUserIndex = 0;

	UPROPERTY(EditAnywhere, Config, Category = "SXP", meta = (ClampMin = "0"))
	int32 SxpExchangeMenuZOrder = 285;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "1.0"))
	float MadnessMax = 100.f;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "1.0"))
	float PainMax = 100.f;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "1.0"))
	float LustMax = 10000.f;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "0.0"))
	float SensationDecayIdleSeconds = 60.f;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "0.1"))
	float SensationDecayIntervalSeconds = 8.f;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SensationDecayRatio = 0.05f;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "0.1"))
	float SensationMaxHoldSeconds = 10.f;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "0.1"))
	float SensoryOverloadRequiredSeconds = 60.f;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "0.1"))
	float SensoryOverloadBlackoutSeconds = 15.f;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "0.0"))
	float SensoryOverloadRecoveryDebuffSeconds = 8.f;

	UPROPERTY(EditAnywhere, Config, Category = "Sensations", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DanceLustPercentOfMaxPerSecond = 0.05f;

	UPROPERTY(EditAnywhere, Config, Category = "Dynamic Maximums")
	TArray<FProjectSinfulAscensionDynamicMaxRule> DynamicMaximumRules;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes", meta = (ClampMin = "0.0"))
	float SadismDamagePerLevel = 2.f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes", meta = (ClampMin = "0.0"))
	float FaithSpellDamagePerLevel = 2.f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes", meta = (ClampMin = "0.0"))
	float FaithSpellDefensePerLevel = 1.f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes", meta = (ClampMin = "0.0"))
	float CunningBurstDamagePerLevel = 1.5f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Cunning", meta = (ClampMin = "0.01"))
	float CunningLockpickSpeedBaseMultiplier = 1.15f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Cunning", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float CunningLockpickMaxSlowPct = 0.45f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Cunning", meta = (ClampMin = "0.01"))
	float CunningLockpickTimePivot = 10.f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Cunning", meta = (ClampMin = "0.01"))
	float CunningLockpickZonePivot = 5.f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Cunning", meta = (ClampMin = "0.01"))
	float CunningStruggleSpeedBaseMultiplier = 1.30f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Cunning", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float CunningStruggleMaxSlowPct = 0.50f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Cunning", meta = (ClampMin = "0.01"))
	float CunningStruggleTimePivot = 10.f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Cunning", meta = (ClampMin = "0"))
	int32 CunningStruggleMissMilestoneLevel = 5;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Cunning", meta = (ClampMin = "1"))
	int32 CunningStruggleMilestoneMaxMisses = 10;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Cunning", meta = (ClampMin = "0"))
	int32 CunningDefeatInventoryRetentionLevel = 10;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Masochism", meta = (ClampMin = "0.0"))
	float MasochismFlatDamageNegationPerLevel = 4.f;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes|Celerity", meta = (ClampMin = "0"))
	int32 CelerityFlatSxpPerLevel = 2;

	UPROPERTY(EditAnywhere, Config, Category = "Attributes", meta = (ClampMin = "0"))
	int32 AllureSxpPerSinfulLevel = 4;

	UPROPERTY(EditAnywhere, Config, Category = "Combat", meta = (ClampMin = "0.1"))
	float CombatStateExitSeconds = 8.f;

	UPROPERTY(EditAnywhere, Config, Category = "Willpower|Endurance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WillpowerSecondBreathRecoveryPct = 0.20f;

	UPROPERTY(EditAnywhere, Config, Category = "Willpower|Endurance")
	TArray<FName> WillpowerSecondBreathNeedNames;

	UPROPERTY(EditAnywhere, Config, Category = "Willpower|Endurance")
	TArray<FName> WillpowerSecondBreathSensationNames;

	UPROPERTY(EditAnywhere, Config, Category = "Willpower|Endurance")
	TArray<FName> WillpowerImmunityStatusNames;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Sadism", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SadismMilestone5LustFromDamagePct = 0.25f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Sadism", meta = (ClampMin = "0.0"))
	float SadismMilestone5LustPerHitCap = 250.f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Sadism", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SadismExecuteHealthThresholdPct = 0.20f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Sadism", meta = (ClampMin = "0.0"))
	float SadismMilestone10ExecuteLust = 500.f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Masochism", meta = (ClampMin = "0.0"))
	float MasochismMilestone5LustPerQualifiedHit = 50.f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Masochism", meta = (ClampMin = "0"))
	int32 MasochismPainRaptureActivationSxp = 2000;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Masochism", meta = (ClampMin = "0.0"))
	float MasochismRaptureHitFeedbackCooldownSeconds = 0.08f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Masochism", meta = (ClampMin = "0.05"))
	float MasochismRaptureHitFeedbackBindRefreshSeconds = 0.50f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Masochism")
	bool bMasochismRaptureSuppressDamageText = true;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Masochism", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasochismPainOverflowLustPct = 0.25f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Masochism", meta = (ClampMin = "0.0"))
	float MasochismPainReflectionPct = 5.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Masochism", meta = (ClampMin = "0.0"))
	float MasochismPainReflectionDelaySeconds = 0.10f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones", meta = (ClampMin = "0.0"))
	float StrongHitDamageThreshold = 15.f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Faith", meta = (ClampMin = "0.0"))
	float FaithMilestone5MadnessRecoveryPerSecond = 0.5f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Faith", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FaithSleepMadnessRestorePct = 0.50f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Celerity", meta = (ClampMin = "0.0"))
	float CelerityYMenuActionSxpMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Celerity", meta = (ClampMin = "0"))
	int32 CelerityYMenuActionSxpMilestoneLevel = 5;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones|Celerity", meta = (ClampMin = "0"))
	int32 CelerityDeathLostSxpToMetaMilestoneLevel = 10;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones", meta = (ClampMin = "0"))
	int32 AllureMilestone5KillSxp = 25;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones", meta = (ClampMin = "0"))
	int32 AllureMilestone10MarkedSxp = 250;

	UPROPERTY(EditAnywhere, Config, Category = "Milestones", meta = (ClampMin = "0.0"))
	float AllureMilestone10MarkedLust = 40.f;

	UPROPERTY(EditAnywhere, Config, Category = "Enemy Filters")
	TArray<FString> QualifiedMaleEnemyClassNameHints = {
		TEXT("MeleeMale"),
		TEXT("DummyMale"),
		TEXT("MageMale"),
		TEXT("RangedMale"),
		TEXT("ACFMeleeEnemyBP"),
		TEXT("ACFRangedEnemyBP"),
		TEXT("ACFMageEnemyBP")
	};
};
