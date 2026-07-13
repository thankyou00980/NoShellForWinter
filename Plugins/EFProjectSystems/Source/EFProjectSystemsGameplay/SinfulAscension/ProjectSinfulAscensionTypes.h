#pragma once

#include "CoreMinimal.h"
#include "ProjectSinfulAscensionTypes.generated.h"

UENUM(BlueprintType)
enum class EProjectSinAttribute : uint8
{
	Willpower UMETA(DisplayName = "Willpower"),
	Sadism UMETA(DisplayName = "Sadism"),
	Masochism UMETA(DisplayName = "Masochism"),
	Faith UMETA(DisplayName = "Faith"),
	Cunning UMETA(DisplayName = "Cunning"),
	Celerity UMETA(DisplayName = "Celerity"),
	Allure UMETA(DisplayName = "Allure"),
	Count UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectSinAttributeState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	EProjectSinAttribute Attribute = EProjectSinAttribute::Willpower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	int32 Level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	int32 NextLevelCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	bool bMilestone5Unlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	bool bMilestone10Unlocked = false;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectSinMilestoneState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	FName AbilityId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	EProjectSinAttribute Attribute = EProjectSinAttribute::Willpower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	int32 RequiredLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	bool bUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	bool bOnCooldown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float CooldownRemainingSeconds = 0.f;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectSinfulAscensionSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	int32 CurrentRunSxp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	int32 MetaBankSxp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	bool bEternalSinMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float Madness = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float MadnessMax = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float Lust = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float LustMax = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float Pain = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float PainMax = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float SensoryOverloadProgressSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	bool bSensoryOverloadActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	bool bResolveRegenActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float ResolveHealthThresholdPct = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float ResolveHealthRegenPerSecond = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	bool bUnbrokenResolveAvailable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	bool bUnbrokenResolveInvulnerable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	float UnbrokenResolveRemainingSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	TArray<FProjectSinAttributeState> Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinful Ascension")
	TArray<FProjectSinMilestoneState> Milestones;
};
