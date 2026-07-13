#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPath.h"
#include "EFProjectUISettings.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSCORE_API FProjectInnerStateEntryDefinition
{
	GENERATED_BODY()

	FProjectInnerStateEntryDefinition()
		: EntryName(NAME_None)
		, DisplayLabel()
		, Monogram()
		, bIsSensation(false)
		, AccentTint(FLinearColor(0.92f, 0.40f, 0.72f, 1.0f))
		, SortOrder(0)
	{
	}

	FProjectInnerStateEntryDefinition(
		const FName InEntryName,
		const FString& InDisplayLabel,
		const FString& InMonogram,
		const bool bInIsSensation,
		const FLinearColor& InAccentTint,
		const int32 InSortOrder)
		: EntryName(InEntryName)
		, DisplayLabel(InDisplayLabel)
		, Monogram(InMonogram)
		, bIsSensation(bInIsSensation)
		, AccentTint(InAccentTint)
		, SortOrder(InSortOrder)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inner State")
	FName EntryName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inner State")
	FString DisplayLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inner State")
	FString Monogram;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inner State")
	bool bIsSensation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inner State")
	FLinearColor AccentTint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inner State")
	int32 SortOrder;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSCORE_API FProjectSinfulAscensionEntryDefinition
{
	GENERATED_BODY()

	FProjectSinfulAscensionEntryDefinition()
		: AttributeName(NAME_None)
		, DisplayLabel()
		, ShortLabel()
		, IconTexture(nullptr)
		, MenuDisplayLabel()
		, MenuIconTexture(nullptr)
		, MenuWatermarkTexture(nullptr)
		, DescriptionText()
		, MilestoneFiveText()
		, MilestoneTenText()
		, AccentTint(FLinearColor(0.92f, 0.40f, 0.72f, 1.0f))
		, SortOrder(0)
	{
	}

	FProjectSinfulAscensionEntryDefinition(
		const FName InAttributeName,
		const FString& InDisplayLabel,
		const FString& InShortLabel,
		const TSoftObjectPtr<UTexture2D>& InIconTexture,
		const FString& InMenuDisplayLabel,
		const TSoftObjectPtr<UTexture2D>& InMenuIconTexture,
		const TSoftObjectPtr<UTexture2D>& InMenuWatermarkTexture,
		const FString& InDescriptionText,
		const FString& InMilestoneFiveText,
		const FString& InMilestoneTenText,
		const FLinearColor& InAccentTint,
		const int32 InSortOrder)
		: AttributeName(InAttributeName)
		, DisplayLabel(InDisplayLabel)
		, ShortLabel(InShortLabel)
		, IconTexture(InIconTexture)
		, MenuDisplayLabel(InMenuDisplayLabel)
		, MenuIconTexture(InMenuIconTexture)
		, MenuWatermarkTexture(InMenuWatermarkTexture)
		, DescriptionText(InDescriptionText)
		, MilestoneFiveText(InMilestoneFiveText)
		, MilestoneTenText(InMilestoneTenText)
		, AccentTint(InAccentTint)
		, SortOrder(InSortOrder)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FName AttributeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FString DisplayLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FString ShortLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	TSoftObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FString MenuDisplayLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	TSoftObjectPtr<UTexture2D> MenuIconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	TSoftObjectPtr<UTexture2D> MenuWatermarkTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FString DescriptionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FString MilestoneFiveText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FString MilestoneTenText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FLinearColor AccentTint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	int32 SortOrder;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSCORE_API FProjectChronicleChannelDefinition
{
	GENERATED_BODY()

	FProjectChronicleChannelDefinition()
		: ChannelName(NAME_None)
		, DisplayLabel()
		, AccentTint(FLinearColor(0.92f, 0.40f, 0.72f, 1.0f))
		, BadgeFillTint(FLinearColor(0.77f, 0.38f, 0.64f, 0.96f))
		, BadgeTextTint(FLinearColor(0.08f, 0.06f, 0.10f, 1.0f))
	{
	}

	FProjectChronicleChannelDefinition(
		const FName InChannelName,
		const FString& InDisplayLabel,
		const FLinearColor& InAccentTint,
		const FLinearColor& InBadgeFillTint,
		const FLinearColor& InBadgeTextTint)
		: ChannelName(InChannelName)
		, DisplayLabel(InDisplayLabel)
		, AccentTint(InAccentTint)
		, BadgeFillTint(InBadgeFillTint)
		, BadgeTextTint(InBadgeTextTint)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronicle")
	FName ChannelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronicle")
	FString DisplayLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronicle")
	FLinearColor AccentTint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronicle")
	FLinearColor BadgeFillTint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronicle")
	FLinearColor BadgeTextTint;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "EF Project UI"))
class EFPROJECTSYSTEMSCORE_API UEFProjectUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UEFProjectUISettings();

	static const UEFProjectUISettings* Get();

	virtual FName GetCategoryName() const override;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets")
	FSoftClassPath NeedsWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets|Inner State")
	TArray<FProjectInnerStateEntryDefinition> InnerStateEntryDefinitions;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets")
	FSoftClassPath StatusWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets")
	FSoftClassPath SinfulAscensionWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets")
	FSoftClassPath SinfulAscensionExchangeMenuWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets")
	TArray<FDirectoryPath> WidgetDiscoveryRootPaths;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets")
	FSoftClassPath LockpickingWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets")
	FSoftClassPath LockpickingPromptWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets|Sinful Ascension")
	TArray<FProjectSinfulAscensionEntryDefinition> SinfulAscensionEntryDefinitions;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets")
	FSoftClassPath ActivityFeedWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets|Gameplay Debug")
	FSoftClassPath GameplayDebugMenuWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets|Gameplay Debug")
	FSoftClassPath GameplayDebugMenuOptionRowWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "Widgets|Chronicle")
	TArray<FProjectChronicleChannelDefinition> ChronicleChannelDefinitions;
};
