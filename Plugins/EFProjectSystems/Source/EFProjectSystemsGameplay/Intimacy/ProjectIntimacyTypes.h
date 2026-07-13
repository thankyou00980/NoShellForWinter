#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPath.h"
#include "ProjectIntimacyTypes.generated.h"

UENUM(BlueprintType)
enum class EProjectIntimacyPersonality : uint8
{
	Auto UMETA(DisplayName = "Auto"),
	Chill UMETA(DisplayName = "Chill"),
	Nice UMETA(DisplayName = "Nice"),
	Stallion UMETA(DisplayName = "Stallion")
};

UENUM(BlueprintType)
enum class EProjectIntimacyRelationship : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	Familiar UMETA(DisplayName = "Familiar"),
	Close UMETA(DisplayName = "Close"),
	Ally UMETA(DisplayName = "Ally")
};

UENUM(BlueprintType)
enum class EProjectIntimacyControlState : uint8
{
	Compliant UMETA(DisplayName = "Compliant"),
	Calm UMETA(DisplayName = "Calm"),
	Heated UMETA(DisplayName = "Heated"),
	Defiant UMETA(DisplayName = "Defiant"),
	Overpowering UMETA(DisplayName = "Overpowering")
};

UENUM(BlueprintType)
enum class EProjectIntimacyHudMode : uint8
{
	Main UMETA(DisplayName = "Main"),
	Talk UMETA(DisplayName = "Talk"),
	Items UMETA(DisplayName = "Items"),
	Please UMETA(DisplayName = "Please")
};

UENUM(BlueprintType)
enum class EProjectIntimacyTalkAction : uint8
{
	None UMETA(DisplayName = "None"),
	SpeedSlow UMETA(DisplayName = "Speed Slow"),
	SpeedNormal UMETA(DisplayName = "Speed Normal"),
	SpeedIntense UMETA(DisplayName = "Speed Intense"),
	Compliment UMETA(DisplayName = "Compliment"),
	Pathetic UMETA(DisplayName = "Pathetic"),
	More UMETA(DisplayName = "More"),
	Recruit UMETA(DisplayName = "Recruit"),
	Back UMETA(DisplayName = "Back")
};

UENUM(BlueprintType)
enum class EProjectIntimacyMediaType : uint8
{
	None UMETA(DisplayName = "None"),
	Image UMETA(DisplayName = "Image"),
	Gif UMETA(DisplayName = "GIF"),
	Video UMETA(DisplayName = "Video"),
	Animation UMETA(DisplayName = "Animation")
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectIntimacyPartnerProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FString PartnerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 Encounters = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 SatisfiedWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 ClimaxCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 Children = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 FailedEncounters = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FDateTime FirstEncounterUtc = FDateTime();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float TotalIntimateTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyPersonality Personality = EProjectIntimacyPersonality::Nice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyRelationship Relationship = EProjectIntimacyRelationship::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTagContainer RelationshipTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTag GenderTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 ControlPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bControlInitialized = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 Affect = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bHasFirstEncounter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bConvertedToAlly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bHasHusbandRing = false;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectIntimacyTalkOptionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FName OptionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyTalkAction Action = EProjectIntimacyTalkAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FName CategoryId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTag RequiredPersonalityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTagContainer TalkTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float LustDrain = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 SxpReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 ControlDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 AffectDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float AnimationRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bRequiresAllyEligibility = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bCanBeCorrectTalkOption = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bUsesTalkCooldown = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bCanBeFlavorCorrectOption = true;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectIntimacyPartnerResponseRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FName OptionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTag PersonalityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText AcceptedResponse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText RefusedResponse;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectIntimacyMediaCueRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FName CueId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FName TriggerOptionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyTalkAction TriggerTalkAction = EProjectIntimacyTalkAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FName TriggerEventId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyMediaType MediaType = EProjectIntimacyMediaType::Image;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FSoftObjectPath TextureAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FString SourceImagePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FSoftObjectPath SoundAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy", meta = (ClampMin = "0.0"))
	float FadeInSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy", meta = (ClampMin = "0.0"))
	float HoldSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy", meta = (ClampMin = "0.0"))
	float FadeOutSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FVector2D SourceMediaSize = FVector2D(1080.0f, 720.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectIntimacyPersonalityRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyPersonality Personality = EProjectIntimacyPersonality::Nice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTag PersonalityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float DefaultAnimationRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 InitialControl = 100;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectIntimacyControlRuleRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyPersonality Personality = EProjectIntimacyPersonality::Nice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 InitialControl = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTagContainer PreferredTalkTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float PassiveDrainMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectIntimacyTalkAffinityRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyPersonality Personality = EProjectIntimacyPersonality::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTag ControlStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTag RelationshipTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTagContainer PreferredTalkTags;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectIntimacyItemEffectRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float SxpMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float LustDrainBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 AffectBonus = 0;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectIntimacyHudOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FName OptionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText Label;
};

USTRUCT(BlueprintType)
struct EFPROJECTSYSTEMSGAMEPLAY_API FProjectIntimacySessionSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bHudVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyHudMode HudMode = EProjectIntimacyHudMode::Main;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText PartnerDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FString PartnerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyPersonality Personality = EProjectIntimacyPersonality::Nice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyPersonality EffectivePersonality = EProjectIntimacyPersonality::Nice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyRelationship Relationship = EProjectIntimacyRelationship::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTagContainer RelationshipTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText RelationshipText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTag GenderTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText GenderText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float CurrentLust = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float MaxLust = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float LustDrainPerSecond = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 Affect = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 SatisfiedWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 Encounters = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 Children = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 FailedEncounters = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 ControlPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	EProjectIntimacyControlState ControlState = EProjectIntimacyControlState::Calm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FGameplayTag ControlStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText ControlStateText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float TotalIntimateTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bProfileHistoryVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	bool bPleaseActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 PleaseAttemptIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 PleaseAttemptCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 PleaseSuccessCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float PleaseCursorValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float PleaseTargetCenter = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float PleaseTargetHalfRange = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float PleasePreviewDrain = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	float TalkCooldownRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FName CorrectTalkOptionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	int32 SelectedOptionIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	TArray<FProjectIntimacyHudOption> Options;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText StatusText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Project|Intimacy")
	FText HintText;
};
