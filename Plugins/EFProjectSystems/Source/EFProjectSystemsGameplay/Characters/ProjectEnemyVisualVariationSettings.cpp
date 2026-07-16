#include "Characters/ProjectEnemyVisualVariationSettings.h"

#include "GameFramework/Pawn.h"
#include "UObject/SoftObjectPath.h"

namespace ProjectEnemyVisualVariationSettingsPrivate
{
	static TSoftClassPtr<APawn> MakeEnemyClass(const TCHAR* AssetPath)
	{
		return TSoftClassPtr<APawn>(FSoftObjectPath(AssetPath));
	}

	static FProjectEnemyMorphBiasEntry MakeMorphBiasEntry(
		const TCHAR* MorphName,
		const float PositiveWeight,
		const float NegativeWeight,
		const float NearZeroWeight)
	{
		FProjectEnemyMorphBiasEntry Entry;
		Entry.MorphName = FName(MorphName);
		Entry.PositiveWeight = PositiveWeight;
		Entry.NegativeWeight = NegativeWeight;
		Entry.NearZeroWeight = NearZeroWeight;
		return Entry;
	}

	static FProjectEnemyLevelBiasedMorphEntry MakeLevelBiasedMorphEntry(
		const TCHAR* MorphName,
		const int32 PreferredMinLevel,
		const int32 PreferredMaxLevel,
		const float PreferredRangeWeightMultiplier)
	{
		FProjectEnemyLevelBiasedMorphEntry Entry;
		Entry.MorphName = FName(MorphName);
		Entry.PreferredMinLevel = PreferredMinLevel;
		Entry.PreferredMaxLevel = PreferredMaxLevel;
		Entry.PreferredRangeWeightMultiplier = PreferredRangeWeightMultiplier;
		return Entry;
	}
}

UProjectEnemyVisualVariationSettings::UProjectEnemyVisualVariationSettings()
{
	CategoryName = TEXT("Game");
	SectionName = TEXT("ProjectEnemyVisualVariation");

	TargetEnemyClasses = {
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFDefenderEnemyBPMale.ACFDefenderEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFDummyAmbushEnemyBPMale.ACFDummyAmbushEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFDummyEnemyBPMale.ACFDummyEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFGunEnemyBPMale.ACFGunEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFMageEnemyBPMale.ACFMageEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFMeleeEnemyBPMale.ACFMeleeEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFMMEnemyBPMale.ACFMMEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFRangedEnemyBPMale.ACFRangedEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Female/ACFDefenderEnemyBPFemale.ACFDefenderEnemyBPFemale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Female/ACFDummyAmbushEnemyBPFemale.ACFDummyAmbushEnemyBPFemale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Female/ACFDummyEnemyBPFemale.ACFDummyEnemyBPFemale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Female/ACFGunEnemyBPFemale.ACFGunEnemyBPFemale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Female/ACFMageEnemyBPFemale.ACFMageEnemyBPFemale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Female/ACFMeleeEnemyBPFemale.ACFMeleeEnemyBPFemale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Female/ACFMMEnemyBPFemale.ACFMMEnemyBPFemale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Female/ACFRangedEnemyBPFemale.ACFRangedEnemyBPFemale_C"))
	};

	MaleMorphTargetEnemyClasses = {
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFDefenderEnemyBPMale.ACFDefenderEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFDummyAmbushEnemyBPMale.ACFDummyAmbushEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFDummyEnemyBPMale.ACFDummyEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFGunEnemyBPMale.ACFGunEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFMageEnemyBPMale.ACFMageEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFMeleeEnemyBPMale.ACFMeleeEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFMMEnemyBPMale.ACFMMEnemyBPMale_C")),
		ProjectEnemyVisualVariationSettingsPrivate::MakeEnemyClass(TEXT("/Game/_Game/Characters/Male/ACFRangedEnemyBPMale.ACFRangedEnemyBPMale_C"))
	};

	AllowedMorphNames = {
		TEXT("Body Emaciated"),
		TEXT("Body Fitness Details"),
		TEXT("Body Fitness Mass"),
		TEXT("Body Heavy"),
		TEXT("Body Lithe"),
		TEXT("Body Muscular Details"),
		TEXT("Body Muscular Mass"),
		TEXT("Body Older"),
		TEXT("Body Portly"),
		TEXT("Body Stocky"),
		TEXT("Body Thin"),
		TEXT("Body Tone")
	};

	MorphBiasEntries = {
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Muscular Mass"), 28.0f, 0.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Muscular Details"), 22.0f, 0.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Portly"), 22.0f, 0.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Heavy"), 14.0f, 0.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Stocky"), 8.0f, 0.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Tone"), 4.0f, 2.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Fitness Mass"), 2.0f, 10.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Fitness Details"), 0.0f, 12.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Thin"), 0.0f, 26.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Emaciated"), 0.0f, 22.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Lithe"), 0.0f, 18.0f, 1.0f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeMorphBiasEntry(TEXT("Body Older"), 0.0f, 0.0f, 1.0f)
	};

	GroupOneMorphEntries = {
		ProjectEnemyVisualVariationSettingsPrivate::MakeLevelBiasedMorphEntry(TEXT("DK_29-Cornelius"), 1, 11, 1.25f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeLevelBiasedMorphEntry(TEXT("DK_32-Caligula"), 12, 20, 1.25f),
		ProjectEnemyVisualVariationSettingsPrivate::MakeLevelBiasedMorphEntry(TEXT("DK_40-MagnaMater"), 30, 0, 1.25f)
	};

	GroupTwoMorphNames = {
		TEXT("DK_Glans_Inflate1"),
		TEXT("DK_Glans_Inflate2"),
		TEXT("DK_Glans_Mushroomy"),
		TEXT("DK_Urethra_Inflate1"),
		TEXT("DK_Shaft_Inflate2"),
		TEXT("DK_Shaft_Inflate3"),
		TEXT("DK_Scrotum_Down1"),
		TEXT("DK_Scrotum_Stretch")
	};

	GroupTwoHighValueChance = 0.70f;
	GroupTwoHighValueMin = 0.50f;
	GroupTwoHighValueMax = 1.00f;
	GroupTwoLowValueMin = 0.00f;
	GroupTwoLowValueMax = 0.49f;

	GroupThreeFixedMorphName = TEXT("DK_Scrotum_Scale");
	GroupThreeOptionalMorphName = TEXT("DK_Serial Killers");
	GroupThreeOptionalMorphChance = 0.50f;
	GroupThreeStartLevel = 25;
	GroupThreeMaxLevel = 125;

	SkinBrightnessMin = 0.35f;
	SkinBrightnessMax = 1.70f;
	bApplySkinColorNatively = true;
	EnemySkinColorParameterNames = {
		TEXT("Diffuse Color")
	};
	EnemySkinMaterialHints = {
		TEXT("Genesis9_Body"),
		TEXT("Genesis9_Head"),
		TEXT("Genesis9_Arms"),
		TEXT("Genesis9_Legs"),
		TEXT("Genesis9_Fingernails")
	};

	bEnableSightDrivenArousalMorphs = true;
	FlaccidMorphName = TEXT("DK_Flacid04");
	ErectionMorphName = TEXT("DK_Erection");
	MorphTransitionSpeed = 0.25f;
	SightCheckIntervalSeconds = 0.10f;
	SightRange = 2200.0f;
	SightDotThreshold = 0.40f;
}

const UProjectEnemyVisualVariationSettings* UProjectEnemyVisualVariationSettings::Get()
{
	return GetDefault<UProjectEnemyVisualVariationSettings>();
}

FName UProjectEnemyVisualVariationSettings::GetCategoryName() const
{
	return TEXT("Game");
}
