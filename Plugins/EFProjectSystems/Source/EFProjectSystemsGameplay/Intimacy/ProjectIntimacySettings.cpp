#include "Intimacy/ProjectIntimacySettings.h"

UProjectIntimacySettings::UProjectIntimacySettings()
{
	HudToggleKey = EKeys::Hyphen;
	HudSecondaryToggleKey = EKeys::Subtract;
	SocialCardRowsTable = FSoftObjectPath(TEXT("/Game/_Game/Data/Intimacy/DT_ProjectSocialCardRows.DT_ProjectSocialCardRows"));
}

const UProjectIntimacySettings* UProjectIntimacySettings::Get()
{
	return GetDefault<UProjectIntimacySettings>();
}

float UProjectIntimacySettings::ComputePartnerMaxLust(const int32 PartnerLevel, const UProjectIntimacySettings* Settings)
{
	const UProjectIntimacySettings* ResolvedSettings = Settings ? Settings : Get();
	const float MinLust = ResolvedSettings ? FMath::Max(1.0f, ResolvedSettings->MinPartnerLust) : 10000.0f;
	const float MaxLust = ResolvedSettings ? FMath::Max(MinLust, ResolvedSettings->MaxPartnerLust) : 50000.0f;
	const float Alpha = static_cast<float>(FMath::Clamp(PartnerLevel, 1, 100) - 1) / 99.0f;
	return FMath::Lerp(MinLust, MaxLust, Alpha);
}

float UProjectIntimacySettings::ComputeAllureDrainPerSecond(const int32 AllureLevel, const UProjectIntimacySettings* Settings)
{
	const UProjectIntimacySettings* ResolvedSettings = Settings ? Settings : Get();
	const float DrainPerLevel = ResolvedSettings ? FMath::Max(0.0f, ResolvedSettings->AllureDrainPerLevel) : 5.0f;
	return FMath::Max(0, AllureLevel) * DrainPerLevel;
}

float UProjectIntimacySettings::ComputePleaseInstantDrain(
	const int32 AllureLevel,
	const int32 SuccessfulHits,
	const UProjectIntimacySettings* Settings)
{
	return ComputeAllureDrainPerSecond(AllureLevel, Settings) * FMath::Max(0, SuccessfulHits);
}

float UProjectIntimacySettings::ComputePleasePulsePeriod(
	const int32 PartnerLevel,
	const int32 AllureLevel,
	const UProjectIntimacySettings* Settings)
{
	const UProjectIntimacySettings* ResolvedSettings = Settings ? Settings : Get();
	const float Base = ResolvedSettings ? ResolvedSettings->PleaseBasePulsePeriod : 1.20f;
	const float LevelPenalty = ResolvedSettings ? ResolvedSettings->PleasePartnerLevelSpeedPenalty : 0.004f;
	const float AllureBonus = ResolvedSettings ? ResolvedSettings->PleaseAllurePeriodBonus : 0.010f;
	const float MinPeriod = ResolvedSettings ? ResolvedSettings->PleaseMinPulsePeriod : 0.55f;
	const float MaxPeriod = ResolvedSettings ? ResolvedSettings->PleaseMaxPulsePeriod : 2.20f;
	const float Period = Base - static_cast<float>(FMath::Clamp(PartnerLevel, 1, 100)) * LevelPenalty
		+ static_cast<float>(FMath::Max(0, AllureLevel)) * AllureBonus;
	return FMath::Clamp(Period, FMath::Max(0.01f, MinPeriod), FMath::Max(MinPeriod, MaxPeriod));
}

float UProjectIntimacySettings::ComputePleaseTargetHalfRange(
	const int32 PartnerLevel,
	const int32 AllureLevel,
	const UProjectIntimacySettings* Settings)
{
	const UProjectIntimacySettings* ResolvedSettings = Settings ? Settings : Get();
	const float Base = ResolvedSettings ? ResolvedSettings->PleaseBaseTargetHalfRange : 0.075f;
	const float LevelPenalty = ResolvedSettings ? ResolvedSettings->PleasePartnerLevelRangePenalty : 0.00025f;
	const float AllureBonus = ResolvedSettings ? ResolvedSettings->PleaseAllureRangeBonus : 0.0015f;
	const float MinRange = ResolvedSettings ? ResolvedSettings->PleaseMinTargetHalfRange : 0.045f;
	const float MaxRange = ResolvedSettings ? ResolvedSettings->PleaseMaxTargetHalfRange : 0.22f;
	const float Range = Base - static_cast<float>(FMath::Clamp(PartnerLevel, 1, 100)) * LevelPenalty
		+ static_cast<float>(FMath::Max(0, AllureLevel)) * AllureBonus;
	return FMath::Clamp(Range, FMath::Max(0.001f, MinRange), FMath::Max(MinRange, MaxRange));
}

float UProjectIntimacySettings::ComputeClimaxThreshold(const int32 PartnerLevel, const UProjectIntimacySettings* Settings)
{
	const UProjectIntimacySettings* ResolvedSettings = Settings ? Settings : Get();
	const float BaseThreshold = ResolvedSettings ? FMath::Max(0.0f, ResolvedSettings->ClimaxBaseThreshold) : 1000.0f;
	const float PerLevel = ResolvedSettings ? FMath::Max(0.0f, ResolvedSettings->ClimaxThresholdPerPartnerLevel) : 100.0f;
	return FMath::Max(1.0f, BaseThreshold + (static_cast<float>(FMath::Max(0, PartnerLevel)) * PerLevel));
}

int32 UProjectIntimacySettings::ConsumeClimaxProgress(
	const float CurrentProgress,
	const float EffectiveDrain,
	const float ClimaxThreshold,
	float& OutRemainingProgress)
{
	const float SafeThreshold = FMath::Max(1.0f, ClimaxThreshold);
	const float TotalProgress = FMath::Max(0.0f, CurrentProgress) + FMath::Max(0.0f, EffectiveDrain);
	const int32 ClimaxCount = FMath::FloorToInt(TotalProgress / SafeThreshold);
	OutRemainingProgress = FMath::Fmod(TotalProgress, SafeThreshold);
	return FMath::Max(0, ClimaxCount);
}

float UProjectIntimacySettings::ComputeClimaxAnticipationMultiplier(
	const float CurrentProgress,
	const float ClimaxThreshold,
	const UProjectIntimacySettings* Settings)
{
	const UProjectIntimacySettings* ResolvedSettings = Settings ? Settings : Get();
	const float TargetMultiplier = ResolvedSettings ? FMath::Max(1.0f, ResolvedSettings->ClimaxIntensityMultiplier) : 1.25f;
	const float Window = ResolvedSettings ? FMath::Max(0.0f, ResolvedSettings->ClimaxAnticipationWindow) : 200.0f;
	const float SafeThreshold = FMath::Max(1.0f, ClimaxThreshold);
	if (Window <= 0.0f)
	{
		return 1.0f;
	}

	const float Remaining = SafeThreshold - FMath::Clamp(CurrentProgress, 0.0f, SafeThreshold);
	if (Remaining > Window)
	{
		return 1.0f;
	}

	const float Alpha = FMath::Clamp(1.0f - (Remaining / Window), 0.0f, 1.0f);
	return FMath::Lerp(1.0f, TargetMultiplier, Alpha);
}

int32 UProjectIntimacySettings::ClampControlPoints(const int32 ControlPoints, const UProjectIntimacySettings* Settings)
{
	const UProjectIntimacySettings* ResolvedSettings = Settings ? Settings : Get();
	const int32 MinControl = ResolvedSettings ? ResolvedSettings->MinControlPoints : 0;
	const int32 MaxControl = ResolvedSettings ? FMath::Max(MinControl, ResolvedSettings->MaxControlPoints) : 499;
	return FMath::Clamp(ControlPoints, MinControl, MaxControl);
}

int32 UProjectIntimacySettings::ApplyControlDelta(
	const int32 ControlPoints,
	const int32 Delta,
	const UProjectIntimacySettings* Settings)
{
	return ClampControlPoints(ControlPoints + Delta, Settings);
}

int32 UProjectIntimacySettings::ComputeInitialControl(
	const EProjectIntimacyPersonality Personality,
	const UProjectIntimacySettings* Settings)
{
	const UProjectIntimacySettings* ResolvedSettings = Settings ? Settings : Get();
	switch (Personality)
	{
	case EProjectIntimacyPersonality::Chill:
		return ClampControlPoints(ResolvedSettings ? ResolvedSettings->ChillInitialControl : 125, ResolvedSettings);
	case EProjectIntimacyPersonality::Stallion:
		return ClampControlPoints(ResolvedSettings ? ResolvedSettings->StallionInitialControl : 425, ResolvedSettings);
	case EProjectIntimacyPersonality::Nice:
	case EProjectIntimacyPersonality::Auto:
	default:
		return ClampControlPoints(ResolvedSettings ? ResolvedSettings->NiceInitialControl : 225, ResolvedSettings);
	}
}

EProjectIntimacyControlState UProjectIntimacySettings::GetControlState(
	const int32 ControlPoints,
	const UProjectIntimacySettings* Settings)
{
	const int32 ClampedControl = ClampControlPoints(ControlPoints, Settings);
	if (ClampedControl < 100)
	{
		return EProjectIntimacyControlState::Compliant;
	}
	if (ClampedControl < 200)
	{
		return EProjectIntimacyControlState::Calm;
	}
	if (ClampedControl < 300)
	{
		return EProjectIntimacyControlState::Heated;
	}
	if (ClampedControl < 400)
	{
		return EProjectIntimacyControlState::Defiant;
	}
	return EProjectIntimacyControlState::Overpowering;
}

FGameplayTag UProjectIntimacySettings::GetControlStateTag(
	const int32 ControlPoints,
	const UProjectIntimacySettings* Settings)
{
	switch (GetControlState(ControlPoints, Settings))
	{
	case EProjectIntimacyControlState::Compliant:
		return FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Control.Compliant"), false);
	case EProjectIntimacyControlState::Calm:
		return FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Control.Calm"), false);
	case EProjectIntimacyControlState::Heated:
		return FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Control.Heated"), false);
	case EProjectIntimacyControlState::Defiant:
		return FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Control.Defiant"), false);
	case EProjectIntimacyControlState::Overpowering:
	default:
		return FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Control.Overpowering"), false);
	}
}

FText UProjectIntimacySettings::GetControlStateText(
	const int32 ControlPoints,
	const UProjectIntimacySettings* Settings)
{
	switch (GetControlState(ControlPoints, Settings))
	{
	case EProjectIntimacyControlState::Compliant:
		return FText::FromString(TEXT("Compliant"));
	case EProjectIntimacyControlState::Calm:
		return FText::FromString(TEXT("Calm"));
	case EProjectIntimacyControlState::Heated:
		return FText::FromString(TEXT("Heated"));
	case EProjectIntimacyControlState::Defiant:
		return FText::FromString(TEXT("Defiant"));
	case EProjectIntimacyControlState::Overpowering:
	default:
		return FText::FromString(TEXT("Overpowering"));
	}
}

float UProjectIntimacySettings::ComputeControlDrainMultiplier(
	const int32 ControlPoints,
	const UProjectIntimacySettings* Settings)
{
	const UProjectIntimacySettings* ResolvedSettings = Settings ? Settings : Get();
	switch (GetControlState(ControlPoints, ResolvedSettings))
	{
	case EProjectIntimacyControlState::Compliant:
		return ResolvedSettings ? FMath::Max(0.0f, ResolvedSettings->CompliantDrainMultiplier) : 1.0f;
	case EProjectIntimacyControlState::Calm:
		return ResolvedSettings ? FMath::Max(0.0f, ResolvedSettings->CalmDrainMultiplier) : 0.90f;
	case EProjectIntimacyControlState::Heated:
		return ResolvedSettings ? FMath::Max(0.0f, ResolvedSettings->HeatedDrainMultiplier) : 0.75f;
	case EProjectIntimacyControlState::Defiant:
		return ResolvedSettings ? FMath::Max(0.0f, ResolvedSettings->DefiantDrainMultiplier) : 0.60f;
	case EProjectIntimacyControlState::Overpowering:
	default:
		return ResolvedSettings ? FMath::Max(0.0f, ResolvedSettings->OverpoweringDrainMultiplier) : 0.35f;
	}
}

FName UProjectIntimacySettings::GetCategoryName() const
{
	return TEXT("Project");
}
