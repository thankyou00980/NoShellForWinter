#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ProjectEnemyVisualVariationSettings.generated.h"

class APawn;

UENUM()
enum class EProjectEnemyMorphDistributionMode : uint8
{
	Uniform,
	Weighted
};

USTRUCT()
struct FProjectEnemyMorphBiasEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Config, Category = "Morph")
	FName MorphName = NAME_None;

	UPROPERTY(EditAnywhere, Config, Category = "Morph", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float PositiveWeight = 1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Morph", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float NegativeWeight = 1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Morph", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float NearZeroWeight = 1.0f;
};

USTRUCT()
struct FProjectEnemyLevelBiasedMorphEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Config, Category = "Morph")
	FName MorphName = NAME_None;

	UPROPERTY(EditAnywhere, Config, Category = "Morph", meta = (ClampMin = "1", UIMin = "1"))
	int32 PreferredMinLevel = 1;

	UPROPERTY(EditAnywhere, Config, Category = "Morph", meta = (ClampMin = "0", UIMin = "0"))
	int32 PreferredMaxLevel = 0;

	UPROPERTY(EditAnywhere, Config, Category = "Morph", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float PreferredRangeWeightMultiplier = 1.25f;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Project Enemy Visual Variation"))
class EFPROJECTSYSTEMSGAMEPLAY_API UProjectEnemyVisualVariationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UProjectEnemyVisualVariationSettings();

	static const UProjectEnemyVisualVariationSettings* Get();

	virtual FName GetCategoryName() const override;

	UPROPERTY(EditAnywhere, Config, Category = "Targets", meta = (AllowedClasses = "/Script/Engine.Pawn"))
	TArray<TSoftClassPtr<APawn>> TargetEnemyClasses;

	UPROPERTY(EditAnywhere, Config, Category = "Morphs")
	TArray<FName> AllowedMorphNames;

	UPROPERTY(EditAnywhere, Config, Category = "Morphs")
	EProjectEnemyMorphDistributionMode DistributionMode = EProjectEnemyMorphDistributionMode::Uniform;

	UPROPERTY(EditAnywhere, Config, Category = "Morphs", meta = (TitleProperty = "MorphName"))
	TArray<FProjectEnemyMorphBiasEntry> MorphBiasEntries;

	UPROPERTY(EditAnywhere, Config, Category = "Distribution", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float PositiveBucketChance = 0.70f;

	UPROPERTY(EditAnywhere, Config, Category = "Distribution", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float NegativeBucketChance = 0.25f;

	UPROPERTY(EditAnywhere, Config, Category = "Distribution", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float NearZeroBucketChance = 0.05f;

	UPROPERTY(EditAnywhere, Config, Category = "Distribution", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float PositiveExtremeExponent = 0.35f;

	UPROPERTY(EditAnywhere, Config, Category = "Distribution", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float NegativeExtremeExponent = 0.50f;

	UPROPERTY(EditAnywhere, Config, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float PositiveMinAbsValue = 0.55f;

	UPROPERTY(EditAnywhere, Config, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float NegativeMinAbsValue = 0.45f;

	UPROPERTY(EditAnywhere, Config, Category = "Distribution", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float NearZeroMaxAbsValue = 0.12f;

	UPROPERTY(EditAnywhere, Config, Category = "Morphs", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
	float MorphMinValue = -1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Morphs", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
	float MorphMaxValue = 1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group1", meta = (TitleProperty = "MorphName"))
	TArray<FProjectEnemyLevelBiasedMorphEntry> GroupOneMorphEntries;

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group2")
	TArray<FName> GroupTwoMorphNames;

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group2", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float GroupTwoHighValueChance = 0.70f;

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group2", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float GroupTwoHighValueMin = 0.50f;

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group2", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float GroupTwoHighValueMax = 1.00f;

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group2", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float GroupTwoLowValueMin = 0.00f;

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group2", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float GroupTwoLowValueMax = 0.49f;

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group3")
	FName GroupThreeFixedMorphName = TEXT("DK_Scrotum_Scale");

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group3")
	FName GroupThreeOptionalMorphName = TEXT("DK_Serial Killers");

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group3", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float GroupThreeOptionalMorphChance = 0.50f;

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group3", meta = (ClampMin = "1", UIMin = "1"))
	int32 GroupThreeStartLevel = 25;

	UPROPERTY(EditAnywhere, Config, Category = "Male Morphs|Group3", meta = (ClampMin = "1", UIMin = "1"))
	int32 GroupThreeMaxLevel = 125;

	UPROPERTY(EditAnywhere, Config, Category = "Skin", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SkinBrightnessMin = 0.35f;

	UPROPERTY(EditAnywhere, Config, Category = "Skin", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SkinBrightnessMax = 1.70f;

	UPROPERTY(EditAnywhere, Config, Category = "Skin")
	bool bApplySkinColorNatively = true;

	UPROPERTY(EditAnywhere, Config, Category = "Skin")
	TArray<FName> EnemySkinColorParameterNames;

	UPROPERTY(EditAnywhere, Config, Category = "Skin")
	TArray<FString> EnemySkinMaterialHints;

	UPROPERTY(EditAnywhere, Config, Category = "Runtime", meta = (ClampMin = "0", UIMin = "0"))
	int32 InitializationRetryCount = 3;

	UPROPERTY(EditAnywhere, Config, Category = "Arousal")
	bool bEnableSightDrivenArousalMorphs = true;

	UPROPERTY(EditAnywhere, Config, Category = "Arousal")
	FName FlaccidMorphName = TEXT("DK_Flacid04");

	UPROPERTY(EditAnywhere, Config, Category = "Arousal")
	FName ErectionMorphName = TEXT("DK_Erection");

	UPROPERTY(EditAnywhere, Config, Category = "Arousal", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float MorphTransitionSpeed = 0.25f;

	UPROPERTY(EditAnywhere, Config, Category = "Arousal", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float SightCheckIntervalSeconds = 0.10f;

	UPROPERTY(EditAnywhere, Config, Category = "Arousal", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SightRange = 2200.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Arousal", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
	float SightDotThreshold = 0.40f;
};
