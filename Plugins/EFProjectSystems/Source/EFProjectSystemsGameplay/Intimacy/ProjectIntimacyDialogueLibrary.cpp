#include "Intimacy/ProjectIntimacyDialogueLibrary.h"

#include "Engine/DataTable.h"

#include <initializer_list>

namespace ProjectIntimacyDialogueLibraryPrivate
{
	void AddTag(FGameplayTagContainer& Container, const TCHAR* TagName)
	{
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
		if (Tag.IsValid())
		{
			Container.AddTag(Tag);
		}
	}

	bool HasRelationshipTag(const FGameplayTagContainer& Container, const TCHAR* TagName)
	{
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
		return Tag.IsValid() && Container.HasTagExact(Tag);
	}
}

FGameplayTag UProjectIntimacyDialogueLibrary::GetPersonalityTag(const EProjectIntimacyPersonality Personality)
{
	switch (Personality)
	{
	case EProjectIntimacyPersonality::Chill:
		return FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Personality.Chill"), false);
	case EProjectIntimacyPersonality::Stallion:
		return FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Personality.Stallion"), false);
	case EProjectIntimacyPersonality::Nice:
	case EProjectIntimacyPersonality::Auto:
	default:
		return FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Personality.Nice"), false);
	}
}

FGameplayTag UProjectIntimacyDialogueLibrary::GetTalkStyleTag(const FName StyleName)
{
	return FGameplayTag::RequestGameplayTag(FName(FString::Printf(TEXT("Project.Intimacy.Talk.Tag.%s"), *StyleName.ToString())), false);
}

void UProjectIntimacyDialogueLibrary::BuildPreferredTalkTags(
	const EProjectIntimacyPersonality Personality,
	const FGameplayTag ControlStateTag,
	const FGameplayTagContainer& RelationshipTags,
	FGameplayTagContainer& OutPreferredTags)
{
	OutPreferredTags.Reset();

	if (ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Attached"))
		|| ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Devoted"))
		|| ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Partner"))
		|| ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Husband")))
	{
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Soft"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Comfort"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Affectionate"));
		return;
	}

	switch (Personality)
	{
	case EProjectIntimacyPersonality::Chill:
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Soft"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Comfort"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Affectionate"));
		break;
	case EProjectIntimacyPersonality::Stallion:
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Dominant"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Risky"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Command"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Submissive"));
		break;
	case EProjectIntimacyPersonality::Nice:
	case EProjectIntimacyPersonality::Auto:
	default:
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Playful"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Affectionate"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Soft"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Command"));
		break;
	}

	if (ControlStateTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Control.Overpowering"), false)))
	{
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Risky"));
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutPreferredTags, TEXT("Project.Intimacy.Talk.Tag.Command"));
	}
}

int32 UProjectIntimacyDialogueLibrary::ScoreTalkOptionForPreferredTags(
	const FProjectIntimacyTalkOptionRow& Option,
	const FGameplayTagContainer& PreferredTags)
{
	int32 Score = 0;
	TArray<FGameplayTag> Tags;
	Option.TalkTags.GetGameplayTagArray(Tags);
	for (const FGameplayTag& Tag : Tags)
	{
		if (PreferredTags.HasTagExact(Tag))
		{
			Score += 1;
		}
	}
	return Score;
}

void UProjectIntimacyDialogueLibrary::BuildRelationshipTagsFromCounters(
	const int32 Encounters,
	const int32 Children,
	const int32 FailedEncounters,
	const FGameplayTag GenderTag,
	const bool bHasHusbandRing,
	FGameplayTagContainer& OutRelationshipTags)
{
	OutRelationshipTags.Reset();
	if (Encounters >= 50)
	{
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutRelationshipTags, TEXT("Project.Intimacy.Relationship.Partner"));
	}
	else if (Encounters >= 30)
	{
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutRelationshipTags, TEXT("Project.Intimacy.Relationship.Devoted"));
	}
	else if (Encounters >= 20)
	{
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutRelationshipTags, TEXT("Project.Intimacy.Relationship.Attached"));
	}
	else if (Encounters >= 10)
	{
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutRelationshipTags, TEXT("Project.Intimacy.Relationship.Interested"));
	}
	else
	{
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutRelationshipTags, TEXT("Project.Intimacy.Relationship.Unknown"));
	}

	if (Children >= 5)
	{
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutRelationshipTags, TEXT("Project.Intimacy.Relationship.Bull"));
	}
	if (FailedEncounters >= 10)
	{
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutRelationshipTags, TEXT("Project.Intimacy.Relationship.Master"));
	}

	const FGameplayTag MaleTag = FGameplayTag::RequestGameplayTag(TEXT("Project.Gender.Male"), false);
	if (bHasHusbandRing && MaleTag.IsValid() && GenderTag == MaleTag)
	{
		ProjectIntimacyDialogueLibraryPrivate::AddTag(OutRelationshipTags, TEXT("Project.Intimacy.Relationship.Husband"));
	}
}

bool UProjectIntimacyDialogueLibrary::RelationshipTagsForceChill(const FGameplayTagContainer& RelationshipTags)
{
	return ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Attached"))
		|| ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Devoted"))
		|| ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Partner"))
		|| ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Husband"));
}

bool UProjectIntimacyDialogueLibrary::RelationshipTagsAllowRecruit(const FGameplayTagContainer& RelationshipTags)
{
	return ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Devoted"))
		|| ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Partner"))
		|| ProjectIntimacyDialogueLibraryPrivate::HasRelationshipTag(RelationshipTags, TEXT("Project.Intimacy.Relationship.Husband"));
}

void UProjectIntimacyDialogueLibrary::BuildFallbackTalkOptions(
	TArray<FProjectIntimacyTalkOptionRow>& OutOptions,
	const bool bAllyEligible)
{
	OutOptions.Reset();

	auto AddOption = [&OutOptions](
		const TCHAR* Id,
		const TCHAR* Label,
		const EProjectIntimacyTalkAction Action,
		const TCHAR* CategoryId,
		std::initializer_list<const TCHAR*> Tags,
		const int32 SxpReward = 0,
		const float LustDrain = 0.0f,
		const int32 ControlDelta = 0,
		const bool bCanBeFlavorCorrectOption = true)
	{
		FProjectIntimacyTalkOptionRow& Option = OutOptions.AddDefaulted_GetRef();
		Option.OptionId = FName(Id);
		Option.Label = FText::FromString(Label);
		Option.Action = Action;
		Option.CategoryId = FName(CategoryId);
		Option.SxpReward = SxpReward;
		Option.LustDrain = LustDrain;
		Option.ControlDelta = ControlDelta;
		Option.bCanBeCorrectTalkOption = bCanBeFlavorCorrectOption;
		Option.bCanBeFlavorCorrectOption = bCanBeFlavorCorrectOption;
		for (const TCHAR* TagName : Tags)
		{
			ProjectIntimacyDialogueLibraryPrivate::AddTag(Option.TalkTags, TagName);
		}
	};

	AddOption(TEXT("Talk.Speed.Slow"), TEXT("Ask: slow"), EProjectIntimacyTalkAction::SpeedSlow,
		TEXT("Talk.Category.Intensity"),
		{ TEXT("Project.Intimacy.Talk.Tag.Soft"), TEXT("Project.Intimacy.Talk.Tag.Comfort") },
		0);
	AddOption(TEXT("Talk.Speed.Normal"), TEXT("Ask: steady"), EProjectIntimacyTalkAction::SpeedNormal,
		TEXT("Talk.Category.Intensity"),
		{ TEXT("Project.Intimacy.Talk.Tag.Affectionate"), TEXT("Project.Intimacy.Talk.Tag.Playful") },
		0);
	AddOption(TEXT("Talk.Speed.Intense"), TEXT("Ask: intense"), EProjectIntimacyTalkAction::SpeedIntense,
		TEXT("Talk.Category.Intensity"),
		{ TEXT("Project.Intimacy.Talk.Tag.Risky"), TEXT("Project.Intimacy.Talk.Tag.Command") },
		0);
	AddOption(TEXT("Talk.Pathetic"), TEXT("Pathetic"), EProjectIntimacyTalkAction::Pathetic,
		TEXT("Talk.Category.Dominant"),
		{ TEXT("Project.Intimacy.Talk.Tag.Dominant"), TEXT("Project.Intimacy.Talk.Tag.Risky") },
		0,
		0.0f,
		-25);
	AddOption(TEXT("Talk.More"), TEXT("More!"), EProjectIntimacyTalkAction::More,
		TEXT("Talk.Category.Submissive"),
		{ TEXT("Project.Intimacy.Talk.Tag.Submissive"), TEXT("Project.Intimacy.Talk.Tag.Playful"), TEXT("Project.Intimacy.Talk.Tag.Risky") },
		0,
		25.0f);
	AddOption(TEXT("Talk.Compliment"), TEXT("Compliment"), EProjectIntimacyTalkAction::Compliment,
		TEXT("Talk.Category.Neutral"),
		{ TEXT("Project.Intimacy.Talk.Tag.Affectionate"), TEXT("Project.Intimacy.Talk.Tag.Soft") },
		10);
	if (bAllyEligible)
	{
		AddOption(TEXT("Talk.Recruit"), TEXT("Become ally"), EProjectIntimacyTalkAction::Recruit,
			TEXT("Talk.Category.Neutral"),
			{ TEXT("Project.Intimacy.Talk.Tag.Affectionate"), TEXT("Project.Intimacy.Talk.Tag.Command") },
			0,
			0.0f,
			0,
			false);
	}
	AddOption(TEXT("Talk.Back"), TEXT("Back"), EProjectIntimacyTalkAction::Back, TEXT(""), {}, 0, 0.0f, 0, false);
}

void UProjectIntimacyDialogueLibrary::BuildFallbackMediaCues(TArray<FProjectIntimacyMediaCueRow>& OutRows)
{
	OutRows.Reset();

	FProjectIntimacyMediaCueRow& MorePreview = OutRows.AddDefaulted_GetRef();
	MorePreview.CueId = TEXT("Talk.More.Preview");
	MorePreview.TriggerOptionId = TEXT("Talk.More");
	MorePreview.TriggerTalkAction = EProjectIntimacyTalkAction::More;
	MorePreview.MediaType = EProjectIntimacyMediaType::Image;
	MorePreview.SourceImagePath = TEXT("_Game/Images/Intimacy/Preview_IntimacyImage.png");
	MorePreview.FadeInSeconds = 1.0f;
	MorePreview.HoldSeconds = 2.0f;
	MorePreview.FadeOutSeconds = 1.0f;
	MorePreview.SourceMediaSize = FVector2D(1080.0f, 720.0f);
	MorePreview.bEnabled = true;

	FProjectIntimacyMediaCueRow& ClimaxPreview = OutRows.AddDefaulted_GetRef();
	ClimaxPreview.CueId = TEXT("Climax.Preview");
	ClimaxPreview.TriggerEventId = TEXT("Climax");
	ClimaxPreview.MediaType = EProjectIntimacyMediaType::Image;
	ClimaxPreview.SourceImagePath = TEXT("_Game/Images/Intimacy/Preview_IntimacyImage.png");
	ClimaxPreview.FadeInSeconds = 1.0f;
	ClimaxPreview.HoldSeconds = 2.0f;
	ClimaxPreview.FadeOutSeconds = 1.0f;
	ClimaxPreview.SourceMediaSize = FVector2D(1080.0f, 720.0f);
	ClimaxPreview.bEnabled = true;
}

void UProjectIntimacyDialogueLibrary::BuildFallbackSocialCardRows(TArray<FProjectSocialCardRow>& OutRows)
{
	OutRows.Reset();

	auto AddRow = [&OutRows](
		const TCHAR* RowId,
		const TCHAR* Label,
		const TCHAR* Section,
		const int32 SortOrder,
		const bool bShowBeforeFirstEncounter,
		const bool bShowAfterFirstEncounter = true)
	{
		FProjectSocialCardRow& Row = OutRows.AddDefaulted_GetRef();
		Row.RowId = FName(RowId);
		Row.ValueId = FName(RowId);
		Row.Label = FText::FromString(Label);
		Row.Section = FName(Section);
		Row.SortOrder = SortOrder;
		Row.bEnabled = true;
		Row.bShowBeforeFirstEncounter = bShowBeforeFirstEncounter;
		Row.bShowAfterFirstEncounter = bShowAfterFirstEncounter;
	};

	AddRow(TEXT("Gender"), TEXT("Gender"), TEXT(""), 10, true);
	AddRow(TEXT("Personality"), TEXT("Personality"), TEXT(""), 20, true);
	AddRow(TEXT("Lust"), TEXT("Lust"), TEXT(""), 30, true);
	AddRow(TEXT("Control"), TEXT("Control"), TEXT(""), 40, true);
	AddRow(TEXT("Encounters"), TEXT("Encounters"), TEXT(""), 100, false);
	AddRow(TEXT("SatisfiedWins"), TEXT("Satisfied Wins"), TEXT(""), 110, false);
	AddRow(TEXT("FailedEncounters"), TEXT("Failed Encounters"), TEXT(""), 120, false);
	AddRow(TEXT("ClimaxCount"), TEXT("Climax"), TEXT(""), 125, false);
	AddRow(TEXT("Children"), TEXT("Childrens"), TEXT(""), 130, false);
	AddRow(TEXT("FirstEncounter"), TEXT("First Encounter"), TEXT(""), 140, false);
	AddRow(TEXT("TotalIntimateTime"), TEXT("Total Intimate Time"), TEXT(""), 150, false);
}

FText UProjectIntimacyDialogueLibrary::ResolvePartnerResponse(
	UDataTable* ResponseTable,
	const FName OptionId,
	const EProjectIntimacyPersonality Personality,
	const bool bAccepted)
{
	if (ResponseTable && ResponseTable->GetRowStruct() == FProjectIntimacyPartnerResponseRow::StaticStruct())
	{
		const FGameplayTag PersonalityTag = GetPersonalityTag(Personality);
		TArray<FProjectIntimacyPartnerResponseRow*> Rows;
		ResponseTable->GetAllRows(TEXT("ProjectIntimacyResponses"), Rows);
		for (const FProjectIntimacyPartnerResponseRow* Row : Rows)
		{
			if (!Row || Row->OptionId != OptionId)
			{
				continue;
			}

			if (Row->PersonalityTag.IsValid() && PersonalityTag.IsValid() && Row->PersonalityTag != PersonalityTag)
			{
				continue;
			}

			const FText Response = bAccepted ? Row->AcceptedResponse : Row->RefusedResponse;
			if (!Response.IsEmpty())
			{
				return Response;
			}
		}
	}

	if (!bAccepted)
	{
		return FText::FromString(TEXT("Not now."));
	}

	return FMath::RandBool() ? FText::FromString(TEXT("Yes!")) : FText::FromString(TEXT("Sure."));
}
