#include "Characters/ProjectEnemyLevelSettings.h"

#include "GameFramework/Pawn.h"
#include "UObject/SoftObjectPath.h"

namespace ProjectEnemyLevelSettingsPrivate
{
	static TSoftClassPtr<APawn> MakeEnemyClass(const TCHAR* AssetPath)
	{
		return TSoftClassPtr<APawn>(FSoftObjectPath(AssetPath));
	}

	static FProjectEnemyMapLevelRule MakeMapRule(const TCHAR* MapPattern, const int32 WorldTier)
	{
		FProjectEnemyMapLevelRule Rule;
		Rule.MapNamePattern = MapPattern;
		Rule.WorldTier = WorldTier;
		return Rule;
	}
}

UProjectEnemyLevelSettings::UProjectEnemyLevelSettings()
{
	CategoryName = TEXT("Game");
	SectionName = TEXT("ProjectEnemyLevel");

	TargetEnemyBaseClasses = {
		ProjectEnemyLevelSettingsPrivate::MakeEnemyClass(TEXT("/Game/FullSample/Blueprints/Characters/Enemies/ACFMageEnemyBP.ACFMageEnemyBP_C")),
		ProjectEnemyLevelSettingsPrivate::MakeEnemyClass(TEXT("/Game/FullSample/Blueprints/Characters/Enemies/ACFMeleeEnemyBP.ACFMeleeEnemyBP_C")),
		ProjectEnemyLevelSettingsPrivate::MakeEnemyClass(TEXT("/Game/FullSample/Blueprints/Characters/Enemies/ACFRangedEnemyBP.ACFRangedEnemyBP_C"))
	};

	MapLevelRules = {
		ProjectEnemyLevelSettingsPrivate::MakeMapRule(TEXT("DungeonGeneration"), 1)
	};

	LevelOffsetWeights = { 5, 4, 3, 2, 1 };
	PreferredTargetPointSockets = {
		TEXT("head"),
		TEXT("Head"),
		TEXT("neck_01"),
		TEXT("spine_03")
	};
	bSyncAssignedLevelToAscentLevelingComponent = false;
	bReinitializeAscentStatisticsOnLevelSync = false;
}

const UProjectEnemyLevelSettings* UProjectEnemyLevelSettings::Get()
{
	return GetDefault<UProjectEnemyLevelSettings>();
}

FName UProjectEnemyLevelSettings::GetCategoryName() const
{
	return TEXT("Game");
}
