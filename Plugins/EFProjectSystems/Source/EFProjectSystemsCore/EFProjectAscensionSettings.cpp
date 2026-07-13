#include "EFProjectAscensionSettings.h"

UEFProjectAscensionSettings::UEFProjectAscensionSettings()
{
	bEnableEternalSinModeByDefault = false;
	DefaultMetaSxpGrant = 0;
}

const UEFProjectAscensionSettings* UEFProjectAscensionSettings::Get()
{
	return GetDefault<UEFProjectAscensionSettings>();
}

FName UEFProjectAscensionSettings::GetCategoryName() const
{
	return TEXT("EF Project Systems");
}
