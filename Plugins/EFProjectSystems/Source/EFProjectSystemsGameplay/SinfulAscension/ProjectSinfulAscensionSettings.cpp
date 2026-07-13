#include "SinfulAscension/ProjectSinfulAscensionSettings.h"

namespace
{
	const FName HungerName(TEXT("Hunger"));
	const FName ThirstName(TEXT("Thirst"));
	const FName SleepName(TEXT("Sleep"));
	const FName MadnessName(TEXT("Madness"));
	const FName PainName(TEXT("Pain"));
	const FName LustName(TEXT("Lust"));
	const FName FearStatusName(TEXT("Fear"));
	const FName DizzyStatusName(TEXT("Dizzy"));
}

UProjectSinfulAscensionSettings::UProjectSinfulAscensionSettings()
{
	DynamicMaximumRules = {
		FProjectSinfulAscensionDynamicMaxRule(HungerName, false, EProjectSinAttribute::Willpower, 1.5f, 0.f, true),
		FProjectSinfulAscensionDynamicMaxRule(ThirstName, false, EProjectSinAttribute::Willpower, 1.5f, 0.f, true),
		FProjectSinfulAscensionDynamicMaxRule(SleepName, false, EProjectSinAttribute::Willpower, 1.5f, 0.f, true),
		FProjectSinfulAscensionDynamicMaxRule(MadnessName, true, EProjectSinAttribute::Willpower, 1.5f, 0.f, true),
		FProjectSinfulAscensionDynamicMaxRule(PainName, true, EProjectSinAttribute::Willpower, 1.5f, 0.f, true),
		FProjectSinfulAscensionDynamicMaxRule(LustName, true, EProjectSinAttribute::Willpower, 1.5f, 0.f, true),
		FProjectSinfulAscensionDynamicMaxRule(LustName, true, EProjectSinAttribute::Allure, 100.f, 0.f),
	};

	WillpowerSecondBreathNeedNames = {
		HungerName,
		ThirstName,
		SleepName
	};

	WillpowerSecondBreathSensationNames = {
		LustName,
		PainName,
		MadnessName
	};

	WillpowerImmunityStatusNames = {
		FearStatusName,
		DizzyStatusName
	};
}

const UProjectSinfulAscensionSettings* UProjectSinfulAscensionSettings::Get()
{
	return GetDefault<UProjectSinfulAscensionSettings>();
}

int32 UProjectSinfulAscensionSettings::ComputeAttributeUpgradeCost(const int32 CurrentLevel)
{
	const float SafeLevel = static_cast<float>(FMath::Max(CurrentLevel, 0) + 1);
	return FMath::Max(1, FMath::RoundToInt(80.f * FMath::Pow(SafeLevel, 1.35f)));
}

float UProjectSinfulAscensionSettings::ComputeMasochismFlatDamageNegation(
	const int32 CurrentLevel,
	const float FlatNegationPerLevel)
{
	if (CurrentLevel <= 0 || FlatNegationPerLevel <= 0.f)
	{
		return 0.f;
	}

	return static_cast<float>(CurrentLevel) * FlatNegationPerLevel;
}

float UProjectSinfulAscensionSettings::ComputeMasochismPainOverflowLust(
	const float LustMax,
	const float OverflowPct)
{
	if (LustMax <= 0.f || OverflowPct <= 0.f)
	{
		return 0.f;
	}

	return LustMax * FMath::Clamp(OverflowPct, 0.f, 1.f);
}

float UProjectSinfulAscensionSettings::ComputeMasochismPainReflectionLust(
	const float PainAppliedDelta,
	const float ReflectionPct)
{
	if (PainAppliedDelta <= 0.f || ReflectionPct <= 0.f)
	{
		return 0.f;
	}

	return PainAppliedDelta * ReflectionPct;
}

float UProjectSinfulAscensionSettings::ComputeFaithPassiveBonus(
	const int32 CurrentLevel,
	const float BonusPerLevel)
{
	if (CurrentLevel <= 0 || BonusPerLevel <= 0.f)
	{
		return 0.f;
	}

	return static_cast<float>(CurrentLevel) * BonusPerLevel;
}

float UProjectSinfulAscensionSettings::ComputeFaithMadnessRecovery(
	const float DeltaSeconds,
	const float RecoveryPerSecond)
{
	if (DeltaSeconds <= 0.f || RecoveryPerSecond <= 0.f)
	{
		return 0.f;
	}

	return DeltaSeconds * RecoveryPerSecond;
}

float UProjectSinfulAscensionSettings::ComputeFaithSleepMadnessRestore(
	const float MadnessMax,
	const float RestorePct)
{
	if (MadnessMax <= 0.f || RestorePct <= 0.f)
	{
		return 0.f;
	}

	return MadnessMax * FMath::Clamp(RestorePct, 0.f, 1.f);
}

float UProjectSinfulAscensionSettings::ComputeScaledResourceGain(
	const float AppliedDamage,
	const float GainPerDamagePct,
	const float FlatCap)
{
	if (AppliedDamage <= 0.f || GainPerDamagePct <= 0.f || FlatCap <= 0.f)
	{
		return 0.f;
	}

	return FMath::Min(AppliedDamage * GainPerDamagePct, FlatCap);
}

float UProjectSinfulAscensionSettings::ComputeCunningPassiveRatio(
	const int32 CurrentLevel,
	const float Pivot)
{
	const float SafeLevel = static_cast<float>(FMath::Max(CurrentLevel, 0));
	if (SafeLevel <= 0.f)
	{
		return 0.f;
	}

	return SafeLevel / (SafeLevel + FMath::Max(Pivot, 0.001f));
}

float UProjectSinfulAscensionSettings::ComputeCunningLockpickSpeedMultiplier(
	const int32 Difficulty,
	const int32 CurrentLevel,
	const float SpeedBaseMultiplier,
	const float MaxSlowPct,
	const float TimePivot)
{
	const float BaseSpeed = 1.0f + static_cast<float>(FMath::Clamp(Difficulty, 1, 100)) * 0.025f;
	const float SlowRatio = ComputeCunningPassiveRatio(CurrentLevel, TimePivot);
	const float SafeBaseMultiplier = FMath::Max(SpeedBaseMultiplier, 0.01f);
	const float SafeMaxSlowPct = FMath::Clamp(MaxSlowPct, 0.f, 0.95f);
	return FMath::Clamp(BaseSpeed * SafeBaseMultiplier * (1.f - SafeMaxSlowPct * SlowRatio), 0.85f, 4.0f);
}

float UProjectSinfulAscensionSettings::ComputeCunningLockpickTargetHalfRange(
	const int32 Difficulty,
	const int32 CurrentLevel,
	const float ZonePivot)
{
	const float BaseHalfRange = FMath::Clamp(
		0.10f - static_cast<float>(FMath::Clamp(Difficulty, 1, 100)) * 0.00045f,
		0.035f,
		0.18f);
	const float ZoneRatio = ComputeCunningPassiveRatio(CurrentLevel, ZonePivot);
	return FMath::Clamp(BaseHalfRange * (1.f + ZoneRatio), 0.035f, 0.18f);
}

float UProjectSinfulAscensionSettings::ComputeCunningStruggleSpeedMultiplier(
	const int32 CurrentLevel,
	const float SpeedBaseMultiplier,
	const float MaxSlowPct,
	const float TimePivot)
{
	const float SlowRatio = ComputeCunningPassiveRatio(CurrentLevel, TimePivot);
	const float SafeBaseMultiplier = FMath::Max(SpeedBaseMultiplier, 0.01f);
	const float SafeMaxSlowPct = FMath::Clamp(MaxSlowPct, 0.f, 0.95f);
	return FMath::Clamp(SafeBaseMultiplier * (1.f - SafeMaxSlowPct * SlowRatio), 0.65f, 1.30f);
}

float UProjectSinfulAscensionSettings::ComputeCunningScaledStruggleSeconds(
	const float BaseSeconds,
	const float StruggleSpeedMultiplier,
	const float MinSeconds,
	const float MaxSeconds)
{
	if (BaseSeconds <= 0.f)
	{
		return FMath::Max(MinSeconds, 0.f);
	}

	const float SafeMin = FMath::Max(MinSeconds, 0.f);
	const float SafeMax = FMath::Max(MaxSeconds, SafeMin);
	return FMath::Clamp(BaseSeconds / FMath::Max(StruggleSpeedMultiplier, 0.001f), SafeMin, SafeMax);
}

int32 UProjectSinfulAscensionSettings::ComputeCunningStruggleMaxMisses(
	const int32 CurrentLevel,
	const int32 DefaultMaxMisses,
	const int32 MilestoneLevel,
	const int32 MilestoneMaxMisses)
{
	const int32 SafeDefaultMaxMisses = FMath::Max(DefaultMaxMisses, 1);
	if (FMath::Max(CurrentLevel, 0) < FMath::Max(MilestoneLevel, 0))
	{
		return SafeDefaultMaxMisses;
	}

	return FMath::Max(SafeDefaultMaxMisses, MilestoneMaxMisses);
}

void UProjectSinfulAscensionSettings::AccumulateDynamicMaxRule(
	const FProjectSinfulAscensionDynamicMaxRule& Rule,
	const int32 AttributeLevel,
	float& InOutFlatBonus,
	float& InOutPercentMultiplier)
{
	if (AttributeLevel <= 0)
	{
		return;
	}

	const float SafeLevel = static_cast<float>(AttributeLevel);
	float FlatContribution = SafeLevel * FMath::Max(0.f, Rule.FlatBonusPerLevel);
	if (Rule.bFloorFlatContribution)
	{
		FlatContribution = FMath::FloorToFloat(FlatContribution);
	}
	InOutFlatBonus += FlatContribution;

	const float PercentBonus = SafeLevel * FMath::Max(0.f, Rule.PercentBonusPerLevel);
	if (PercentBonus > 0.f)
	{
		InOutPercentMultiplier *= FMath::Max(0.f, 1.f + PercentBonus);
	}
}

float UProjectSinfulAscensionSettings::ComputeDynamicMaxValue(
	const float BaseMax,
	const float FlatBonusTotal,
	const float PercentMultiplier)
{
	const float SafeBase = FMath::Max(BaseMax, 0.001f);
	const float SafeFlat = FMath::Max(0.f, FlatBonusTotal);
	const float SafeMultiplier = FMath::Max(0.f, PercentMultiplier);
	return FMath::Max((SafeBase + SafeFlat) * SafeMultiplier, 0.001f);
}

FName UProjectSinfulAscensionSettings::GetCategoryName() const
{
	return TEXT("Game");
}
