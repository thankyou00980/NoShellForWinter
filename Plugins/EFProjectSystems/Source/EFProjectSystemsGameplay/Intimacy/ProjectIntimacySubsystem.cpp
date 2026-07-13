#include "Intimacy/ProjectIntimacySubsystem.h"

#include "Actors/ACFCharacter.h"
#include "Components/ACFCompanionGroupAIComponent.h"
#include "Components/ACFGroupAIComponent.h"
#include "Components/ACFTeamComponent.h"
#include "Characters/ProjectTargetingFixComponent.h"
#include "DirtyPawnComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Groups/ACFCompanionsPlayerController.h"
#include "Intimacy/ProjectIntimacyDialogueLibrary.h"
#include "Intimacy/ProjectIntimacyHudWidget.h"
#include "Intimacy/ProjectIntimacyPartnerComponent.h"
#include "Intimacy/ProjectIntimacySaveGame.h"
#include "Intimacy/ProjectIntimacySettings.h"
#include "Kismet/GameplayStatics.h"
#include "Locomotion/ProjectEmoteComponent.h"
#include "SinfulAscension/ProjectSinfulAscensionComponent.h"
#include "SinfulAscension/ProjectSinfulAscensionSettings.h"
#include "Survival/ProjectSurvivalNeedsComponent.h"
#include "Survival/ProjectSurvivalStatusComponent.h"
#include "UI/ProjectActivityFeedSubsystem.h"
#include "UI/ProjectEmoteSubsystem.h"
#include "UI/ProjectWidgetClassResolver.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogProjectIntimacy, Log, All);

const FName UProjectIntimacySubsystem::FirstIntimacyHeartChestTattooRewardId(
	TEXT("Tattoo.TestHeartChest.IntimacyEncounter1"));
const FName UProjectIntimacySubsystem::TestTattooIntimacyRewardId(TEXT("Test_Tattoo_Intimacy"));

namespace ProjectIntimacySubsystemPrivate
{
	const FName IntimacySceneId(TEXT("Actions.Together.0001Scene"));
	const FName PlayerLustSensationName(TEXT("Lust"));
	const FName TiredStatusName(TEXT("Tired"));
	constexpr int32 InputPriority = 75;

	FProjectIntimacyResolvedOption MakeOption(const TCHAR* Id, const TCHAR* Label)
	{
		FProjectIntimacyResolvedOption Option;
		Option.OptionId = FName(Id);
		Option.Label = FText::FromString(Label);
		return Option;
	}

	FGameplayTag Tag(const TCHAR* TagName)
	{
		return FGameplayTag::RequestGameplayTag(TagName, false);
	}

	void AddTag(FGameplayTagContainer& Container, const TCHAR* TagName)
	{
		const FGameplayTag GameplayTag = Tag(TagName);
		if (GameplayTag.IsValid())
		{
			Container.AddTag(GameplayTag);
		}
	}

	void RemoveTag(FGameplayTagContainer& Container, const TCHAR* TagName)
	{
		const FGameplayTag GameplayTag = Tag(TagName);
		if (GameplayTag.IsValid())
		{
			Container.RemoveTag(GameplayTag);
		}
	}

	bool HasTag(const FGameplayTagContainer& Container, const TCHAR* TagName)
	{
		const FGameplayTag GameplayTag = Tag(TagName);
		return GameplayTag.IsValid() && Container.HasTagExact(GameplayTag);
	}

	FString FormatDuration(const float Seconds)
	{
		const int32 TotalSeconds = FMath::Max(0, FMath::RoundToInt(Seconds));
		const int32 Hours = TotalSeconds / 3600;
		const int32 Minutes = (TotalSeconds % 3600) / 60;
		const int32 Remainder = TotalSeconds % 60;
		return Hours > 0
			? FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Remainder)
			: FString::Printf(TEXT("%02d:%02d"), Minutes, Remainder);
	}
}

void UProjectIntimacySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RandomStream.Initialize(static_cast<int32>(FDateTime::UtcNow().GetTicks() & 0x7fffffff));
	RuntimeUnlockedAutomaticTattooIds.Reset();
	LoadPersistentState();
}

void UProjectIntimacySubsystem::Deinitialize()
{
	EndSession(false);
	DetachFromTrackedPlayerController();
	SavePersistentState();
	RuntimeUnlockedAutomaticTattooIds.Reset();
	IntimacySaveGame = nullptr;
	Super::Deinitialize();
}

void UProjectIntimacySubsystem::Tick(const float DeltaTime)
{
	TryResolveRuntimeContext();

	AActor* PartnerActor = nullptr;
	UProjectIntimacyPartnerComponent* PartnerComponent = nullptr;
	const bool bSceneActive = ResolveActiveIntimacyPartner(PartnerActor, PartnerComponent);

	if (!bSceneActive)
	{
		if (bSessionActive)
		{
			if (!ActiveSession.bSatisfied && !bSuppressStartUntilSceneEnds)
			{
				UE_LOG(
					LogProjectIntimacy,
					Warning,
					TEXT("[ProjectIntimacy] unexpected_intimacy_scene_end partner=%s hud=%s please=%s time=%.2f"),
					*GetNameSafe(ActiveSession.PartnerActor.Get()),
					ActiveSession.bHudVisible ? TEXT("true") : TEXT("false"),
					ActiveSession.bPleaseActive ? TEXT("true") : TEXT("false"),
					ActiveSession.SessionTimeSeconds);
			}
			RefreshActiveIntimacyCombatShield();
			EndSession(!ActiveSession.bSatisfied);
		}
		bSuppressStartUntilSceneEnds = false;
		return;
	}

	if (bSuppressStartUntilSceneEnds)
	{
		return;
	}

	if (!bSessionActive)
	{
		StartSession(PartnerActor, PartnerComponent);
	}
	else if (ActiveSession.PartnerActor.Get() != PartnerActor)
	{
		EndSession(!ActiveSession.bSatisfied);
		StartSession(PartnerActor, PartnerComponent);
	}

	UpdateActiveSession(DeltaTime);
}

TStatId UProjectIntimacySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UProjectIntimacySubsystem, STATGROUP_Tickables);
}

bool UProjectIntimacySubsystem::IsTickable() const
{
	return !IsTemplate();
}

bool UProjectIntimacySubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UProjectIntimacySubsystem::IsIntimacySessionActive() const
{
	return bSessionActive;
}

bool UProjectIntimacySubsystem::IsHudVisible() const
{
	return bSessionActive && ActiveSession.bHudVisible;
}

EProjectIntimacyHudMode UProjectIntimacySubsystem::GetHudMode() const
{
	return bSessionActive ? ActiveSession.HudMode : EProjectIntimacyHudMode::Main;
}

bool UProjectIntimacySubsystem::IsPleaseActive() const
{
	return bSessionActive && ActiveSession.bPleaseActive;
}

FString UProjectIntimacySubsystem::GetActivePartnerId() const
{
	return bSessionActive ? ActiveSession.PartnerId : FString();
}

float UProjectIntimacySubsystem::GetCurrentEncounterLust() const
{
	return bSessionActive ? ActiveSession.CurrentLust : 0.0f;
}

int32 UProjectIntimacySubsystem::GetCurrentControlPoints() const
{
	return bSessionActive ? ActiveSession.CurrentControlPoints : 0;
}

EProjectIntimacyControlState UProjectIntimacySubsystem::GetCurrentControlState() const
{
	return UProjectIntimacySettings::GetControlState(GetCurrentControlPoints());
}

float UProjectIntimacySubsystem::GetTalkCooldownRemaining() const
{
	return bSessionActive ? ActiveSession.TalkCooldownRemaining : 0.0f;
}

int32 UProjectIntimacySubsystem::GetTotalIntimacyEncounterCount() const
{
	int32 TotalEncounters = 0;
	if (!IntimacySaveGame)
	{
		return TotalEncounters;
	}

	for (const TPair<FString, FProjectIntimacyPartnerProfile>& ProfilePair : IntimacySaveGame->PartnerProfiles)
	{
		TotalEncounters += FMath::Max(0, ProfilePair.Value.Encounters);
	}

	return TotalEncounters;
}

bool UProjectIntimacySubsystem::HasAnyIntimacyEncounter() const
{
	return GetTotalIntimacyEncounterCount() > 0;
}

bool UProjectIntimacySubsystem::IsAutomaticTattooRewardUnlocked(const FName TattooRewardId) const
{
	if (TattooRewardId.IsNone())
	{
		return false;
	}

	return RuntimeUnlockedAutomaticTattooIds.Contains(TattooRewardId);
}

bool UProjectIntimacySubsystem::UnlockAutomaticTattooReward(const FName TattooRewardId)
{
	if (TattooRewardId.IsNone())
	{
		return false;
	}

	const int32 PreviousCount = RuntimeUnlockedAutomaticTattooIds.Num();
	RuntimeUnlockedAutomaticTattooIds.Add(TattooRewardId);
	const bool bAdded = RuntimeUnlockedAutomaticTattooIds.Num() != PreviousCount;
	if (bAdded)
	{
		UE_LOG(LogProjectIntimacy, Display, TEXT("[ProjectIntimacy] runtime_automatic_tattoo_reward_unlocked id=%s"), *TattooRewardId.ToString());
	}

	return true;
}

bool UProjectIntimacySubsystem::GrantFirstIntimacyEncounterForAutomation()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	LoadPersistentState();
	if (!IntimacySaveGame)
	{
		return false;
	}

	const FString PartnerId(TEXT("TattooShopAutoTattooQA"));
	FProjectIntimacyPartnerProfile& Profile = IntimacySaveGame->PartnerProfiles.FindOrAdd(PartnerId);
	Profile.PartnerId = PartnerId;
	Profile.Encounters = FMath::Max(1, Profile.Encounters);
	Profile.bHasFirstEncounter = true;
	if (Profile.FirstEncounterUtc.GetTicks() == 0)
	{
		Profile.FirstEncounterUtc = FDateTime::UtcNow();
	}
	RefreshRelationshipTags(Profile);
	UnlockAutomaticTattooReward(TestTattooIntimacyRewardId);
	SavePersistentState();
	return true;
#endif
}

bool UProjectIntimacySubsystem::ForcePartnerClimaxForAutomation()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	if (!bSessionActive)
	{
		return false;
	}

	UProjectIntimacyPartnerComponent* PartnerComponent = ActiveSession.PartnerComponent.Get();
	if (!PartnerComponent)
	{
		return false;
	}

	FProjectIntimacyPartnerProfile& Profile = GetMutableProfile(PartnerComponent);
	const int32 PreviousClimaxCount = Profile.ClimaxCount;
	const float RequiredDrain = FMath::Max(
		1.0f,
		ActiveSession.ClimaxThreshold - ActiveSession.ClimaxProgress + 1.0f);
	ActiveSession.MaxLust = FMath::Max(ActiveSession.MaxLust, RequiredDrain + 10.0f);
	ActiveSession.CurrentLust = FMath::Max(ActiveSession.CurrentLust, RequiredDrain + 10.0f);
	ApplyLustDrain(RequiredDrain, FText::FromString(TEXT("Automation climax")), false);
	return Profile.ClimaxCount > PreviousClimaxCount;
#endif
}

void UProjectIntimacySubsystem::RequestToggleHud()
{
	if (!bSessionActive)
	{
		return;
	}

	ActiveSession.bHudVisible = !ActiveSession.bHudVisible;
	RefreshHudWidget();
}

bool UProjectIntimacySubsystem::RequestQuickStartIntimacy()
{
	if (bSessionActive)
	{
		return false;
	}

	TryResolveRuntimeContext();
	if (!TrackedPlayerPawn)
	{
		UE_LOG(LogProjectIntimacy, Warning, TEXT("[ProjectIntimacy] Quick start failed: no tracked player pawn."));
		return false;
	}

	if (!TrackedTargetingFixComponent)
	{
		UE_LOG(LogProjectIntimacy, Warning, TEXT("[ProjectIntimacy] Quick start failed: no targeting component. Select/observe a target with T first."));
		return false;
	}

	AActor* TargetActor = TrackedTargetingFixComponent->GetCurrentTargetActor();
	if (!IsValid(TargetActor) || TargetActor == TrackedPlayerPawn)
	{
		UE_LOG(LogProjectIntimacy, Warning, TEXT("[ProjectIntimacy] Quick start failed: no valid target. Select/observe a target with T first."));
		return false;
	}

	UWorld* World = GetWorld();
	UProjectEmoteSubsystem* EmoteSubsystem = World ? World->GetSubsystem<UProjectEmoteSubsystem>() : nullptr;
	if (!EmoteSubsystem)
	{
		UE_LOG(LogProjectIntimacy, Warning, TEXT("[ProjectIntimacy] Quick start failed: no ProjectEmote subsystem."));
		return false;
	}

	if (EmoteSubsystem->IsRuntimeActionActive())
	{
		UE_LOG(LogProjectIntimacy, Warning, TEXT("[ProjectIntimacy] Quick start failed: another ProjectEmote runtime action is active."));
		return false;
	}

	FProjectEmoteRuntimeActionRequest RuntimeActionRequest;
	RuntimeActionRequest.RuntimeActionId = ProjectIntimacySubsystemPrivate::IntimacySceneId;
	RuntimeActionRequest.InteractionId = ProjectIntimacySubsystemPrivate::IntimacySceneId;
	RuntimeActionRequest.Source = EProjectEmoteRuntimeActionSource::Interaction;
	RuntimeActionRequest.bAllowCancel = true;
	RuntimeActionRequest.bCancelWithY = true;
	RuntimeActionRequest.bRestoreMovementOnEnd = true;
	RuntimeActionRequest.bHiddenFromMenu = true;
	if (EmoteSubsystem->StartRuntimeAction(RuntimeActionRequest))
	{
		return true;
	}

	return TrackedEmoteComponent
		? TrackedEmoteComponent->StartRuntimeInteractionById(ProjectIntimacySubsystemPrivate::IntimacySceneId)
		: false;
}

void UProjectIntimacySubsystem::RequestNavigate(const int32 Direction)
{
	if (!bSessionActive || !ActiveSession.bHudVisible || ResolvedOptions.Num() <= 0)
	{
		return;
	}

	ActiveSession.SelectedOptionIndex = (ActiveSession.SelectedOptionIndex + Direction) % ResolvedOptions.Num();
	if (ActiveSession.SelectedOptionIndex < 0)
	{
		ActiveSession.SelectedOptionIndex += ResolvedOptions.Num();
	}
	RefreshHudWidget();
}

void UProjectIntimacySubsystem::RequestBack()
{
	if (!bSessionActive || !ActiveSession.bHudVisible)
	{
		return;
	}

	if (ActiveSession.HudMode == EProjectIntimacyHudMode::Main)
	{
		ActiveSession.bHudVisible = false;
	}
	else if (ActiveSession.HudMode == EProjectIntimacyHudMode::Talk && !ActiveSession.ActiveTalkCategoryId.IsNone())
	{
		ActiveSession.ActiveTalkCategoryId = NAME_None;
		ActiveSession.StatusText = FText::FromString(TEXT("Choose a Talk style."));
		RefreshResolvedOptions();
		RefreshHudWidget();
	}
	else if (ActiveSession.HudMode == EProjectIntimacyHudMode::Items && !ActiveSession.ActiveItemCategoryId.IsNone())
	{
		ActiveSession.ActiveItemCategoryId = NAME_None;
		ActiveSession.StatusText = FText::FromString(TEXT("Choose an item category."));
		RefreshResolvedOptions();
		RefreshHudWidget();
	}
	else
	{
		SetHudMode(EProjectIntimacyHudMode::Main);
	}
}

void UProjectIntimacySubsystem::RequestConfirm()
{
	if (!bSessionActive || !ActiveSession.bHudVisible)
	{
		return;
	}

	if (ActiveSession.bPleaseActive)
	{
		ResolvePleasePress();
		return;
	}

	if (!ResolvedOptions.IsValidIndex(ActiveSession.SelectedOptionIndex))
	{
		return;
	}

	const FProjectIntimacyResolvedOption Option = ResolvedOptions[ActiveSession.SelectedOptionIndex];
	if (ActiveSession.HudMode == EProjectIntimacyHudMode::Main)
	{
		HandleMainOption(Option.OptionId);
	}
	else if (ActiveSession.HudMode == EProjectIntimacyHudMode::Talk)
	{
		HandleTalkOption(Option);
	}
	else if (ActiveSession.HudMode == EProjectIntimacyHudMode::Items)
	{
		HandleItemsOption(Option);
	}
}

void UProjectIntimacySubsystem::RequestCancelIntimacy()
{
	CancelActiveSession();
}

#if WITH_DEV_AUTOMATION_TESTS
bool UProjectIntimacySubsystem::AutomationRunMenuAndPleaseSmoke(
	FString& OutFailureReason,
	int32& OutStepCount,
	bool& bOutPleaseCompleted,
	bool& bOutClimaxTriggered)
{
	OutFailureReason.Reset();
	OutStepCount = 0;
	bOutPleaseCompleted = false;
	bOutClimaxTriggered = false;

	auto Fail = [&](const FString& Reason) -> bool
	{
		OutFailureReason = Reason;
		RefreshHudWidget();
		return false;
	};

	if (!bSessionActive)
	{
		return Fail(TEXT("Intimacy session is not active."));
	}

	if (!ActiveSession.bHudVisible)
	{
		RequestToggleHud();
		++OutStepCount;
	}

	RefreshResolvedOptions();
	auto FindOptionIndex = [this](const FName OptionId) -> int32
	{
		for (int32 Index = 0; Index < ResolvedOptions.Num(); ++Index)
		{
			if (ResolvedOptions[Index].OptionId == OptionId)
			{
				return Index;
			}
		}
		return INDEX_NONE;
	};

	auto SelectOption = [&](const FName OptionId, const TCHAR* StepName) -> bool
	{
		RefreshResolvedOptions();
		const int32 OptionIndex = FindOptionIndex(OptionId);
		if (!ResolvedOptions.IsValidIndex(OptionIndex))
		{
			OutFailureReason = FString::Printf(TEXT("%s option %s was not available."), StepName, *OptionId.ToString());
			RefreshHudWidget();
			return false;
		}

		ActiveSession.SelectedOptionIndex = OptionIndex;
		RequestConfirm();
		++OutStepCount;
		return true;
	};

	if (FindOptionIndex(TEXT("Main.Talk")) == INDEX_NONE || FindOptionIndex(TEXT("Main.Please")) == INDEX_NONE)
	{
		return Fail(TEXT("Intimacy main menu did not expose Talk and Please."));
	}

	if (FindOptionIndex(TEXT("Main.Items")) != INDEX_NONE)
	{
		if (!SelectOption(TEXT("Main.Items"), TEXT("Items")))
		{
			return Fail(OutFailureReason);
		}
		if (ActiveSession.HudMode != EProjectIntimacyHudMode::Items)
		{
			return Fail(TEXT("Items menu did not open."));
		}
		if (FindOptionIndex(TEXT("Items.Category.Drugs")) != INDEX_NONE)
		{
			if (!SelectOption(TEXT("Items.Category.Drugs"), TEXT("Items category")))
			{
				return Fail(OutFailureReason);
			}
			RequestBack();
			++OutStepCount;
		}
		RequestBack();
		++OutStepCount;
	}

	SetHudMode(EProjectIntimacyHudMode::Main);
	if (!SelectOption(TEXT("Main.Talk"), TEXT("Talk")))
	{
		return Fail(OutFailureReason);
	}
	if (ActiveSession.HudMode != EProjectIntimacyHudMode::Talk)
	{
		return Fail(TEXT("Talk menu did not open."));
	}

	const FName PreferredTalkCategories[] =
	{
		FName(TEXT("Talk.Category.Submissive")),
		FName(TEXT("Talk.Category.Dominant")),
		FName(TEXT("Talk.Category.Neutral")),
		FName(TEXT("Talk.Category.Intensity"))
	};

	FName SelectedTalkCategory = NAME_None;
	for (const FName CategoryId : PreferredTalkCategories)
	{
		if (FindOptionIndex(CategoryId) != INDEX_NONE)
		{
			SelectedTalkCategory = CategoryId;
			break;
		}
	}
	if (SelectedTalkCategory.IsNone())
	{
		return Fail(TEXT("Talk menu did not expose any usable category."));
	}
	if (!SelectOption(SelectedTalkCategory, TEXT("Talk category")))
	{
		return Fail(OutFailureReason);
	}
	if (ActiveSession.ActiveTalkCategoryId.IsNone())
	{
		return Fail(TEXT("Talk category selection did not enter a talk option list."));
	}

	RefreshResolvedOptions();
	int32 TalkOptionIndex = INDEX_NONE;
	if (!ActiveSession.CorrectTalkOptionId.IsNone())
	{
		TalkOptionIndex = FindOptionIndex(ActiveSession.CorrectTalkOptionId);
	}
	if (!ResolvedOptions.IsValidIndex(TalkOptionIndex))
	{
		for (int32 Index = 0; Index < ResolvedOptions.Num(); ++Index)
		{
			const FProjectIntimacyResolvedOption& Option = ResolvedOptions[Index];
			if (Option.OptionId != TEXT("Talk.Category.Back")
				&& Option.OptionId != TEXT("Talk.Back")
				&& Option.TalkAction != EProjectIntimacyTalkAction::Back)
			{
				TalkOptionIndex = Index;
				break;
			}
		}
	}
	if (!ResolvedOptions.IsValidIndex(TalkOptionIndex))
	{
		return Fail(TEXT("Talk category did not expose a playable talk option."));
	}

	ActiveSession.SelectedOptionIndex = TalkOptionIndex;
	RequestConfirm();
	++OutStepCount;
	RequestBack();
	++OutStepCount;
	SetHudMode(EProjectIntimacyHudMode::Main);

	if (!SelectOption(TEXT("Main.Please"), TEXT("Please")))
	{
		return Fail(OutFailureReason);
	}
	if (!ActiveSession.bPleaseActive)
	{
		return Fail(TEXT("Please minigame did not start."));
	}

	const int32 AttemptCount = UProjectIntimacySettings::Get()
		? FMath::Max(1, UProjectIntimacySettings::Get()->PleaseAttemptCount)
		: 5;
	const int32 MaxPresses = AttemptCount + 2;
	for (int32 PressIndex = 0; ActiveSession.bPleaseActive && PressIndex < MaxPresses; ++PressIndex)
	{
		ActiveSession.PleaseCursorValue = ActiveSession.PleaseTargetCenter;
		RequestConfirm();
		++OutStepCount;
	}

	bOutPleaseCompleted = !ActiveSession.bPleaseActive && ActiveSession.PleaseSuccessCount > 0;
	if (!bOutPleaseCompleted)
	{
		return Fail(TEXT("Please minigame did not complete with a successful hit."));
	}

	if (UProjectIntimacyPartnerComponent* PartnerComponent = ActiveSession.PartnerComponent.Get())
	{
		FProjectIntimacyPartnerProfile& Profile = GetMutableProfile(PartnerComponent);
		const int32 PreviousClimaxCount = Profile.ClimaxCount;
		const float RequiredDrain = FMath::Max(
			1.0f,
			ActiveSession.ClimaxThreshold - ActiveSession.ClimaxProgress + 1.0f);
		ActiveSession.MaxLust = FMath::Max(ActiveSession.MaxLust, RequiredDrain + 10.0f);
		ActiveSession.CurrentLust = FMath::Max(ActiveSession.CurrentLust, RequiredDrain + 10.0f);
		ApplyLustDrain(RequiredDrain, FText::FromString(TEXT("Automation climax")), false);
		bOutClimaxTriggered = Profile.ClimaxCount > PreviousClimaxCount;
		++OutStepCount;
	}

	if (!bOutClimaxTriggered)
	{
		return Fail(TEXT("Automation climax did not trigger."));
	}

	RefreshResolvedOptions();
	RefreshHudWidget();
	return true;
}
#endif

FProjectIntimacySessionSnapshot UProjectIntimacySubsystem::BuildSnapshot() const
{
	FProjectIntimacySessionSnapshot Snapshot;
	Snapshot.bActive = bSessionActive;
	Snapshot.bHudVisible = bSessionActive && ActiveSession.bHudVisible;
	Snapshot.HudMode = ActiveSession.HudMode;
	Snapshot.CurrentLust = ActiveSession.CurrentLust;
	Snapshot.MaxLust = FMath::Max(1.0f, ActiveSession.MaxLust);
	Snapshot.LustDrainPerSecond = ActiveSession.LustDrainPerSecond;
	Snapshot.ControlPoints = ActiveSession.CurrentControlPoints;
	Snapshot.ControlState = UProjectIntimacySettings::GetControlState(ActiveSession.CurrentControlPoints);
	Snapshot.ControlStateTag = UProjectIntimacySettings::GetControlStateTag(ActiveSession.CurrentControlPoints);
	Snapshot.ControlStateText = UProjectIntimacySettings::GetControlStateText(ActiveSession.CurrentControlPoints);
	Snapshot.RelationshipText = FText::FromString(TEXT("Unknown"));
	Snapshot.GenderText = FText::FromString(TEXT("Unknown"));
	Snapshot.TalkCooldownRemaining = ActiveSession.TalkCooldownRemaining;
	Snapshot.CorrectTalkOptionId = ActiveSession.CorrectTalkOptionId;
	Snapshot.bPleaseActive = ActiveSession.bPleaseActive;
	Snapshot.PleaseAttemptIndex = ActiveSession.PleaseAttemptIndex;
	Snapshot.PleaseAttemptCount = UProjectIntimacySettings::Get()
		? UProjectIntimacySettings::Get()->PleaseAttemptCount
		: 5;
	Snapshot.PleaseSuccessCount = ActiveSession.PleaseSuccessCount;
	Snapshot.PleaseCursorValue = ActiveSession.PleaseCursorValue;
	Snapshot.PleaseTargetCenter = ActiveSession.PleaseTargetCenter;
	Snapshot.PleaseTargetHalfRange = ActiveSession.PleaseTargetHalfRange;
	Snapshot.PleasePreviewDrain = UProjectIntimacySettings::ComputePleaseInstantDrain(ResolveAllureLevel(), ActiveSession.PleaseSuccessCount);
	Snapshot.SelectedOptionIndex = ActiveSession.SelectedOptionIndex;
	Snapshot.StatusText = ActiveSession.StatusText;
	Snapshot.HintText = FText::FromString(TEXT("Arrows navigate. Space confirms. '-' hides."));

	if (const UProjectIntimacyPartnerComponent* PartnerComponent = ActiveSession.PartnerComponent.Get())
	{
		Snapshot.PartnerDisplayName = PartnerComponent->GetPartnerDisplayName();
		Snapshot.PartnerId = ActiveSession.PartnerId;
		Snapshot.Personality = PartnerComponent->GetResolvedPersonality();
		Snapshot.EffectivePersonality = ActiveSession.EffectivePersonality;
		Snapshot.GenderTag = PartnerComponent->GetResolvedGenderTag();
		Snapshot.GenderText = UProjectIntimacyPartnerComponent::GenderTagToText(Snapshot.GenderTag);
	}

	if (IntimacySaveGame)
	{
		if (const FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId))
		{
			Snapshot.Relationship = Profile->Relationship;
			Snapshot.RelationshipTags = Profile->RelationshipTags;
			Snapshot.RelationshipText = UProjectIntimacyPartnerComponent::RelationshipTagsToText(Profile->RelationshipTags);
			Snapshot.GenderTag = Profile->GenderTag.IsValid() ? Profile->GenderTag : Snapshot.GenderTag;
			Snapshot.GenderText = UProjectIntimacyPartnerComponent::GenderTagToText(Snapshot.GenderTag);
			Snapshot.Affect = Profile->Affect;
			Snapshot.SatisfiedWins = Profile->SatisfiedWins;
			Snapshot.Encounters = Profile->Encounters;
			Snapshot.Children = Profile->Children;
			Snapshot.FailedEncounters = Profile->FailedEncounters;
			Snapshot.TotalIntimateTimeSeconds = Profile->TotalIntimateTimeSeconds;
			Snapshot.bProfileHistoryVisible = Profile->bHasFirstEncounter;
		}
	}

	Snapshot.Options.Reserve(ResolvedOptions.Num());
	for (const FProjectIntimacyResolvedOption& Option : ResolvedOptions)
	{
		FProjectIntimacyHudOption& HudOption = Snapshot.Options.AddDefaulted_GetRef();
		HudOption.OptionId = Option.OptionId;
		HudOption.Label = Option.Label;
	}

	return Snapshot;
}

bool UProjectIntimacySubsystem::TryGetPartnerProfile(AActor* PartnerActor, FProjectIntimacyPartnerProfile& OutProfile) const
{
	const UProjectIntimacyPartnerComponent* PartnerComponent = PartnerActor
		? PartnerActor->FindComponentByClass<UProjectIntimacyPartnerComponent>()
		: nullptr;
	if (!PartnerComponent || !IntimacySaveGame)
	{
		return false;
	}

	if (const FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame->PartnerProfiles.Find(PartnerComponent->GetResolvedPartnerId()))
	{
		OutProfile = *Profile;
		return true;
	}

	return false;
}

void UProjectIntimacySubsystem::AppendTargetIntimacyRows(AActor* PartnerActor, FProjectEnemyCombatStatSnapshot& InOutSnapshot)
{
	InOutSnapshot.Rows.Reset();

	UProjectIntimacyPartnerComponent* PartnerComponent = UProjectIntimacyPartnerComponent::FindOrCreateForActor(PartnerActor);
	if (!PartnerComponent)
	{
		return;
	}

	const FString PartnerId = PartnerComponent->GetResolvedPartnerId();
	LoadPersistentState();
	FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(PartnerId) : nullptr;
	if (Profile)
	{
		NormalizeProfile(PartnerComponent, *Profile);
		RefreshRelationshipTags(*Profile);
	}

	TArray<FProjectSocialCardRow> DefaultSocialRows;
	UProjectIntimacyDialogueLibrary::BuildFallbackSocialCardRows(DefaultSocialRows);

	TSet<FName> SupportedValueIds;
	for (const FProjectSocialCardRow& DefaultRow : DefaultSocialRows)
	{
		SupportedValueIds.Add(!DefaultRow.ValueId.IsNone() ? DefaultRow.ValueId : DefaultRow.RowId);
	}

	TArray<FProjectSocialCardRow> SocialRows;
	if (const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get())
	{
		if (UDataTable* SocialRowsTable = LoadTable(Settings->SocialCardRowsTable))
		{
			if (SocialRowsTable->GetRowStruct() == FProjectSocialCardRow::StaticStruct())
			{
				TArray<FProjectSocialCardRow*> TableRows;
				SocialRowsTable->GetAllRows(TEXT("ProjectSocialCardRows"), TableRows);
				for (const FProjectSocialCardRow* Row : TableRows)
				{
					if (Row)
					{
						const FName RowValueId = !Row->ValueId.IsNone() ? Row->ValueId : Row->RowId;
						if (SupportedValueIds.Contains(RowValueId))
						{
							SocialRows.Add(*Row);
						}
					}
				}
			}
		}
	}

	if (SocialRows.Num() <= 0)
	{
		SocialRows = DefaultSocialRows;
	}
	else if (!SocialRows.ContainsByPredicate([](const FProjectSocialCardRow& Row)
	{
		const FName ValueId = !Row.ValueId.IsNone() ? Row.ValueId : Row.RowId;
		return ValueId == TEXT("ClimaxCount");
	}))
	{
		if (const FProjectSocialCardRow* ClimaxRow = DefaultSocialRows.FindByPredicate([](const FProjectSocialCardRow& Row)
		{
			const FName ValueId = !Row.ValueId.IsNone() ? Row.ValueId : Row.RowId;
			return ValueId == TEXT("ClimaxCount");
		}))
		{
			SocialRows.Add(*ClimaxRow);
		}
	}

	SocialRows.Sort([](const FProjectSocialCardRow& Left, const FProjectSocialCardRow& Right)
	{
		if (Left.SortOrder != Right.SortOrder)
		{
			return Left.SortOrder < Right.SortOrder;
		}
		return Left.RowId.LexicalLess(Right.RowId);
	});

	const bool bHasHistory = Profile && Profile->bHasFirstEncounter;
	const EProjectIntimacyPersonality EffectivePersonality = Profile
		? ResolveEffectivePersonality(*Profile, PartnerComponent)
		: PartnerComponent->GetResolvedPersonality();
	const int32 ControlPoints = Profile
		? Profile->ControlPoints
		: UProjectIntimacySettings::ComputeInitialControl(EffectivePersonality);
	const FGameplayTag GenderTag = Profile && Profile->GenderTag.IsValid()
		? Profile->GenderTag
		: PartnerComponent->GetResolvedGenderTag();

	auto ResolveValue = [this, PartnerComponent, Profile, EffectivePersonality, ControlPoints, GenderTag](const FName ValueId, FString& OutValue) -> bool
	{
		if (ValueId == TEXT("Lust"))
		{
			OutValue = FString::Printf(TEXT("%.0f"), PartnerComponent->GetMaxLust());
			return true;
		}
		if (ValueId == TEXT("Gender"))
		{
			OutValue = UProjectIntimacyPartnerComponent::GenderTagToText(GenderTag).ToString();
			return true;
		}
		if (ValueId == TEXT("Personality"))
		{
			OutValue = UProjectIntimacyPartnerComponent::PersonalityToText(EffectivePersonality).ToString();
			return true;
		}
		if (ValueId == TEXT("Control"))
		{
			OutValue = FString::Printf(
				TEXT("%s (%d)"),
				*UProjectIntimacySettings::GetControlStateText(ControlPoints).ToString(),
				ControlPoints);
			return true;
		}
		if (!Profile)
		{
			return false;
		}
		if (ValueId == TEXT("Relationship"))
		{
			OutValue = UProjectIntimacyPartnerComponent::RelationshipTagsToText(Profile->RelationshipTags).ToString();
			return true;
		}
		if (ValueId == TEXT("Affect"))
		{
			OutValue = FString::Printf(TEXT("%d"), Profile->Affect);
			return true;
		}
		if (ValueId == TEXT("Encounters"))
		{
			OutValue = FString::Printf(TEXT("%d"), Profile->Encounters);
			return true;
		}
		if (ValueId == TEXT("SatisfiedWins"))
		{
			OutValue = FString::Printf(TEXT("%d"), Profile->SatisfiedWins);
			return true;
		}
		if (ValueId == TEXT("FailedEncounters"))
		{
			OutValue = FString::Printf(TEXT("%d"), Profile->FailedEncounters);
			return true;
		}
		if (ValueId == TEXT("ClimaxCount"))
		{
			OutValue = FString::Printf(TEXT("%d"), Profile->ClimaxCount);
			return true;
		}
		if (ValueId == TEXT("Children"))
		{
			OutValue = FString::Printf(TEXT("%d"), Profile->Children);
			return true;
		}
		if (ValueId == TEXT("FirstEncounter"))
		{
			OutValue = Profile->FirstEncounterUtc.GetTicks() > 0
				? Profile->FirstEncounterUtc.ToString(TEXT("%Y-%m-%d"))
				: TEXT("--");
			return true;
		}
		if (ValueId == TEXT("TotalIntimateTime"))
		{
			OutValue = ProjectIntimacySubsystemPrivate::FormatDuration(Profile->TotalIntimateTimeSeconds);
			return true;
		}
		if (ValueId == TEXT("Ally"))
		{
			OutValue = Profile->bConvertedToAlly ? TEXT("Yes") : TEXT("No");
			return true;
		}
		return false;
	};

	TSet<FName> AddedValueIds;
	for (const FProjectSocialCardRow& SocialRow : SocialRows)
	{
		if (!SocialRow.bEnabled)
		{
			continue;
		}
		if (bHasHistory)
		{
			if (!SocialRow.bShowAfterFirstEncounter)
			{
				continue;
			}
		}
		else if (!SocialRow.bShowBeforeFirstEncounter)
		{
			continue;
		}

		const FName ValueId = !SocialRow.ValueId.IsNone() ? SocialRow.ValueId : SocialRow.RowId;
		if (!SupportedValueIds.Contains(ValueId) || AddedValueIds.Contains(ValueId))
		{
			continue;
		}

		FString Value;
		if (!ResolveValue(ValueId, Value))
		{
			continue;
		}

		AddedValueIds.Add(ValueId);
		FProjectEnemyCombatStatRow& Row = InOutSnapshot.Rows.AddDefaulted_GetRef();
		Row.Label = !SocialRow.Label.IsEmpty() ? SocialRow.Label : FText::FromName(ValueId);
		Row.Section = NAME_None;
		Row.ValueOverride = FText::FromString(Value);
		Row.bIsAvailable = true;
	}
}

bool UProjectIntimacySubsystem::BuildTargetSocialCardSnapshot(AActor* PartnerActor, FProjectSocialCardSnapshot& OutSnapshot)
{
	OutSnapshot.Rows.Reset();

	FProjectEnemyCombatStatSnapshot LegacySnapshot;
	AppendTargetIntimacyRows(PartnerActor, LegacySnapshot);
	OutSnapshot.Rows = MoveTemp(LegacySnapshot.Rows);
	return OutSnapshot.Rows.Num() > 0;
}

void UProjectIntimacySubsystem::TryResolveRuntimeContext()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (PlayerController != TrackedPlayerController)
	{
		AttachToPlayerController(PlayerController);
	}

	TrackedPlayerPawn = TrackedPlayerController ? TrackedPlayerController->GetPawn() : nullptr;
	TrackedEmoteComponent = TrackedPlayerPawn ? TrackedPlayerPawn->FindComponentByClass<UProjectEmoteComponent>() : nullptr;
	TrackedTargetingFixComponent = TrackedPlayerPawn ? TrackedPlayerPawn->FindComponentByClass<UProjectTargetingFixComponent>() : nullptr;
}

void UProjectIntimacySubsystem::AttachToPlayerController(APlayerController* PlayerController)
{
	DetachFromTrackedPlayerController();
	TrackedPlayerController = PlayerController;
	if (TrackedPlayerController && TrackedPlayerController->IsLocalController())
	{
		BindInputToTrackedPlayerController();
	}
}

void UProjectIntimacySubsystem::DetachFromTrackedPlayerController()
{
	UnbindInputFromTrackedPlayerController();
	if (HudWidget)
	{
		HudWidget->RemoveFromParent();
		HudWidget = nullptr;
	}
	TrackedPlayerController = nullptr;
	TrackedPlayerPawn = nullptr;
	TrackedEmoteComponent = nullptr;
	TrackedTargetingFixComponent = nullptr;
}

void UProjectIntimacySubsystem::BindInputToTrackedPlayerController()
{
	if (!TrackedPlayerController || IntimacyInputComponent)
	{
		return;
	}

	IntimacyInputComponent = NewObject<UInputComponent>(TrackedPlayerController, TEXT("ProjectIntimacyInputComponent"));
	if (!IntimacyInputComponent)
	{
		return;
	}

	IntimacyInputComponent->bBlockInput = false;
	IntimacyInputComponent->Priority = ProjectIntimacySubsystemPrivate::InputPriority;
	IntimacyInputComponent->RegisterComponent();

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	auto BindKey = [this](const FKey& Key, void (UProjectIntimacySubsystem::*Handler)())
	{
		if (Key.IsValid())
		{
			FInputKeyBinding& Binding = IntimacyInputComponent->BindKey(Key, IE_Pressed, this, Handler);
			Binding.bConsumeInput = true;
		}
	};

	BindKey(Settings ? Settings->HudToggleKey : EKeys::Hyphen, &ThisClass::HandleToggleHudPressed);
	BindKey(Settings ? Settings->HudSecondaryToggleKey : EKeys::Subtract, &ThisClass::HandleToggleHudPressed);
	BindKey(EKeys::Up, &ThisClass::HandleNavigateUpPressed);
	BindKey(EKeys::Down, &ThisClass::HandleNavigateDownPressed);
	BindKey(EKeys::Left, &ThisClass::HandleNavigateLeftPressed);
	BindKey(EKeys::Right, &ThisClass::HandleNavigateRightPressed);
	BindKey(EKeys::SpaceBar, &ThisClass::HandleConfirmPressed);
	BindKey(EKeys::Enter, &ThisClass::HandleConfirmPressed);

	TrackedPlayerController->PushInputComponent(IntimacyInputComponent);
}

void UProjectIntimacySubsystem::UnbindInputFromTrackedPlayerController()
{
	if (TrackedPlayerController && IntimacyInputComponent)
	{
		TrackedPlayerController->PopInputComponent(IntimacyInputComponent);
	}

	if (IntimacyInputComponent && IntimacyInputComponent->IsRegistered())
	{
		IntimacyInputComponent->DestroyComponent();
	}
	IntimacyInputComponent = nullptr;
}

bool UProjectIntimacySubsystem::ResolveActiveIntimacyPartner(
	AActor*& OutPartnerActor,
	UProjectIntimacyPartnerComponent*& OutPartnerComponent) const
{
	OutPartnerActor = nullptr;
	OutPartnerComponent = nullptr;

	if (!TrackedPlayerPawn || !TrackedEmoteComponent || !TrackedEmoteComponent->IsEmoteActive())
	{
		return false;
	}

	if (TrackedEmoteComponent->GetActiveInteractionId() != ProjectIntimacySubsystemPrivate::IntimacySceneId)
	{
		return false;
	}

	OutPartnerActor = TrackedEmoteComponent->GetActiveBlueprintSceneTargetActor();
	if (!OutPartnerActor && TrackedTargetingFixComponent)
	{
		OutPartnerActor = TrackedTargetingFixComponent->GetCurrentTargetActor();
	}

	if (!IsValid(OutPartnerActor) || OutPartnerActor == TrackedPlayerPawn)
	{
		return false;
	}

	OutPartnerComponent = UProjectIntimacyPartnerComponent::FindOrCreateForActor(OutPartnerActor);
	return OutPartnerComponent != nullptr;
}

void UProjectIntimacySubsystem::StartSession(AActor* PartnerActor, UProjectIntimacyPartnerComponent* PartnerComponent)
{
	if (!PartnerActor || !PartnerComponent)
	{
		return;
	}

	ActiveSession = FProjectIntimacyRuntimeSession();
	ActiveSession.PartnerActor = PartnerActor;
	ActiveSession.PartnerComponent = PartnerComponent;
	ActiveSession.PartnerId = PartnerComponent->GetResolvedPartnerId();
	ActiveSession.MaxLust = PartnerComponent->GetMaxLust();
	ActiveSession.CurrentLust = ActiveSession.MaxLust;
	ActiveSession.ClimaxThreshold = UProjectIntimacySettings::ComputeClimaxThreshold(PartnerComponent->GetPartnerLevel());
	ActiveSession.ClimaxProgress = 0.0f;
	ActiveSession.ClimaxMultiplier = 1.0f;
	ActiveSession.ClimaxRecoveryRemaining = 0.0f;
	ActiveSession.StatusText = FText::FromString(TEXT("Session started."));

	FProjectIntimacyPartnerProfile& Profile = GetMutableProfile(PartnerComponent);
	Profile.Encounters += 1;
	Profile.bHasFirstEncounter = true;
	if (Profile.FirstEncounterUtc.GetTicks() == 0)
	{
		Profile.FirstEncounterUtc = FDateTime::UtcNow();
	}
	RefreshRelationshipTags(Profile);
	UnlockAutomaticTattooReward(TestTattooIntimacyRewardId);
	ActiveSession.EffectivePersonality = ResolveEffectivePersonality(Profile, PartnerComponent);
	if (RelationshipForcesChill(Profile.RelationshipTags))
	{
		const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
		Profile.ControlPoints = FMath::Min(
			Profile.ControlPoints,
			Settings ? Settings->RelationshipChillMaxControl : 199);
	}
	ActiveSession.CurrentControlPoints = UProjectIntimacySettings::ClampControlPoints(Profile.ControlPoints);

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	switch (ActiveSession.EffectivePersonality)
	{
	case EProjectIntimacyPersonality::Chill:
		ActiveSession.AnimationRate = Settings ? Settings->ChillAnimationRate : 0.75f;
		break;
	case EProjectIntimacyPersonality::Stallion:
		ActiveSession.AnimationRate = Settings ? Settings->StallionAnimationRate : 1.35f;
		break;
	case EProjectIntimacyPersonality::Nice:
	case EProjectIntimacyPersonality::Auto:
	default:
		ActiveSession.AnimationRate = Settings ? Settings->NiceAnimationRate : 1.0f;
		break;
	}

	bSessionActive = true;
	RefreshActiveIntimacyCombatShield();
	ApplyAnimationRate(ActiveSession.AnimationRate);
	RefreshResolvedOptions();
	EnsureHudWidget();
	RefreshHudWidget();
	SavePersistentState();
	AddChronicleSystem(FText::FromString(TEXT("Intimacy session started.")));
}

void UProjectIntimacySubsystem::UpdateActiveSession(const float DeltaTime)
{
	if (!bSessionActive)
	{
		return;
	}

	RefreshActiveIntimacyCombatShield();
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	const int32 AllureLevel = ResolveAllureLevel();
	ActiveSession.LustDrainPerSecond = UProjectIntimacySettings::ComputeAllureDrainPerSecond(AllureLevel);
	ActiveSession.SessionTimeSeconds += DeltaTime;
	ActiveSession.TalkCooldownRemaining = FMath::Max(0.0f, ActiveSession.TalkCooldownRemaining - DeltaTime);
	if (UDirtyPawnComponent* DirtyPawnComponent = UDirtyPawnComponent::FindCanonicalDirtyPawnComponent(TrackedPlayerPawn))
	{
		DirtyPawnComponent->AddSweatPoints(FMath::Max(DirtyPawnComponent->SweatIntimacyGainPerSecond, 0.0f) * DeltaTime);
	}

	if (FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId) : nullptr)
	{
		Profile->TotalIntimateTimeSeconds += DeltaTime;
		Profile->ControlPoints = ActiveSession.CurrentControlPoints;
	}

	ApplyLustDrain(ActiveSession.LustDrainPerSecond * DeltaTime, FText(), true);
	UpdateOverpoweringPulse(DeltaTime);
	UpdateClimaxIntensity(DeltaTime);

	if (ActiveSession.bPleaseActive)
	{
		ActiveSession.PleaseElapsedSeconds += DeltaTime;
		const float Period = FMath::Max(0.01f, ActiveSession.PleasePulsePeriod);
		ActiveSession.PleaseCursorValue = 0.5f + 0.5f * FMath::Sin((ActiveSession.PleaseElapsedSeconds / Period) * UE_TWO_PI);
	}

	ApplyAnimationRate(ActiveSession.AnimationRate);

	if (ActiveSession.CurrentLust <= 0.0f && !ActiveSession.bSatisfied)
	{
		FinishSatisfiedSession();
		return;
	}

	RefreshHudWidget();
}

void UProjectIntimacySubsystem::RefreshActiveIntimacyCombatShield()
{
	if (TrackedEmoteComponent && bSessionActive)
	{
		TrackedEmoteComponent->EnsureIntimacyCombatShield(TrackedPlayerPawn, ActiveSession.PartnerActor.Get());
	}
}

void UProjectIntimacySubsystem::EndSession(const bool bCancelled)
{
	if (!bSessionActive)
	{
		return;
	}

	if (bCancelled && !ActiveSession.bSatisfied)
	{
		if (FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId) : nullptr)
		{
			Profile->FailedEncounters += 1;
			Profile->ControlPoints = ActiveSession.CurrentControlPoints;
			RefreshRelationshipTags(*Profile);
		}
		if (UProjectSurvivalStatusComponent* StatusComponent = EnsureStatusComponent())
		{
			StatusComponent->SetForcedStatusActive(ProjectIntimacySubsystemPrivate::TiredStatusName, true);
		}
		AddChronicleSystem(FText::FromString(TEXT("The intimate minigame was cancelled. Tired applied; no bonus SXP awarded.")));
	}
	else if (FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId) : nullptr)
	{
		Profile->ControlPoints = ActiveSession.CurrentControlPoints;
		RefreshRelationshipTags(*Profile);
	}

	RestoreAnimationRates();
	SavePersistentState();
	bSessionActive = false;
	ActiveSession = FProjectIntimacyRuntimeSession();
	ResolvedOptions.Reset();
	RefreshHudWidget();
}

void UProjectIntimacySubsystem::FinishSatisfiedSession()
{
	if (!bSessionActive || ActiveSession.bSatisfied)
	{
		return;
	}

	ActiveSession.bSatisfied = true;
	if (FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId) : nullptr)
	{
		const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
		Profile->SatisfiedWins += 1;
		Profile->Affect = FMath::Min(
			Settings ? FMath::Max(1, Settings->AffectMax) : 100,
			Profile->Affect + (Settings ? Settings->SatisfiedWinAffectGain : 10));
		Profile->ControlPoints = ActiveSession.CurrentControlPoints;
		RefreshRelationshipTags(*Profile);
	}

	GrantSatisfiedSxp();
	AddChronicleSystem(FText::FromString(TEXT("Partner satisfied. Bonus SXP awarded.")));
	bSuppressStartUntilSceneEnds = true;

	if (UWorld* World = GetWorld())
	{
		if (UProjectEmoteSubsystem* EmoteSubsystem = World->GetSubsystem<UProjectEmoteSubsystem>())
		{
			EmoteSubsystem->RequestCancelActiveEmote();
		}
	}

	EndSession(false);
}

void UProjectIntimacySubsystem::CancelActiveSession()
{
	if (!bSessionActive)
	{
		return;
	}

	bSuppressStartUntilSceneEnds = true;
	if (UWorld* World = GetWorld())
	{
		if (UProjectEmoteSubsystem* EmoteSubsystem = World->GetSubsystem<UProjectEmoteSubsystem>())
		{
			EmoteSubsystem->RequestCancelActiveEmote();
		}
	}

	EndSession(true);
}

void UProjectIntimacySubsystem::EnsureHudWidget()
{
	if (!TrackedPlayerController || HudWidget)
	{
		return;
	}

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	const TSubclassOf<UProjectIntimacyHudWidget> WidgetClass =
		ProjectWidgetClassResolver::ResolveWidgetClass<UProjectIntimacyHudWidget>(
			Settings ? Settings->IntimacyHudWidgetClass : FSoftClassPath(),
			TEXT("ProjectIntimacyHudWidget"));
	TSubclassOf<UUserWidget> ResolvedWidgetClass = WidgetClass;
	if (!ResolvedWidgetClass)
	{
		ResolvedWidgetClass = UProjectIntimacyHudWidget::StaticClass();
	}

	HudWidget = CreateWidget<UProjectIntimacyHudWidget>(
		TrackedPlayerController,
		ResolvedWidgetClass,
		TEXT("ProjectIntimacyHudWidget"));
	if (!HudWidget)
	{
		UE_LOG(LogProjectIntimacy, Warning, TEXT("[Intimacy] Failed to create HUD widget."));
		return;
	}

	const int32 ZOrder = Settings ? Settings->IntimacyHudZOrder : 325;
	if (!HudWidget->AddToPlayerScreen(ZOrder))
	{
		HudWidget->AddToViewport(ZOrder);
	}
}

void UProjectIntimacySubsystem::RefreshHudWidget()
{
	if (HudWidget)
	{
		HudWidget->SetSnapshot(BuildSnapshot());
	}
}

void UProjectIntimacySubsystem::RefreshResolvedOptions()
{
	ResolvedOptions.Reset();

	if (!bSessionActive)
	{
		return;
	}

	if (ActiveSession.HudMode == EProjectIntimacyHudMode::Main)
	{
		ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Main.Please"), TEXT("Please")));
		ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Main.Talk"), TEXT("Talk")));
		ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Main.Items"), TEXT("Items")));
		ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Main.Cancel"), TEXT("Cancel")));
	}
	else if (ActiveSession.HudMode == EProjectIntimacyHudMode::Talk)
	{
		if (ActiveSession.ActiveTalkCategoryId.IsNone())
		{
			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Talk.Category.Intensity"), TEXT("Intensity")));
			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Talk.Category.Dominant"), TEXT("Dominant")));
			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Talk.Category.Submissive"), TEXT("Submissive")));
			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Talk.Category.Neutral"), TEXT("Neutral")));
			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Talk.Back"), TEXT("Back")));
		}
		else
		{
			FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId) : nullptr;
			const bool bAllyEligible = Profile && IsAllyEligible(*Profile);
			const FGameplayTag PersonalityTag = UProjectIntimacyDialogueLibrary::GetPersonalityTag(ActiveSession.EffectivePersonality);

			TArray<FProjectIntimacyTalkOptionRow> TalkRows;
			if (const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get())
			{
				if (UDataTable* TalkTable = LoadTable(Settings->TalkOptionsTable))
				{
					TArray<FProjectIntimacyTalkOptionRow*> TableRows;
					TalkTable->GetAllRows(TEXT("ProjectIntimacyTalkOptions"), TableRows);
					for (const FProjectIntimacyTalkOptionRow* Row : TableRows)
					{
						if (Row)
						{
							TalkRows.Add(*Row);
						}
					}
				}
			}

			if (TalkRows.Num() <= 0)
			{
				UProjectIntimacyDialogueLibrary::BuildFallbackTalkOptions(TalkRows, bAllyEligible);
			}

			int32 AddedRows = 0;
			for (const FProjectIntimacyTalkOptionRow& Row : TalkRows)
			{
				if (Row.CategoryId != ActiveSession.ActiveTalkCategoryId)
				{
					continue;
				}
				if (Row.RequiredPersonalityTag.IsValid()
					&& PersonalityTag.IsValid()
					&& Row.RequiredPersonalityTag != PersonalityTag)
				{
					continue;
				}
				if (Row.bRequiresAllyEligibility && !bAllyEligible)
				{
					continue;
				}
				if (Row.Action == EProjectIntimacyTalkAction::Recruit && !bAllyEligible)
				{
					continue;
				}

				FProjectIntimacyResolvedOption& Option = ResolvedOptions.AddDefaulted_GetRef();
				Option.OptionId = !Row.OptionId.IsNone() ? Row.OptionId : FName(*UEnum::GetValueAsString(Row.Action));
				Option.Label = !Row.Label.IsEmpty() ? Row.Label : FText::FromName(Option.OptionId);
				Option.TalkAction = Row.Action;
				Option.CategoryId = Row.CategoryId;
				Option.TalkTags = Row.TalkTags;
				Option.LustDrain = Row.LustDrain;
				Option.SxpReward = Row.SxpReward;
				Option.ControlDelta = Row.ControlDelta;
				Option.AffectDelta = Row.AffectDelta;
				Option.AnimationRate = Row.AnimationRate;
				Option.bRequiresAllyEligibility = Row.bRequiresAllyEligibility;
				Option.bCanBeCorrectTalkOption = Row.bCanBeCorrectTalkOption;
				Option.bUsesTalkCooldown = Row.bUsesTalkCooldown;
				Option.bCanBeFlavorCorrectOption = Row.bCanBeFlavorCorrectOption;
				++AddedRows;
			}

			if (AddedRows <= 0)
			{
				TArray<FProjectIntimacyTalkOptionRow> FallbackRows;
				UProjectIntimacyDialogueLibrary::BuildFallbackTalkOptions(FallbackRows, bAllyEligible);
				for (const FProjectIntimacyTalkOptionRow& Row : FallbackRows)
				{
					if (Row.CategoryId != ActiveSession.ActiveTalkCategoryId)
					{
						continue;
					}

					FProjectIntimacyResolvedOption& Option = ResolvedOptions.AddDefaulted_GetRef();
					Option.OptionId = Row.OptionId;
					Option.Label = Row.Label;
					Option.TalkAction = Row.Action;
					Option.CategoryId = Row.CategoryId;
					Option.TalkTags = Row.TalkTags;
					Option.LustDrain = Row.LustDrain;
					Option.SxpReward = Row.SxpReward;
					Option.ControlDelta = Row.ControlDelta;
					Option.AffectDelta = Row.AffectDelta;
					Option.AnimationRate = Row.AnimationRate;
					Option.bRequiresAllyEligibility = Row.bRequiresAllyEligibility;
					Option.bCanBeCorrectTalkOption = Row.bCanBeCorrectTalkOption;
					Option.bUsesTalkCooldown = Row.bUsesTalkCooldown;
					Option.bCanBeFlavorCorrectOption = Row.bCanBeFlavorCorrectOption;
				}
			}

			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Talk.Category.Back"), TEXT("Back")));
		}
	}
	else if (ActiveSession.HudMode == EProjectIntimacyHudMode::Items)
	{
		if (ActiveSession.ActiveItemCategoryId.IsNone())
		{
			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Items.Category.Drugs"), TEXT("Drugs")));
			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Items.Category.Toys"), TEXT("Toys")));
			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Items.Back"), TEXT("Back")));
		}
		else
		{
			const FString CategoryName = ActiveSession.ActiveItemCategoryId == TEXT("Items.Category.Toys")
				? FString(TEXT("toys"))
				: FString(TEXT("drugs"));
			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(
				TEXT("Items.Empty"),
				*FString::Printf(TEXT("No %s yet"), *CategoryName)));
			ResolvedOptions.Add(ProjectIntimacySubsystemPrivate::MakeOption(TEXT("Items.Category.Back"), TEXT("Back")));
		}
	}

	ActiveSession.SelectedOptionIndex = FMath::Clamp(ActiveSession.SelectedOptionIndex, 0, FMath::Max(0, ResolvedOptions.Num() - 1));
	if (ActiveSession.HudMode == EProjectIntimacyHudMode::Talk && !ActiveSession.ActiveTalkCategoryId.IsNone())
	{
		ChooseCorrectTalkOption();
	}
	else
	{
		ActiveSession.CorrectTalkOptionId = NAME_None;
	}
}

void UProjectIntimacySubsystem::ChooseCorrectTalkOption()
{
	if (!bSessionActive || ActiveSession.HudMode != EProjectIntimacyHudMode::Talk)
	{
		ActiveSession.CorrectTalkOptionId = NAME_None;
		return;
	}

	FGameplayTagContainer RelationshipTags;
	if (const FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId) : nullptr)
	{
		RelationshipTags = Profile->RelationshipTags;
	}

	FGameplayTagContainer PreferredTags;
	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	if (Settings)
	{
		if (UDataTable* AffinityTable = LoadTable(Settings->TalkAffinityTable))
		{
			TArray<FProjectIntimacyTalkAffinityRow*> Rows;
			AffinityTable->GetAllRows(TEXT("ProjectIntimacyTalkAffinity"), Rows);
			for (const FProjectIntimacyTalkAffinityRow* Row : Rows)
			{
				if (!Row)
				{
					continue;
				}
				if (Row->Personality != EProjectIntimacyPersonality::Auto && Row->Personality != ActiveSession.EffectivePersonality)
				{
					continue;
				}
				const FGameplayTag ControlTag = UProjectIntimacySettings::GetControlStateTag(ActiveSession.CurrentControlPoints);
				if (Row->ControlStateTag.IsValid() && Row->ControlStateTag != ControlTag)
				{
					continue;
				}
				if (Row->RelationshipTag.IsValid() && !RelationshipTags.HasTagExact(Row->RelationshipTag))
				{
					continue;
				}
				PreferredTags.AppendTags(Row->PreferredTalkTags);
			}
		}
	}

	if (PreferredTags.Num() <= 0)
	{
		UProjectIntimacyDialogueLibrary::BuildPreferredTalkTags(
			ActiveSession.EffectivePersonality,
			UProjectIntimacySettings::GetControlStateTag(ActiveSession.CurrentControlPoints),
			RelationshipTags,
			PreferredTags);
	}

	int32 BestScore = INDEX_NONE;
	TArray<int32> BestOptionIndexes;
	for (int32 Index = 0; Index < ResolvedOptions.Num(); ++Index)
	{
		const FProjectIntimacyResolvedOption& Option = ResolvedOptions[Index];
		if (!Option.bCanBeFlavorCorrectOption || Option.TalkAction == EProjectIntimacyTalkAction::Back || Option.TalkAction == EProjectIntimacyTalkAction::Recruit)
		{
			continue;
		}

		FProjectIntimacyTalkOptionRow ScoreRow;
		ScoreRow.TalkTags = Option.TalkTags;
		const int32 Score = UProjectIntimacyDialogueLibrary::ScoreTalkOptionForPreferredTags(ScoreRow, PreferredTags);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestOptionIndexes.Reset();
			BestOptionIndexes.Add(Index);
		}
		else if (Score == BestScore)
		{
			BestOptionIndexes.Add(Index);
		}
	}

	if (BestOptionIndexes.Num() <= 0)
	{
		ActiveSession.CorrectTalkOptionId = NAME_None;
		return;
	}

	const int32 PickedIndex = BestOptionIndexes[RandomStream.RandRange(0, BestOptionIndexes.Num() - 1)];
	ActiveSession.CorrectTalkOptionId = ResolvedOptions.IsValidIndex(PickedIndex)
		? ResolvedOptions[PickedIndex].OptionId
		: NAME_None;
}

void UProjectIntimacySubsystem::SetHudMode(const EProjectIntimacyHudMode NewMode)
{
	ActiveSession.HudMode = NewMode;
	if (NewMode != EProjectIntimacyHudMode::Talk)
	{
		ActiveSession.ActiveTalkCategoryId = NAME_None;
	}
	if (NewMode != EProjectIntimacyHudMode::Items)
	{
		ActiveSession.ActiveItemCategoryId = NAME_None;
	}
	ActiveSession.SelectedOptionIndex = 0;
	ActiveSession.bPleaseActive = NewMode == EProjectIntimacyHudMode::Please && ActiveSession.bPleaseActive;
	RefreshResolvedOptions();
	RefreshHudWidget();
}

void UProjectIntimacySubsystem::HandleMainOption(const FName OptionId)
{
	if (OptionId == TEXT("Main.Please"))
	{
		StartPlease();
	}
	else if (OptionId == TEXT("Main.Talk"))
	{
		SetHudMode(EProjectIntimacyHudMode::Talk);
	}
	else if (OptionId == TEXT("Main.Items"))
	{
		SetHudMode(EProjectIntimacyHudMode::Items);
	}
	else if (OptionId == TEXT("Main.Cancel"))
	{
		CancelActiveSession();
	}
}

void UProjectIntimacySubsystem::HandleTalkOption(const FProjectIntimacyResolvedOption& Option)
{
	if (Option.OptionId == TEXT("Talk.Back"))
	{
		SetHudMode(EProjectIntimacyHudMode::Main);
		return;
	}

	if (Option.OptionId == TEXT("Talk.Category.Back"))
	{
		ActiveSession.ActiveTalkCategoryId = NAME_None;
		ActiveSession.SelectedOptionIndex = 0;
		ActiveSession.StatusText = FText::FromString(TEXT("Choose a Talk style."));
		RefreshResolvedOptions();
		RefreshHudWidget();
		return;
	}

	if (ActiveSession.ActiveTalkCategoryId.IsNone() && Option.OptionId.ToString().StartsWith(TEXT("Talk.Category.")))
	{
		ActiveSession.ActiveTalkCategoryId = Option.OptionId;
		ActiveSession.SelectedOptionIndex = 0;
		ActiveSession.StatusText = FText::FromString(FString::Printf(
			TEXT("%s selected."),
			*Option.Label.ToString()));
		RefreshResolvedOptions();
		RefreshHudWidget();
		return;
	}

	ExecuteTalkOption(Option);
}

void UProjectIntimacySubsystem::HandleItemsOption(const FProjectIntimacyResolvedOption& Option)
{
	if (Option.OptionId == TEXT("Items.Back"))
	{
		SetHudMode(EProjectIntimacyHudMode::Main);
		return;
	}

	if (Option.OptionId == TEXT("Items.Category.Back"))
	{
		ActiveSession.ActiveItemCategoryId = NAME_None;
		ActiveSession.SelectedOptionIndex = 0;
		ActiveSession.StatusText = FText::FromString(TEXT("Choose an item category."));
		RefreshResolvedOptions();
		RefreshHudWidget();
		return;
	}

	if (ActiveSession.ActiveItemCategoryId.IsNone()
		&& (Option.OptionId == TEXT("Items.Category.Drugs") || Option.OptionId == TEXT("Items.Category.Toys")))
	{
		ActiveSession.ActiveItemCategoryId = Option.OptionId;
		ActiveSession.SelectedOptionIndex = 0;
		ActiveSession.StatusText = Option.OptionId == TEXT("Items.Category.Toys")
			? FText::FromString(TEXT("No toys configured yet."))
			: FText::FromString(TEXT("No drugs configured yet."));
		RefreshResolvedOptions();
		RefreshHudWidget();
		return;
	}

	ActiveSession.StatusText = FText::FromString(TEXT("No intimate items configured yet."));
	RefreshHudWidget();
}

void UProjectIntimacySubsystem::ExecuteTalkOption(const FProjectIntimacyResolvedOption& Option)
{
	if (Option.TalkAction == EProjectIntimacyTalkAction::Back)
	{
		SetHudMode(EProjectIntimacyHudMode::Main);
		return;
	}

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	if (Option.bUsesTalkCooldown && ActiveSession.TalkCooldownRemaining > 0.0f)
	{
		ActiveSession.StatusText = FText::FromString(FString::Printf(
			TEXT("Talk cooldown %.1fs."),
			ActiveSession.TalkCooldownRemaining));
		RefreshHudWidget();
		return;
	}

	if (Option.bUsesTalkCooldown)
	{
		ActiveSession.TalkCooldownRemaining = Settings ? FMath::Max(0.0f, Settings->TalkCooldownSeconds) : 2.0f;
	}
	GrantTalkSxp(Option.SxpReward);

	const float RefusalChance = Settings ? FMath::Clamp(Settings->TalkRefusalChance, 0.0f, 1.0f) : 0.10f;
	const bool bSpeedRequest = Option.TalkAction == EProjectIntimacyTalkAction::SpeedSlow
		|| Option.TalkAction == EProjectIntimacyTalkAction::SpeedNormal
		|| Option.TalkAction == EProjectIntimacyTalkAction::SpeedIntense;
	const bool bAccepted = !bSpeedRequest || RandomStream.FRand() >= RefusalChance;

	UDataTable* ResponseTable = Settings ? LoadTable(Settings->PartnerResponsesTable) : nullptr;
	const EProjectIntimacyPersonality Personality = ActiveSession.EffectivePersonality;
	FText Response = UProjectIntimacyDialogueLibrary::ResolvePartnerResponse(ResponseTable, Option.OptionId, Personality, bAccepted);
	AddChronicleDialogue(Response);
	ActiveSession.StatusText = Response;

	if (!bAccepted)
	{
		RefreshHudWidget();
		return;
	}

	switch (Option.TalkAction)
	{
	case EProjectIntimacyTalkAction::SpeedSlow:
		ApplyAnimationRate(Settings ? Settings->ChillAnimationRate : 0.75f);
		break;
	case EProjectIntimacyTalkAction::SpeedNormal:
		ApplyAnimationRate(Settings ? Settings->NiceAnimationRate : 1.0f);
		break;
	case EProjectIntimacyTalkAction::SpeedIntense:
		ApplyAnimationRate(Settings ? Settings->StallionAnimationRate : 1.35f);
		break;
	case EProjectIntimacyTalkAction::Compliment:
		break;
	case EProjectIntimacyTalkAction::Pathetic:
		break;
	case EProjectIntimacyTalkAction::More:
		break;
	case EProjectIntimacyTalkAction::Recruit:
		TryRecruitPartner();
		break;
	default:
		break;
	}

	if (Option.ControlDelta != 0)
	{
		ApplyControlDelta(Option.ControlDelta, FText::FromString(TEXT("Talk")));
	}

	if (Option.LustDrain > 0.0f && Option.TalkAction != EProjectIntimacyTalkAction::Recruit)
	{
		ApplyLustDrain(Option.LustDrain, FText::FromString(TEXT("Talk")), false);
	}

	if (Option.SxpReward > 0 && FMath::IsNearlyZero(Option.LustDrain) && Option.ControlDelta == 0 && Option.TalkAction != EProjectIntimacyTalkAction::Recruit)
	{
		ActiveSession.StatusText = FText::FromString(FString::Printf(TEXT("Talk granted %d SXP."), Option.SxpReward));
	}

	if (Option.AffectDelta != 0)
	{
		if (FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId) : nullptr)
		{
			Profile->Affect = FMath::Clamp(Profile->Affect + Option.AffectDelta, 0, Settings ? Settings->AffectMax : 100);
			SavePersistentState();
		}
	}

	TriggerMediaCueForTalkOption(Option);
	RefreshResolvedOptions();
	RefreshHudWidget();
}

void UProjectIntimacySubsystem::TriggerMediaCueForTalkOption(const FProjectIntimacyResolvedOption& Option)
{
	if (!HudWidget)
	{
		EnsureHudWidget();
	}

	if (!HudWidget)
	{
		return;
	}

	FProjectIntimacyMediaCueRow Cue;
	if (TryResolveMediaCueForTalkOption(Option, Cue))
	{
		HudWidget->PlayMediaCue(Cue);
	}
}

bool UProjectIntimacySubsystem::TryResolveMediaCueForTalkOption(
	const FProjectIntimacyResolvedOption& Option,
	FProjectIntimacyMediaCueRow& OutCue) const
{
	auto MatchesOption = [&Option](const FProjectIntimacyMediaCueRow& Cue)
	{
		if (!Cue.bEnabled)
		{
			return false;
		}
		if (!Cue.TriggerOptionId.IsNone() && Cue.TriggerOptionId == Option.OptionId)
		{
			return true;
		}
		return Cue.TriggerTalkAction != EProjectIntimacyTalkAction::None
			&& Cue.TriggerTalkAction == Option.TalkAction;
	};

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	if (Settings)
	{
		if (UDataTable* MediaCuesTable = LoadTable(Settings->MediaCuesTable))
		{
			if (MediaCuesTable->GetRowStruct() == FProjectIntimacyMediaCueRow::StaticStruct())
			{
				TArray<FProjectIntimacyMediaCueRow*> Rows;
				MediaCuesTable->GetAllRows(TEXT("ProjectIntimacyMediaCues"), Rows);
				for (const FProjectIntimacyMediaCueRow* Row : Rows)
				{
					if (Row && MatchesOption(*Row))
					{
						OutCue = *Row;
						return true;
					}
				}
			}
		}
	}

	TArray<FProjectIntimacyMediaCueRow> FallbackCues;
	UProjectIntimacyDialogueLibrary::BuildFallbackMediaCues(FallbackCues);
	for (const FProjectIntimacyMediaCueRow& Cue : FallbackCues)
	{
		if (MatchesOption(Cue))
		{
			OutCue = Cue;
			return true;
		}
	}

	return false;
}

void UProjectIntimacySubsystem::TriggerMediaCueForEvent(const FName EventId)
{
	if (EventId.IsNone())
	{
		return;
	}

	if (!HudWidget)
	{
		EnsureHudWidget();
	}

	if (!HudWidget)
	{
		return;
	}

	FProjectIntimacyMediaCueRow Cue;
	if (TryResolveMediaCueForEvent(EventId, Cue))
	{
		HudWidget->PlayMediaCue(Cue);
	}
}

bool UProjectIntimacySubsystem::TryResolveMediaCueForEvent(const FName EventId, FProjectIntimacyMediaCueRow& OutCue) const
{
	if (EventId.IsNone())
	{
		return false;
	}

	auto MatchesEvent = [EventId](const FProjectIntimacyMediaCueRow& Cue)
	{
		return Cue.bEnabled && !Cue.TriggerEventId.IsNone() && Cue.TriggerEventId == EventId;
	};

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	if (Settings)
	{
		if (UDataTable* MediaCuesTable = LoadTable(Settings->MediaCuesTable))
		{
			if (MediaCuesTable->GetRowStruct() == FProjectIntimacyMediaCueRow::StaticStruct())
			{
				TArray<FProjectIntimacyMediaCueRow*> Rows;
				MediaCuesTable->GetAllRows(TEXT("ProjectIntimacyMediaCues"), Rows);
				for (const FProjectIntimacyMediaCueRow* Row : Rows)
				{
					if (Row && MatchesEvent(*Row))
					{
						OutCue = *Row;
						return true;
					}
				}
			}
		}
	}

	TArray<FProjectIntimacyMediaCueRow> FallbackCues;
	UProjectIntimacyDialogueLibrary::BuildFallbackMediaCues(FallbackCues);
	for (const FProjectIntimacyMediaCueRow& Cue : FallbackCues)
	{
		if (MatchesEvent(Cue))
		{
			OutCue = Cue;
			return true;
		}
	}

	return false;
}

void UProjectIntimacySubsystem::StartPlease()
{
	ActiveSession.HudMode = EProjectIntimacyHudMode::Please;
	ActiveSession.bPleaseActive = true;
	ActiveSession.PleaseAttemptIndex = 0;
	ActiveSession.PleaseSuccessCount = 0;
	ActiveSession.StatusText = FText::FromString(TEXT("Press Space on the sweet spot."));
	StartNextPleaseAttempt();
	RefreshResolvedOptions();
	RefreshHudWidget();
}

void UProjectIntimacySubsystem::StartNextPleaseAttempt()
{
	ActiveSession.PleaseElapsedSeconds = 0.0f;
	ActiveSession.PleaseCursorValue = 0.0f;
	ActiveSession.PleaseTargetCenter = RandomStream.FRandRange(0.25f, 0.75f);
	ActiveSession.PleasePulsePeriod = UProjectIntimacySettings::ComputePleasePulsePeriod(ResolvePartnerLevel(), ResolveAllureLevel());
	ActiveSession.PleaseTargetHalfRange = UProjectIntimacySettings::ComputePleaseTargetHalfRange(ResolvePartnerLevel(), ResolveAllureLevel());
}

void UProjectIntimacySubsystem::ResolvePleasePress()
{
	if (!ActiveSession.bPleaseActive)
	{
		return;
	}

	if (FMath::Abs(ActiveSession.PleaseCursorValue - ActiveSession.PleaseTargetCenter) <= ActiveSession.PleaseTargetHalfRange)
	{
		ActiveSession.PleaseSuccessCount += 1;
		ActiveSession.StatusText = FText::FromString(TEXT("Good."));
		const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
		ApplyControlDelta(Settings ? Settings->PleaseHitControlDelta : -10, FText::FromString(TEXT("Please hit")));
	}
	else
	{
		ActiveSession.StatusText = FText::FromString(TEXT("Miss."));
		const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
		ApplyControlDelta(Settings ? Settings->PleaseMissControlDelta : 10, FText::FromString(TEXT("Please miss")));
	}

	ActiveSession.PleaseAttemptIndex += 1;
	const int32 AttemptCount = UProjectIntimacySettings::Get() ? UProjectIntimacySettings::Get()->PleaseAttemptCount : 5;
	if (ActiveSession.PleaseAttemptIndex >= FMath::Max(1, AttemptCount))
	{
		const float InstantDrain = UProjectIntimacySettings::ComputePleaseInstantDrain(ResolveAllureLevel(), ActiveSession.PleaseSuccessCount);
		ApplyLustDrain(InstantDrain, FText::FromString(TEXT("Please")), false);
		AddChronicleSystem(FText::FromString(FString::Printf(
			TEXT("Please landed %d/%d and drained %.0f Lust."),
			ActiveSession.PleaseSuccessCount,
			AttemptCount,
			InstantDrain)));
		ActiveSession.bPleaseActive = false;
		SetHudMode(EProjectIntimacyHudMode::Main);
		return;
	}

	StartNextPleaseAttempt();
	RefreshHudWidget();
}

void UProjectIntimacySubsystem::ApplyControlDelta(const int32 Delta, const FText& ReasonText)
{
	if (!bSessionActive || Delta == 0)
	{
		return;
	}

	ActiveSession.CurrentControlPoints = UProjectIntimacySettings::ApplyControlDelta(ActiveSession.CurrentControlPoints, Delta);
	if (FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId) : nullptr)
	{
		if (RelationshipForcesChill(Profile->RelationshipTags))
		{
			const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
			ActiveSession.CurrentControlPoints = FMath::Min(
				ActiveSession.CurrentControlPoints,
				Settings ? Settings->RelationshipChillMaxControl : 199);
		}
		Profile->ControlPoints = ActiveSession.CurrentControlPoints;
	}

	if (!ReasonText.IsEmpty())
	{
		ActiveSession.StatusText = FText::FromString(FString::Printf(
			TEXT("%s changed Control to %d."),
			*ReasonText.ToString(),
			ActiveSession.CurrentControlPoints));
	}
}

void UProjectIntimacySubsystem::ApplyLustDrain(const float Amount, const FText& ReasonText, const bool bApplyControlResistance)
{
	if (!bSessionActive || Amount <= 0.0f)
	{
		return;
	}

	const float EffectiveAmount = bApplyControlResistance
		? Amount * UProjectIntimacySettings::ComputeControlDrainMultiplier(ActiveSession.CurrentControlPoints)
		: Amount;
	const float PreviousLust = ActiveSession.CurrentLust;
	ActiveSession.CurrentLust = FMath::Max(0.0f, ActiveSession.CurrentLust - EffectiveAmount);
	const float ActualDrain = FMath::Max(0.0f, PreviousLust - ActiveSession.CurrentLust);
	if (ActualDrain > 0.0f)
	{
		if (ActiveSession.ClimaxThreshold <= 0.0f)
		{
			ActiveSession.ClimaxThreshold = UProjectIntimacySettings::ComputeClimaxThreshold(ResolvePartnerLevel());
		}

		float RemainingProgress = ActiveSession.ClimaxProgress;
		const int32 ClimaxCount = UProjectIntimacySettings::ConsumeClimaxProgress(
			ActiveSession.ClimaxProgress,
			ActualDrain,
			ActiveSession.ClimaxThreshold,
			RemainingProgress);
		ActiveSession.ClimaxProgress = RemainingProgress;
		if (ClimaxCount > 0)
		{
			TriggerPartnerClimax(ClimaxCount);
		}
	}
	if (!ReasonText.IsEmpty())
	{
		ActiveSession.StatusText = FText::FromString(FString::Printf(
			TEXT("%s drained %.0f Lust."),
			*ReasonText.ToString(),
			ActualDrain));
	}
}

void UProjectIntimacySubsystem::UpdateClimaxIntensity(const float DeltaTime)
{
	if (!bSessionActive)
	{
		return;
	}

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	const float TargetMultiplier = Settings ? FMath::Max(1.0f, Settings->ClimaxIntensityMultiplier) : 1.25f;
	const float RecoverySeconds = Settings ? FMath::Max(0.0f, Settings->ClimaxRecoverySeconds) : 2.0f;

	float RecoveryMultiplier = 1.0f;
	if (ActiveSession.ClimaxRecoveryRemaining > 0.0f && RecoverySeconds > 0.0f)
	{
		ActiveSession.ClimaxRecoveryRemaining = FMath::Max(0.0f, ActiveSession.ClimaxRecoveryRemaining - FMath::Max(0.0f, DeltaTime));
		const float Alpha = FMath::Clamp(ActiveSession.ClimaxRecoveryRemaining / RecoverySeconds, 0.0f, 1.0f);
		RecoveryMultiplier = FMath::Lerp(1.0f, TargetMultiplier, Alpha);
	}
	else
	{
		ActiveSession.ClimaxRecoveryRemaining = 0.0f;
	}

	const float AnticipationMultiplier = UProjectIntimacySettings::ComputeClimaxAnticipationMultiplier(
		ActiveSession.ClimaxProgress,
		ActiveSession.ClimaxThreshold,
		Settings);
	ActiveSession.ClimaxMultiplier = FMath::Max(1.0f, FMath::Max(RecoveryMultiplier, AnticipationMultiplier));
}

void UProjectIntimacySubsystem::TriggerPartnerClimax(const int32 ClimaxCount)
{
	if (!bSessionActive || ClimaxCount <= 0)
	{
		return;
	}

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	ActiveSession.ClimaxRecoveryRemaining = Settings ? FMath::Max(0.0f, Settings->ClimaxRecoverySeconds) : 2.0f;
	ActiveSession.ClimaxMultiplier = Settings ? FMath::Max(1.0f, Settings->ClimaxIntensityMultiplier) : 1.25f;
	if (FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId) : nullptr)
	{
		Profile->ClimaxCount += ClimaxCount;
		SavePersistentState();
	}
	GrantPlayerLustFromClimax(ClimaxCount);
	AddChronicleSystem(FText::FromString(TEXT("Climax")));
	TriggerMediaCueForEvent(Settings ? Settings->ClimaxMediaEventId : FName(TEXT("Climax")));
	ActiveSession.StatusText = FText::FromString(TEXT("Climax"));
	if (TrackedEmoteComponent
		&& TrackedEmoteComponent->GetActiveInteractionId() == ProjectIntimacySubsystemPrivate::IntimacySceneId)
	{
		TrackedEmoteComponent->TriggerBlueprintSceneVisualClimaxCue();
	}
	ApplyAnimationRate(ActiveSession.AnimationRate);
}

void UProjectIntimacySubsystem::GrantPlayerLustFromClimax(const int32 ClimaxCount)
{
	if (ClimaxCount <= 0)
	{
		return;
	}

	UProjectSurvivalNeedsComponent* NeedsComponent = EnsureSurvivalNeedsComponent();
	if (!NeedsComponent)
	{
		return;
	}

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	const float GainPercent = Settings ? FMath::Max(0.0f, Settings->PlayerLustGainPercentOnClimax) : 0.10f;
	const float MaxLust = NeedsComponent->GetSensationMaxValue(ProjectIntimacySubsystemPrivate::PlayerLustSensationName);
	const float LustGain = FMath::Max(0.0f, MaxLust * GainPercent * static_cast<float>(ClimaxCount));
	if (LustGain > 0.0f)
	{
		NeedsComponent->ModifySensationValue(ProjectIntimacySubsystemPrivate::PlayerLustSensationName, LustGain, true);
	}
}

void UProjectIntimacySubsystem::UpdateOverpoweringPulse(const float DeltaTime)
{
	if (UProjectIntimacySettings::GetControlState(ActiveSession.CurrentControlPoints) != EProjectIntimacyControlState::Overpowering)
	{
		ActiveSession.OverpoweringPulseElapsedSeconds = 0.0f;
		return;
	}

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	const float PulseInterval = Settings ? FMath::Max(1.0f, Settings->OverpoweringPulseIntervalSeconds) : 60.0f;
	ActiveSession.OverpoweringPulseElapsedSeconds += DeltaTime;
	if (ActiveSession.OverpoweringPulseElapsedSeconds < PulseInterval)
	{
		return;
	}

	ActiveSession.OverpoweringPulseElapsedSeconds = 0.0f;
	const float HealAmount = Settings ? FMath::Max(0.0f, Settings->OverpoweringLustHealPerPulse) : 100.0f;
	if (HealAmount > 0.0f)
	{
		ActiveSession.CurrentLust = FMath::Min(ActiveSession.MaxLust, ActiveSession.CurrentLust + HealAmount);
	}

	if (RandomStream.FRand() < (Settings ? FMath::Clamp(Settings->OverpoweringIntensityChance, 0.0f, 1.0f) : 0.20f))
	{
		ApplyAnimationRate(Settings ? Settings->OverpoweringSelfIntensityRate : 1.50f);
		AddChronicleSystem(FText::FromString(TEXT("The partner takes control and increases the intensity.")));
	}

	if (RandomStream.FRand() < (Settings ? FMath::Clamp(Settings->OverpoweringPoseChangeChance, 0.0f, 1.0f) : 0.20f))
	{
		AddChronicleSystem(FText::FromString(TEXT("The partner tries to change pose, but no alternate intimate pose is configured yet.")));
	}

	if (HealAmount > 0.0f)
	{
		ActiveSession.StatusText = FText::FromString(FString::Printf(
			TEXT("Overpowering restored %.0f Lust."),
			HealAmount));
	}
}

void UProjectIntimacySubsystem::ApplyAnimationRate(const float NewRate)
{
	ActiveSession.AnimationRate = FMath::Max(0.05f, NewRate);
	const float EffectiveRate = ComputeEffectiveAnimationRate();
	TArray<USkeletalMeshComponent*> Meshes;
	CollectSessionMeshes(Meshes);
	for (USkeletalMeshComponent* Mesh : Meshes)
	{
		CacheAndSetMeshRate(Mesh, EffectiveRate);
	}
}

float UProjectIntimacySubsystem::ComputeEffectiveAnimationRate() const
{
	return FMath::Max(0.05f, ActiveSession.AnimationRate * FMath::Max(1.0f, ActiveSession.ClimaxMultiplier));
}

void UProjectIntimacySubsystem::RestoreAnimationRates()
{
	for (const TPair<TWeakObjectPtr<USkeletalMeshComponent>, float>& Pair : CachedMeshRates)
	{
		if (USkeletalMeshComponent* Mesh = Pair.Key.Get())
		{
			Mesh->GlobalAnimRateScale = Pair.Value;
		}
	}
	CachedMeshRates.Reset();
}

void UProjectIntimacySubsystem::CacheAndSetMeshRate(USkeletalMeshComponent* MeshComponent, const float NewRate)
{
	if (!MeshComponent)
	{
		return;
	}

	if (!CachedMeshRates.Contains(MeshComponent))
	{
		CachedMeshRates.Add(MeshComponent, MeshComponent->GlobalAnimRateScale);
	}
	MeshComponent->GlobalAnimRateScale = FMath::Max(0.05f, NewRate);
}

void UProjectIntimacySubsystem::CollectSessionMeshes(TArray<USkeletalMeshComponent*>& OutMeshes) const
{
	OutMeshes.Reset();

	auto AddActorMeshes = [&OutMeshes](AActor* Actor)
	{
		if (!Actor)
		{
			return;
		}
		TInlineComponentArray<USkeletalMeshComponent*> Meshes(Actor);
		for (USkeletalMeshComponent* Mesh : Meshes)
		{
			if (Mesh)
			{
				OutMeshes.AddUnique(Mesh);
			}
		}
	};

	AddActorMeshes(TrackedPlayerPawn);
	AddActorMeshes(ActiveSession.PartnerActor.Get());
	if (TrackedEmoteComponent)
	{
		AddActorMeshes(TrackedEmoteComponent->GetActiveBlueprintSceneVisualActor());
	}
}

bool UProjectIntimacySubsystem::TryRecruitPartner()
{
	if (!bSessionActive)
	{
		return false;
	}

	FProjectIntimacyPartnerProfile* Profile = IntimacySaveGame ? IntimacySaveGame->PartnerProfiles.Find(ActiveSession.PartnerId) : nullptr;
	if (!Profile || !IsAllyEligible(*Profile))
	{
		ActiveSession.StatusText = FText::FromString(TEXT("Not close enough yet."));
		return false;
	}

	AActor* PartnerActor = ActiveSession.PartnerActor.Get();
	if (!PartnerActor)
	{
		return false;
	}

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	UACFTeamComponent* PartnerTeam = PartnerActor->FindComponentByClass<UACFTeamComponent>();
	UACFTeamComponent* PlayerTeam = TrackedPlayerPawn ? TrackedPlayerPawn->FindComponentByClass<UACFTeamComponent>() : nullptr;
	const FGameplayTag NewTeam = Settings && Settings->OverrideAllyTeamTag.IsValid()
		? Settings->OverrideAllyTeamTag
		: (PlayerTeam ? PlayerTeam->GetTeam() : FGameplayTag());
	if (PartnerTeam && NewTeam.IsValid())
	{
		if (PartnerActor->HasAuthority())
		{
			PartnerTeam->SetTeam(NewTeam);
		}
		else
		{
			PartnerTeam->ServerRequestTeamChange(NewTeam);
		}
	}

	if (AACFCompanionsPlayerController* CompanionController = Cast<AACFCompanionsPlayerController>(TrackedPlayerController))
	{
		if (UACFCompanionGroupAIComponent* Companions = CompanionController->GetCompanionsComponent())
		{
			if (AACFCharacter* CompanionCharacter = Cast<AACFCharacter>(PartnerActor))
			{
				if (!Companions->IsAlreadyInGroup(CompanionCharacter))
				{
					Companions->AddExistingCharacterToGroup(CompanionCharacter);
				}
				Companions->SetInBattle(false, nullptr);
			}
		}
	}

	Profile->bConvertedToAlly = true;
	SavePersistentState();
	ActiveSession.StatusText = FText::FromString(TEXT("They are now your ally."));
	AddChronicleSystem(FText::FromString(TEXT("Converted to ally through intimate Talk.")));
	return true;
}

void UProjectIntimacySubsystem::GrantSatisfiedSxp()
{
	const UProjectIntimacySettings* IntimacySettings = UProjectIntimacySettings::Get();
	const UProjectSinfulAscensionSettings* SinSettings = UProjectSinfulAscensionSettings::Get();
	const int32 Amount = IntimacySettings
		? IntimacySettings->SatisfiedWinSxp
		: (SinSettings ? SinSettings->SinfulInteractionSxp : 150);

	if (UProjectSinfulAscensionComponent* SinComponent = EnsureSinfulAscensionComponent())
	{
		SinComponent->GrantSxpWithAttributeAffinity(
			TEXT("Intimacy.SatisfiedWin"),
			Amount,
			true,
			{ EProjectSinAttribute::Allure });
	}
}

void UProjectIntimacySubsystem::GrantTalkSxp(const int32 Amount)
{
	const int32 ClampedAmount = FMath::Max(0, Amount);
	if (ClampedAmount <= 0)
	{
		return;
	}

	if (UProjectSinfulAscensionComponent* SinComponent = EnsureSinfulAscensionComponent())
	{
		SinComponent->GrantSxpWithAttributeAffinity(
			TEXT("Intimacy.Talk"),
			ClampedAmount,
			true,
			{ EProjectSinAttribute::Allure });
	}
}

void UProjectIntimacySubsystem::AddChronicleDialogue(const FText& Message)
{
	if (UWorld* World = GetWorld())
	{
		if (UProjectActivityFeedSubsystem* ActivityFeed = World->GetSubsystem<UProjectActivityFeedSubsystem>())
		{
			ActivityFeed->AddPartnerDialogueEntry(ActiveSession.PartnerActor.Get(), Message);
		}
	}
}

void UProjectIntimacySubsystem::AddChronicleSystem(const FText& Message)
{
	if (UWorld* World = GetWorld())
	{
		if (UProjectActivityFeedSubsystem* ActivityFeed = World->GetSubsystem<UProjectActivityFeedSubsystem>())
		{
			ActivityFeed->AddSystemEntry(Message);
		}
	}
}

int32 UProjectIntimacySubsystem::ResolveAllureLevel() const
{
	if (const UProjectSinfulAscensionComponent* SinComponent = TrackedPlayerPawn
		? TrackedPlayerPawn->FindComponentByClass<UProjectSinfulAscensionComponent>()
		: nullptr)
	{
		return SinComponent->GetAttributeLevel(EProjectSinAttribute::Allure);
	}

	return 0;
}

int32 UProjectIntimacySubsystem::ResolvePartnerLevel() const
{
	if (const UProjectIntimacyPartnerComponent* PartnerComponent = ActiveSession.PartnerComponent.Get())
	{
		return PartnerComponent->GetPartnerLevel();
	}

	return 1;
}

EProjectIntimacyPersonality UProjectIntimacySubsystem::ResolveEffectivePersonality(
	const FProjectIntimacyPartnerProfile& Profile,
	const UProjectIntimacyPartnerComponent* PartnerComponent) const
{
	if (RelationshipForcesChill(Profile.RelationshipTags))
	{
		return EProjectIntimacyPersonality::Chill;
	}

	return PartnerComponent ? PartnerComponent->GetResolvedPersonality() : Profile.Personality;
}

void UProjectIntimacySubsystem::NormalizeProfile(
	UProjectIntimacyPartnerComponent* PartnerComponent,
	FProjectIntimacyPartnerProfile& Profile) const
{
	if (Profile.PartnerId.IsEmpty() && PartnerComponent)
	{
		Profile.PartnerId = PartnerComponent->GetResolvedPartnerId();
	}

	if (!Profile.GenderTag.IsValid())
	{
		Profile.GenderTag = PartnerComponent
			? PartnerComponent->GetResolvedGenderTag()
			: ProjectIntimacySubsystemPrivate::Tag(TEXT("Project.Gender.Male"));
	}

	if (Profile.RelationshipTags.Num() <= 0)
	{
		RefreshRelationshipTags(Profile);
	}

	if (!Profile.bControlInitialized)
	{
		const EProjectIntimacyPersonality InitialPersonality = PartnerComponent
			? PartnerComponent->GetResolvedPersonality()
			: Profile.Personality;
		Profile.ControlPoints = UProjectIntimacySettings::ComputeInitialControl(InitialPersonality);
		Profile.bControlInitialized = true;
	}

	Profile.ControlPoints = UProjectIntimacySettings::ClampControlPoints(Profile.ControlPoints);
}

FGameplayTag UProjectIntimacySubsystem::GetBaseRelationshipTag(const int32 Encounters) const
{
	if (Encounters >= 50)
	{
		return ProjectIntimacySubsystemPrivate::Tag(TEXT("Project.Intimacy.Relationship.Partner"));
	}
	if (Encounters >= 30)
	{
		return ProjectIntimacySubsystemPrivate::Tag(TEXT("Project.Intimacy.Relationship.Devoted"));
	}
	if (Encounters >= 20)
	{
		return ProjectIntimacySubsystemPrivate::Tag(TEXT("Project.Intimacy.Relationship.Attached"));
	}
	if (Encounters >= 10)
	{
		return ProjectIntimacySubsystemPrivate::Tag(TEXT("Project.Intimacy.Relationship.Interested"));
	}
	return ProjectIntimacySubsystemPrivate::Tag(TEXT("Project.Intimacy.Relationship.Unknown"));
}

void UProjectIntimacySubsystem::RefreshRelationshipTags(FProjectIntimacyPartnerProfile& Profile) const
{
	ProjectIntimacySubsystemPrivate::RemoveTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Unknown"));
	ProjectIntimacySubsystemPrivate::RemoveTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Interested"));
	ProjectIntimacySubsystemPrivate::RemoveTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Attached"));
	ProjectIntimacySubsystemPrivate::RemoveTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Devoted"));
	ProjectIntimacySubsystemPrivate::RemoveTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Partner"));

	const FGameplayTag BaseRelationshipTag = GetBaseRelationshipTag(Profile.Encounters);
	if (BaseRelationshipTag.IsValid())
	{
		Profile.RelationshipTags.AddTag(BaseRelationshipTag);
	}

	if (Profile.Children >= 5)
	{
		ProjectIntimacySubsystemPrivate::AddTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Bull"));
	}
	else
	{
		ProjectIntimacySubsystemPrivate::RemoveTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Bull"));
	}

	if (Profile.FailedEncounters >= 10)
	{
		ProjectIntimacySubsystemPrivate::AddTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Master"));
	}
	else
	{
		ProjectIntimacySubsystemPrivate::RemoveTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Master"));
	}

	const FGameplayTag MaleTag = ProjectIntimacySubsystemPrivate::Tag(TEXT("Project.Gender.Male"));
	if (Profile.bHasHusbandRing && MaleTag.IsValid() && Profile.GenderTag == MaleTag)
	{
		ProjectIntimacySubsystemPrivate::AddTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Husband"));
	}
	else
	{
		ProjectIntimacySubsystemPrivate::RemoveTag(Profile.RelationshipTags, TEXT("Project.Intimacy.Relationship.Husband"));
	}
}

bool UProjectIntimacySubsystem::RelationshipForcesChill(const FGameplayTagContainer& RelationshipTags) const
{
	return UProjectIntimacyDialogueLibrary::RelationshipTagsForceChill(RelationshipTags);
}

bool UProjectIntimacySubsystem::HasRelationshipTag(
	const FProjectIntimacyPartnerProfile& Profile,
	const TCHAR* TagName) const
{
	return ProjectIntimacySubsystemPrivate::HasTag(Profile.RelationshipTags, TagName);
}

FProjectIntimacyPartnerProfile& UProjectIntimacySubsystem::GetMutableProfile(UProjectIntimacyPartnerComponent* PartnerComponent)
{
	LoadPersistentState();

	const FString PartnerId = PartnerComponent ? PartnerComponent->GetResolvedPartnerId() : FString(TEXT("UnknownPartner"));
	if (!IntimacySaveGame->PartnerProfiles.Contains(PartnerId))
	{
		FProjectIntimacyPartnerProfile Profile;
		Profile.PartnerId = PartnerId;
		Profile.Personality = PartnerComponent ? PartnerComponent->GetResolvedPersonality() : EProjectIntimacyPersonality::Nice;
		Profile.Relationship = PartnerComponent ? PartnerComponent->InitialRelationship : EProjectIntimacyRelationship::Unknown;
		Profile.Children = PartnerComponent ? PartnerComponent->InitialChildren : 0;
		Profile.Affect = PartnerComponent ? PartnerComponent->InitialAffect : 0;
		Profile.GenderTag = PartnerComponent
			? PartnerComponent->GetResolvedGenderTag()
			: ProjectIntimacySubsystemPrivate::Tag(TEXT("Project.Gender.Male"));
		Profile.ControlPoints = UProjectIntimacySettings::ComputeInitialControl(
			PartnerComponent ? PartnerComponent->GetResolvedPersonality() : Profile.Personality);
		Profile.bControlInitialized = true;
		RefreshRelationshipTags(Profile);
		IntimacySaveGame->PartnerProfiles.Add(PartnerId, Profile);
	}

	FProjectIntimacyPartnerProfile& Profile = IntimacySaveGame->PartnerProfiles.FindChecked(PartnerId);
	NormalizeProfile(PartnerComponent, Profile);
	RefreshRelationshipTags(Profile);
	return Profile;
}

void UProjectIntimacySubsystem::LoadPersistentState()
{
	if (IntimacySaveGame)
	{
		return;
	}

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	const FString SlotName = Settings ? Settings->SaveSlotName : FString(TEXT("ProjectIntimacy"));
	const int32 UserIndex = Settings ? Settings->SaveUserIndex : 0;
	if (USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex))
	{
		IntimacySaveGame = Cast<UProjectIntimacySaveGame>(LoadedSave);
	}

	if (!IntimacySaveGame)
	{
		IntimacySaveGame = Cast<UProjectIntimacySaveGame>(UGameplayStatics::CreateSaveGameObject(UProjectIntimacySaveGame::StaticClass()));
	}
}

void UProjectIntimacySubsystem::SavePersistentState() const
{
	if (!IntimacySaveGame)
	{
		return;
	}

	const UProjectIntimacySettings* Settings = UProjectIntimacySettings::Get();
	const FString SlotName = Settings ? Settings->SaveSlotName : FString(TEXT("ProjectIntimacy"));
	const int32 UserIndex = Settings ? Settings->SaveUserIndex : 0;
	UGameplayStatics::SaveGameToSlot(IntimacySaveGame, SlotName, UserIndex);
}

UProjectSinfulAscensionComponent* UProjectIntimacySubsystem::EnsureSinfulAscensionComponent() const
{
	if (!TrackedPlayerPawn)
	{
		return nullptr;
	}

	if (UProjectSinfulAscensionComponent* ExistingComponent = TrackedPlayerPawn->FindComponentByClass<UProjectSinfulAscensionComponent>())
	{
		return ExistingComponent;
	}

	UProjectSinfulAscensionComponent* Component = NewObject<UProjectSinfulAscensionComponent>(
		TrackedPlayerPawn,
		UProjectSinfulAscensionComponent::StaticClass(),
		TEXT("ProjectSinfulAscensionComponent"));
	if (Component)
	{
		TrackedPlayerPawn->AddInstanceComponent(Component);
		Component->RegisterComponent();
	}
	return Component;
}

UProjectSurvivalNeedsComponent* UProjectIntimacySubsystem::EnsureSurvivalNeedsComponent() const
{
	if (!TrackedPlayerPawn)
	{
		return nullptr;
	}

	if (UProjectSurvivalNeedsComponent* ExistingComponent = TrackedPlayerPawn->FindComponentByClass<UProjectSurvivalNeedsComponent>())
	{
		return ExistingComponent;
	}

	UProjectSurvivalNeedsComponent* Component = NewObject<UProjectSurvivalNeedsComponent>(
		TrackedPlayerPawn,
		UProjectSurvivalNeedsComponent::StaticClass(),
		TEXT("ProjectSurvivalNeedsComponent"));
	if (Component)
	{
		TrackedPlayerPawn->AddInstanceComponent(Component);
		Component->RegisterComponent();
	}
	return Component;
}

UProjectSurvivalStatusComponent* UProjectIntimacySubsystem::EnsureStatusComponent() const
{
	if (!TrackedPlayerPawn)
	{
		return nullptr;
	}

	if (UProjectSurvivalStatusComponent* ExistingComponent = TrackedPlayerPawn->FindComponentByClass<UProjectSurvivalStatusComponent>())
	{
		return ExistingComponent;
	}

	UProjectSurvivalStatusComponent* Component = NewObject<UProjectSurvivalStatusComponent>(
		TrackedPlayerPawn,
		UProjectSurvivalStatusComponent::StaticClass(),
		TEXT("ProjectSurvivalStatusComponent"));
	if (Component)
	{
		TrackedPlayerPawn->AddInstanceComponent(Component);
		Component->RegisterComponent();
	}
	return Component;
}

UDataTable* UProjectIntimacySubsystem::LoadTable(const FSoftObjectPath& TablePath) const
{
	return TablePath.IsValid() ? Cast<UDataTable>(TablePath.TryLoad()) : nullptr;
}

bool UProjectIntimacySubsystem::IsAllyEligible(const FProjectIntimacyPartnerProfile& Profile) const
{
	return !Profile.bConvertedToAlly
		&& UProjectIntimacyDialogueLibrary::RelationshipTagsAllowRecruit(Profile.RelationshipTags);
}

void UProjectIntimacySubsystem::HandleToggleHudPressed()
{
	if (bSessionActive)
	{
		RequestToggleHud();
		return;
	}

	RequestQuickStartIntimacy();
}

void UProjectIntimacySubsystem::HandleNavigateUpPressed()
{
	RequestNavigate(-1);
}

void UProjectIntimacySubsystem::HandleNavigateDownPressed()
{
	RequestNavigate(1);
}

void UProjectIntimacySubsystem::HandleNavigateLeftPressed()
{
	RequestBack();
}

void UProjectIntimacySubsystem::HandleNavigateRightPressed()
{
	RequestConfirm();
}

void UProjectIntimacySubsystem::HandleConfirmPressed()
{
	RequestConfirm();
}
