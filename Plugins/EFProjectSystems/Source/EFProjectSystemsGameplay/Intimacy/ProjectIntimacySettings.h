#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "GameplayTagContainer.h"
#include "Intimacy/ProjectIntimacyTypes.h"
#include "UObject/SoftObjectPath.h"
#include "ProjectIntimacySettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Project Intimacy"))
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectIntimacySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UProjectIntimacySettings();

	static const UProjectIntimacySettings* Get();
	static float ComputePartnerMaxLust(int32 PartnerLevel, const UProjectIntimacySettings* Settings = nullptr);
	static float ComputeAllureDrainPerSecond(int32 AllureLevel, const UProjectIntimacySettings* Settings = nullptr);
	static float ComputePleaseInstantDrain(int32 AllureLevel, int32 SuccessfulHits, const UProjectIntimacySettings* Settings = nullptr);
	static float ComputePleasePulsePeriod(int32 PartnerLevel, int32 AllureLevel, const UProjectIntimacySettings* Settings = nullptr);
	static float ComputePleaseTargetHalfRange(int32 PartnerLevel, int32 AllureLevel, const UProjectIntimacySettings* Settings = nullptr);
	static float ComputeClimaxThreshold(int32 PartnerLevel, const UProjectIntimacySettings* Settings = nullptr);
	static int32 ConsumeClimaxProgress(float CurrentProgress, float EffectiveDrain, float ClimaxThreshold, float& OutRemainingProgress);
	static float ComputeClimaxAnticipationMultiplier(float CurrentProgress, float ClimaxThreshold, const UProjectIntimacySettings* Settings = nullptr);
	static int32 ClampControlPoints(int32 ControlPoints, const UProjectIntimacySettings* Settings = nullptr);
	static int32 ApplyControlDelta(int32 ControlPoints, int32 Delta, const UProjectIntimacySettings* Settings = nullptr);
	static int32 ComputeInitialControl(EProjectIntimacyPersonality Personality, const UProjectIntimacySettings* Settings = nullptr);
	static EProjectIntimacyControlState GetControlState(int32 ControlPoints, const UProjectIntimacySettings* Settings = nullptr);
	static FGameplayTag GetControlStateTag(int32 ControlPoints, const UProjectIntimacySettings* Settings = nullptr);
	static FText GetControlStateText(int32 ControlPoints, const UProjectIntimacySettings* Settings = nullptr);
	static float ComputeControlDrainMultiplier(int32 ControlPoints, const UProjectIntimacySettings* Settings = nullptr);

	virtual FName GetCategoryName() const override;

	UPROPERTY(EditAnywhere, Config, Category = "Lust", meta = (ClampMin = "1.0"))
	float MinPartnerLust = 10000.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Lust", meta = (ClampMin = "1.0"))
	float MaxPartnerLust = 50000.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Lust", meta = (ClampMin = "0.0"))
	float AllureDrainPerLevel = 5.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Climax", meta = (ClampMin = "0.0"))
	float ClimaxBaseThreshold = 1000.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Climax", meta = (ClampMin = "0.0"))
	float ClimaxThresholdPerPartnerLevel = 100.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Climax", meta = (ClampMin = "0.0"))
	float ClimaxAnticipationWindow = 200.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Climax", meta = (ClampMin = "1.0"))
	float ClimaxIntensityMultiplier = 1.25f;

	UPROPERTY(EditAnywhere, Config, Category = "Climax", meta = (ClampMin = "0.0"))
	float ClimaxRecoverySeconds = 2.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Climax", meta = (ClampMin = "0.0"))
	float PlayerLustGainPercentOnClimax = 0.10f;

	UPROPERTY(EditAnywhere, Config, Category = "Climax")
	FName ClimaxMediaEventId = TEXT("Climax");

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "1"))
	int32 PleaseAttemptCount = 5;

	UPROPERTY(EditAnywhere, Config, Category = "Control")
	int32 MinControlPoints = 0;

	UPROPERTY(EditAnywhere, Config, Category = "Control")
	int32 MaxControlPoints = 499;

	UPROPERTY(EditAnywhere, Config, Category = "Control")
	int32 PleaseHitControlDelta = -10;

	UPROPERTY(EditAnywhere, Config, Category = "Control")
	int32 PleaseMissControlDelta = 10;

	UPROPERTY(EditAnywhere, Config, Category = "Control")
	int32 ChillInitialControl = 125;

	UPROPERTY(EditAnywhere, Config, Category = "Control")
	int32 NiceInitialControl = 225;

	UPROPERTY(EditAnywhere, Config, Category = "Control")
	int32 StallionInitialControl = 425;

	UPROPERTY(EditAnywhere, Config, Category = "Control")
	int32 RelationshipChillMaxControl = 199;

	UPROPERTY(EditAnywhere, Config, Category = "Control", meta = (ClampMin = "0.0"))
	float CompliantDrainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Control", meta = (ClampMin = "0.0"))
	float CalmDrainMultiplier = 0.90f;

	UPROPERTY(EditAnywhere, Config, Category = "Control", meta = (ClampMin = "0.0"))
	float HeatedDrainMultiplier = 0.75f;

	UPROPERTY(EditAnywhere, Config, Category = "Control", meta = (ClampMin = "0.0"))
	float DefiantDrainMultiplier = 0.60f;

	UPROPERTY(EditAnywhere, Config, Category = "Control", meta = (ClampMin = "0.0"))
	float OverpoweringDrainMultiplier = 0.35f;

	UPROPERTY(EditAnywhere, Config, Category = "Control", meta = (ClampMin = "1.0"))
	float OverpoweringPulseIntervalSeconds = 60.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Control", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OverpoweringIntensityChance = 0.20f;

	UPROPERTY(EditAnywhere, Config, Category = "Control", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OverpoweringPoseChangeChance = 0.20f;

	UPROPERTY(EditAnywhere, Config, Category = "Control", meta = (ClampMin = "0.0"))
	float OverpoweringLustHealPerPulse = 100.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Control", meta = (ClampMin = "0.05"))
	float OverpoweringSelfIntensityRate = 1.50f;

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "0.01"))
	float PleaseBasePulsePeriod = 1.20f;

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "0.0"))
	float PleasePartnerLevelSpeedPenalty = 0.004f;

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "0.0"))
	float PleaseAllurePeriodBonus = 0.010f;

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "0.01"))
	float PleaseMinPulsePeriod = 0.55f;

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "0.01"))
	float PleaseMaxPulsePeriod = 2.20f;

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "0.001"))
	float PleaseBaseTargetHalfRange = 0.075f;

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "0.0"))
	float PleasePartnerLevelRangePenalty = 0.00025f;

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "0.0"))
	float PleaseAllureRangeBonus = 0.0015f;

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "0.001"))
	float PleaseMinTargetHalfRange = 0.045f;

	UPROPERTY(EditAnywhere, Config, Category = "Please", meta = (ClampMin = "0.001"))
	float PleaseMaxTargetHalfRange = 0.22f;

	UPROPERTY(EditAnywhere, Config, Category = "Talk", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TalkRefusalChance = 0.10f;

	UPROPERTY(EditAnywhere, Config, Category = "Talk", meta = (ClampMin = "0.0"))
	float PersonalityTalkLustDrain = 500.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Talk", meta = (ClampMin = "0.0"))
	float TalkCooldownSeconds = 2.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Talk", meta = (ClampMin = "0"))
	int32 TalkSxp = 20;

	UPROPERTY(EditAnywhere, Config, Category = "Talk", meta = (ClampMin = "0.0"))
	float CorrectTalkLustDrainBonus = 25.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Talk", meta = (ClampMin = "0"))
	int32 ComplimentAffectGain = 3;

	UPROPERTY(EditAnywhere, Config, Category = "Relationship", meta = (ClampMin = "1"))
	int32 AffectMax = 100;

	UPROPERTY(EditAnywhere, Config, Category = "Relationship", meta = (ClampMin = "0"))
	int32 SatisfiedWinAffectGain = 10;

	UPROPERTY(EditAnywhere, Config, Category = "Relationship", meta = (ClampMin = "0"))
	int32 RequiredSatisfiedWinsForAlly = 10;

	UPROPERTY(EditAnywhere, Config, Category = "SXP", meta = (ClampMin = "0"))
	int32 SatisfiedWinSxp = 150;

	UPROPERTY(EditAnywhere, Config, Category = "Animation", meta = (ClampMin = "0.05"))
	float ChillAnimationRate = 0.75f;

	UPROPERTY(EditAnywhere, Config, Category = "Animation", meta = (ClampMin = "0.05"))
	float NiceAnimationRate = 1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Animation", meta = (ClampMin = "0.05"))
	float StallionAnimationRate = 1.35f;

	UPROPERTY(EditAnywhere, Config, Category = "Input")
	FKey HudToggleKey;

	UPROPERTY(EditAnywhere, Config, Category = "Input")
	FKey HudSecondaryToggleKey;

	UPROPERTY(EditAnywhere, Config, Category = "UI")
	FSoftClassPath IntimacyHudWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "UI", meta = (ClampMin = "0"))
	int32 IntimacyHudZOrder = 325;

	UPROPERTY(EditAnywhere, Config, Category = "DataTables")
	FSoftObjectPath TalkOptionsTable;

	UPROPERTY(EditAnywhere, Config, Category = "DataTables")
	FSoftObjectPath PartnerResponsesTable;

	UPROPERTY(EditAnywhere, Config, Category = "DataTables")
	FSoftObjectPath MediaCuesTable;

	UPROPERTY(EditAnywhere, Config, Category = "DataTables")
	FSoftObjectPath PersonalitiesTable;

	UPROPERTY(EditAnywhere, Config, Category = "DataTables")
	FSoftObjectPath ControlRulesTable;

	UPROPERTY(EditAnywhere, Config, Category = "DataTables")
	FSoftObjectPath TalkAffinityTable;

	UPROPERTY(EditAnywhere, Config, Category = "DataTables")
	FSoftObjectPath ItemEffectsTable;

	UPROPERTY(EditAnywhere, Config, Category = "DataTables")
	FSoftObjectPath SocialCardRowsTable;

	UPROPERTY(EditAnywhere, Config, Category = "Save")
	FString SaveSlotName = TEXT("ProjectIntimacy");

	UPROPERTY(EditAnywhere, Config, Category = "Save", meta = (ClampMin = "0"))
	int32 SaveUserIndex = 0;

	UPROPERTY(EditAnywhere, Config, Category = "Recruit")
	FGameplayTag OverrideAllyTeamTag;
};
