#include "EFProjectUISettings.h"

UEFProjectUISettings::UEFProjectUISettings()
{
	const auto CompactIconPath = [](const TCHAR* AssetName) -> TSoftObjectPtr<UTexture2D>
	{
		return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(FString::Printf(
			TEXT("/Game/_Game/Widgets/Attributes/Assets/Textures/%s.%s"),
			AssetName,
			AssetName)));
	};

	const auto MenuIconPath = [](const TCHAR* AssetName) -> TSoftObjectPtr<UTexture2D>
	{
		return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(FString::Printf(
			TEXT("/Game/_Game/Widgets/SinfulAscensionAltar/Assets/Textures/%s.%s"),
			AssetName,
			AssetName)));
	};

	NeedsWidgetClass = FSoftClassPath();
	InnerStateEntryDefinitions = {
		FProjectInnerStateEntryDefinition(TEXT("Hunger"), TEXT("HUNGER"), TEXT("H"), false, FLinearColor(0.95f, 0.44f, 0.73f, 1.0f), 10),
		FProjectInnerStateEntryDefinition(TEXT("Thirst"), TEXT("THIRST"), TEXT("T"), false, FLinearColor(0.74f, 0.57f, 0.98f, 1.0f), 20),
		FProjectInnerStateEntryDefinition(TEXT("Sleep"), TEXT("SLEEP"), TEXT("S"), false, FLinearColor(0.93f, 0.48f, 0.76f, 1.0f), 30),
		FProjectInnerStateEntryDefinition(TEXT("Madness"), TEXT("MADNESS"), TEXT("M"), true, FLinearColor(0.62f, 0.39f, 0.88f, 1.0f), 40),
		FProjectInnerStateEntryDefinition(TEXT("Lust"), TEXT("LUST"), TEXT("L"), true, FLinearColor(0.91f, 0.33f, 0.69f, 1.0f), 50),
		FProjectInnerStateEntryDefinition(TEXT("Pain"), TEXT("PAIN"), TEXT("P"), true, FLinearColor(1.0f, 0.72f, 0.84f, 1.0f), 60),
	};
	StatusWidgetClass = FSoftClassPath();
	SinfulAscensionWidgetClass = FSoftClassPath();
	SinfulAscensionExchangeMenuWidgetClass = FSoftClassPath();

	FDirectoryPath WidgetRootPath;
	WidgetRootPath.Path = TEXT("/Game/_Game/Widgets");
	WidgetDiscoveryRootPaths = { WidgetRootPath };

	SinfulAscensionEntryDefinitions = {
		FProjectSinfulAscensionEntryDefinition(
			TEXT("Willpower"),
			TEXT("WILLPOWER"),
			TEXT("WIL"),
			CompactIconPath(TEXT("T_SinfulAscension_Icon_Willpower")),
			TEXT("Willpower"),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Icon_Willpower")),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Watermark_Willpower")),
			TEXT("Endurance for long expeditions: expands survival limits, grants a second breath between floors, and steadies the mind against fear and confusion."),
			TEXT("Second Breath"),
			TEXT("Unbroken Mind"),
			FLinearColor(0.97f, 0.67f, 0.85f, 1.0f),
			10),
		FProjectSinfulAscensionEntryDefinition(
			TEXT("Sadism"),
			TEXT("SADISM"),
			TEXT("SAD"),
			CompactIconPath(TEXT("T_SinfulAscension_Icon_Sadism")),
			TEXT("Sadism"),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Icon_Sadism")),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Watermark_Sadism")),
			TEXT("Turns non-spell aggression into lust pressure, then rewards clean executions on weakened male targets."),
			TEXT("Milestone V"),
			TEXT("Milestone X"),
			FLinearColor(0.95f, 0.54f, 0.78f, 1.0f),
			20),
		FProjectSinfulAscensionEntryDefinition(
			TEXT("Masochism"),
			TEXT("MASOCHISM"),
			TEXT("MAS"),
			CompactIconPath(TEXT("T_SinfulAscension_Icon_Masochism")),
			TEXT("Masochism"),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Icon_Masochism")),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Watermark_Masochism")),
			TEXT("Converts endured pain into flat damage denial, rapture protection, Pain Overflow, and Down Arrow surrender or recovery control."),
			TEXT("Pain Rapture"),
			TEXT("Pain Overflow"),
			FLinearColor(0.96f, 0.45f, 0.74f, 1.0f),
			30),
		FProjectSinfulAscensionEntryDefinition(
			TEXT("Faith"),
			TEXT("FAITH"),
			TEXT("FAI"),
			CompactIconPath(TEXT("T_SinfulAscension_Icon_Faith")),
			TEXT("Faith"),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Icon_Faith")),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Watermark_Faith")),
			TEXT("Devotion that sharpens spellcraft, wards against magic, calms Madness over time, and lets sleep cleanse the mind."),
			TEXT("Quiet Mind"),
			TEXT("Sanctified Rest"),
			FLinearColor(0.92f, 0.59f, 0.84f, 1.0f),
			40),
		FProjectSinfulAscensionEntryDefinition(
			TEXT("Cunning"),
			TEXT("CUNNING"),
			TEXT("CUN"),
			CompactIconPath(TEXT("T_SinfulAscension_Icon_Cunning")),
			TEXT("Cunning"),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Icon_Cunning")),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Watermark_Cunning")),
			TEXT("Slows lockpick and struggle timing through precise hands, widening lockpick openings and preserving gear at mastery."),
			TEXT("Steady Hands"),
			TEXT("Clean Getaway"),
			FLinearColor(0.98f, 0.57f, 0.82f, 1.0f),
			50),
		FProjectSinfulAscensionEntryDefinition(
			TEXT("Celerity"),
			TEXT("CELERITY"),
			TEXT("CEL"),
			CompactIconPath(TEXT("T_SinfulAscension_Icon_Celerity")),
			TEXT("Celerity"),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Icon_Celerity")),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Watermark_Celerity")),
			TEXT("Turns speed into momentum for faster SXP gain, richer Y-menu actions, and recovered run SXP at mastery."),
			TEXT("Y-Menu Surge"),
			TEXT("Recovered Momentum"),
			FLinearColor(0.88f, 0.63f, 0.88f, 1.0f),
			60),
		FProjectSinfulAscensionEntryDefinition(
			TEXT("Allure"),
			TEXT("ALLURE"),
			TEXT("ALL"),
			CompactIconPath(TEXT("T_SinfulAscension_Icon_Allure")),
			TEXT("Allure"),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Icon_Allure")),
			MenuIconPath(TEXT("T_SinfulAscension_Altar_Watermark_Allure")),
			TEXT("Makes sinful encounters more profitable, pulling extra value out of corrupted moments."),
			TEXT(""),
			TEXT(""),
			FLinearColor(0.99f, 0.63f, 0.86f, 1.0f),
			70),
	};
	ActivityFeedWidgetClass = FSoftClassPath();
	GameplayDebugMenuWidgetClass = FSoftClassPath();
	GameplayDebugMenuOptionRowWidgetClass = FSoftClassPath();
	ChronicleChannelDefinitions = {
		FProjectChronicleChannelDefinition(
			TEXT("System"),
			TEXT("LOG"),
			FLinearColor(0.86f, 0.64f, 0.76f, 1.0f),
			FLinearColor(0.70f, 0.44f, 0.58f, 0.96f),
			FLinearColor(0.98f, 0.95f, 0.98f, 1.0f)),
		FProjectChronicleChannelDefinition(
			TEXT("Loot"),
			TEXT("LOOT"),
			FLinearColor(0.93f, 0.67f, 0.72f, 1.0f),
			FLinearColor(0.83f, 0.46f, 0.58f, 0.96f),
			FLinearColor(0.98f, 0.95f, 0.98f, 1.0f)),
		FProjectChronicleChannelDefinition(
			TEXT("Experience"),
			TEXT("SXP"),
			FLinearColor(0.98f, 0.52f, 0.86f, 1.0f),
			FLinearColor(0.86f, 0.34f, 0.70f, 0.98f),
			FLinearColor(0.98f, 0.95f, 0.98f, 1.0f)),
		FProjectChronicleChannelDefinition(
			TEXT("Combat"),
			TEXT("COMBAT"),
			FLinearColor(0.95f, 0.46f, 0.60f, 1.0f),
			FLinearColor(0.82f, 0.33f, 0.48f, 0.98f),
			FLinearColor(0.98f, 0.95f, 0.98f, 1.0f)),
		FProjectChronicleChannelDefinition(
			TEXT("Status"),
			TEXT("STATUS"),
			FLinearColor(0.83f, 0.64f, 0.96f, 1.0f),
			FLinearColor(0.69f, 0.48f, 0.84f, 0.98f),
			FLinearColor(0.98f, 0.95f, 0.98f, 1.0f)),
		FProjectChronicleChannelDefinition(
			TEXT("Dialogue"),
			TEXT("ENEMY"),
			FLinearColor(0.86f, 0.76f, 0.96f, 1.0f),
			FLinearColor(0.72f, 0.58f, 0.86f, 0.98f),
			FLinearColor(0.98f, 0.95f, 0.98f, 1.0f)),
	};
}

const UEFProjectUISettings* UEFProjectUISettings::Get()
{
	return GetDefault<UEFProjectUISettings>();
}

FName UEFProjectUISettings::GetCategoryName() const
{
	return TEXT("EF Project Systems");
}
