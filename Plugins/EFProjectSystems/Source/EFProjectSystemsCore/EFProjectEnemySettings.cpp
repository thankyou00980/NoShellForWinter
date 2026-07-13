#include "EFProjectEnemySettings.h"

UEFProjectEnemySettings::UEFProjectEnemySettings()
{
	RuntimeEnemyClasses.Add(FSoftClassPath(TEXT("/Game/FullSample/Blueprints/Characters/Enemies/ACFMeleeEnemyBP.ACFMeleeEnemyBP_C")));
	RuntimeEnemyClasses.Add(FSoftClassPath(TEXT("/Game/FullSample/Blueprints/Characters/Enemies/ACFRangedEnemyBP.ACFRangedEnemyBP_C")));
	RuntimeEnemyClasses.Add(FSoftClassPath(TEXT("/Game/FullSample/Blueprints/Characters/Enemies/ACFMageEnemyBP.ACFMageEnemyBP_C")));
	bEnableExtendedTargetStatsWhenNeedsHudVisible = true;
}

const UEFProjectEnemySettings* UEFProjectEnemySettings::Get()
{
	return GetDefault<UEFProjectEnemySettings>();
}

FName UEFProjectEnemySettings::GetCategoryName() const
{
	return TEXT("EF Project Systems");
}
