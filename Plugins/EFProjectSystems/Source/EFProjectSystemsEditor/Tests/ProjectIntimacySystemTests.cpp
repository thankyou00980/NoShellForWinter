#if WITH_DEV_AUTOMATION_TESTS

#include "Intimacy/ProjectIntimacyDialogueLibrary.h"
#include "Intimacy/ProjectIntimacySettings.h"
#include "Combat/ProjectCombatAttributeComponent.h"
#include "Combat/ProjectCombatTypes.h"
#include "Components/ACFDamageHandlerComponent.h"
#include "Locomotion/ProjectEmoteComponent.h"
#include "SinfulAscension/ProjectSinfulAscensionComponent.h"
#include "Survival/ProjectSurvivalNeedsComponent.h"
#include "Survival/ProjectSurvivalStatusComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyCombatShieldStateTest,
	"ACFUltimateSample.Intimacy.CombatShield.StateAndRestore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyCombatShieldStateTest::RunTest(const FString& Parameters)
{
	AActor* Player = NewObject<AActor>();
	AActor* Partner = NewObject<AActor>();
	UProjectEmoteComponent* EmoteComponent = NewObject<UProjectEmoteComponent>(Player);
	UACFDamageHandlerComponent* PlayerDamageHandler = NewObject<UACFDamageHandlerComponent>(Player);
	UACFDamageHandlerComponent* PartnerDamageHandler = NewObject<UACFDamageHandlerComponent>(Partner);
	TestNotNull(TEXT("Player should be constructible"), Player);
	TestNotNull(TEXT("Partner should be constructible"), Partner);
	TestNotNull(TEXT("Emote component should be constructible"), EmoteComponent);
	TestNotNull(TEXT("Player damage handler should be constructible"), PlayerDamageHandler);
	TestNotNull(TEXT("Partner damage handler should be constructible"), PartnerDamageHandler);
	if (!Player || !Partner || !EmoteComponent || !PlayerDamageHandler || !PartnerDamageHandler)
	{
		return false;
	}

	Player->AddInstanceComponent(EmoteComponent);
	Player->AddInstanceComponent(PlayerDamageHandler);
	Partner->AddInstanceComponent(PartnerDamageHandler);
	Player->SetCanBeDamaged(true);
	Partner->SetCanBeDamaged(true);
	PlayerDamageHandler->SetIsImmortal(false);
	PartnerDamageHandler->SetIsImmortal(false);

	EmoteComponent->AutomationApplyIntimacyCombatShieldForTest(Player, Partner);
	TestTrue(TEXT("Player should have intimacy shield tag"), ProjectCombatTags::IsActorIntimacyShielded(Player));
	TestTrue(TEXT("Partner should have intimacy shield tag"), ProjectCombatTags::IsActorIntimacyShielded(Partner));
	TestFalse(TEXT("Player damage should be disabled"), Player->CanBeDamaged());
	TestFalse(TEXT("Partner damage should be disabled"), Partner->CanBeDamaged());
	TestTrue(TEXT("Player ACF damage handler should be immortal"), PlayerDamageHandler->GetIsImmortal());
	TestTrue(TEXT("Partner ACF damage handler should be immortal"), PartnerDamageHandler->GetIsImmortal());
	TestTrue(TEXT("Damage cancellation should be blocked while shield is active"), EmoteComponent->IsDamageCancellationBlockedByIntimacyShield());

	EmoteComponent->AutomationRestoreIntimacyCombatShieldForTest();
	TestFalse(TEXT("Player shield tag should restore"), ProjectCombatTags::IsActorIntimacyShielded(Player));
	TestFalse(TEXT("Partner shield tag should restore"), ProjectCombatTags::IsActorIntimacyShielded(Partner));
	TestTrue(TEXT("Player damage flag should restore"), Player->CanBeDamaged());
	TestTrue(TEXT("Partner damage flag should restore"), Partner->CanBeDamaged());
	TestFalse(TEXT("Player ACF immortal flag should restore"), PlayerDamageHandler->GetIsImmortal());
	TestFalse(TEXT("Partner ACF immortal flag should restore"), PartnerDamageHandler->GetIsImmortal());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyCombatShieldEnsureTest,
	"ACFUltimateSample.Intimacy.CombatShield.EnsureReappliesLostState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyCombatShieldEnsureTest::RunTest(const FString& Parameters)
{
	AActor* Player = NewObject<AActor>();
	AActor* Partner = NewObject<AActor>();
	UProjectEmoteComponent* EmoteComponent = NewObject<UProjectEmoteComponent>(Player);
	TestNotNull(TEXT("Player should be constructible"), Player);
	TestNotNull(TEXT("Partner should be constructible"), Partner);
	TestNotNull(TEXT("Emote component should be constructible"), EmoteComponent);
	if (!Player || !Partner || !EmoteComponent)
	{
		return false;
	}

	Player->AddInstanceComponent(EmoteComponent);
	Player->SetCanBeDamaged(true);
	Partner->SetCanBeDamaged(true);
	EmoteComponent->AutomationApplyIntimacyCombatShieldForTest(Player, Partner);

	Player->Tags.Remove(ProjectCombatTags::IntimacyCombatShield());
	Player->SetCanBeDamaged(true);
	EmoteComponent->EnsureIntimacyCombatShield(Player, Partner);

	TestTrue(TEXT("Ensure should restore lost player shield tag"), ProjectCombatTags::IsActorIntimacyShielded(Player));
	TestFalse(TEXT("Ensure should disable player damage again"), Player->CanBeDamaged());
	TestTrue(TEXT("Partner should remain shielded"), ProjectCombatTags::IsActorIntimacyShielded(Partner));
	TestFalse(TEXT("Partner damage should remain disabled"), Partner->CanBeDamaged());

	EmoteComponent->AutomationRestoreIntimacyCombatShieldForTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyCombatShieldDamageBlockTest,
	"ACFUltimateSample.Intimacy.CombatShield.ProjectDamageBlocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyCombatShieldDamageBlockTest::RunTest(const FString& Parameters)
{
	AActor* Target = NewObject<AActor>();
	AActor* Source = NewObject<AActor>();
	UProjectCombatAttributeComponent* CombatComponent = NewObject<UProjectCombatAttributeComponent>(Target);
	TestNotNull(TEXT("Target should be constructible"), Target);
	TestNotNull(TEXT("Source should be constructible"), Source);
	TestNotNull(TEXT("Combat component should be constructible"), CombatComponent);
	if (!Target || !Source || !CombatComponent)
	{
		return false;
	}

	Target->AddInstanceComponent(CombatComponent);
	Target->Tags.AddUnique(ProjectCombatTags::IntimacyCombatShield());
	const float InitialHealth = CombatComponent->GetAttributeCurrentValue(CombatComponent->HealthAttributeName);

	FProjectCombatDamageSpec DamageSpec;
	DamageSpec.SourceActor = Source;
	DamageSpec.DamageCauser = Source;
	DamageSpec.TargetAttribute = CombatComponent->HealthAttributeName;
	DamageSpec.BaseDamage = 25.0f;
	const FProjectCombatDamageResult Result = CombatComponent->ApplyDamage(DamageSpec);

	TestTrue(TEXT("Requested damage should be recorded"), FMath::IsNearlyEqual(Result.RequestedDamage, 25.0f));
	TestTrue(TEXT("Applied damage should be zero while shielded"), FMath::IsNearlyZero(Result.AppliedDamage));
	TestTrue(TEXT("Shielded target health should remain unchanged"),
		FMath::IsNearlyEqual(CombatComponent->GetAttributeCurrentValue(CombatComponent->HealthAttributeName), InitialHealth));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyExhaustionStatusBlackoutBehaviorTest,
	"ACFUltimateSample.Intimacy.ExhaustionStatusBlackoutBehavior",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyExhaustionStatusBlackoutBehaviorTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UProjectSurvivalStatusComponent* StatusComponent = NewObject<UProjectSurvivalStatusComponent>(Owner);
	TestNotNull(TEXT("Owner should be constructible"), Owner);
	TestNotNull(TEXT("Status component should be constructible"), StatusComponent);
	if (!Owner || !StatusComponent)
	{
		return false;
	}

	Owner->AddInstanceComponent(StatusComponent);
	StatusComponent->ForceRefresh();

	TestTrue(TEXT("ExhaustedRecovery should apply as a non-blackout debuff"), StatusComponent->ApplyStatus(TEXT("ExhaustedRecovery"), 0.f, Owner));
	TestFalse(TEXT("ExhaustedRecovery should not trigger the blackout sequence"), StatusComponent->IsBlackoutActive());

	StatusComponent->ClearStatus(TEXT("ExhaustedRecovery"));
	TestTrue(TEXT("Exhausted should still apply"), StatusComponent->ApplyStatus(TEXT("Exhausted"), 0.f, Owner));
	TestTrue(TEXT("Exhausted should still trigger blackout"), StatusComponent->IsBlackoutActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyTiredStatusBehaviorTest,
	"ACFUltimateSample.Intimacy.TiredStatusBehavior",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyTiredStatusBehaviorTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UProjectSurvivalNeedsComponent* NeedsComponent = NewObject<UProjectSurvivalNeedsComponent>(Owner);
	UProjectSurvivalStatusComponent* StatusComponent = NewObject<UProjectSurvivalStatusComponent>(Owner);
	UProjectSinfulAscensionComponent* SinComponent = NewObject<UProjectSinfulAscensionComponent>(Owner);
	TestNotNull(TEXT("Owner should be constructible"), Owner);
	TestNotNull(TEXT("Needs component should be constructible"), NeedsComponent);
	TestNotNull(TEXT("Status component should be constructible"), StatusComponent);
	TestNotNull(TEXT("Sinful Ascension component should be constructible"), SinComponent);
	if (!Owner || !NeedsComponent || !StatusComponent || !SinComponent)
	{
		return false;
	}

	Owner->AddInstanceComponent(NeedsComponent);
	Owner->AddInstanceComponent(StatusComponent);
	Owner->AddInstanceComponent(SinComponent);
	StatusComponent->ForceRefresh();
	StatusComponent->SetForcedStatusActive(TEXT("Tired"), true);

	TestTrue(TEXT("Tired should activate as a forced penalty"), StatusComponent->IsStatusActive(TEXT("Tired")));
	TestFalse(TEXT("Tired should not trigger blackout"), StatusComponent->IsBlackoutActive());
	TestTrue(TEXT("Tired should expose a 1.25x Sleep decay multiplier"),
		FMath::IsNearlyEqual(StatusComponent->GetNeedDecayMultiplier(TEXT("Sleep")), 1.25f, 0.0001f));
	TestTrue(TEXT("Tired should expose a 0.2 Madness gain per second"),
		FMath::IsNearlyEqual(StatusComponent->GetSensationDeltaPerSecond(TEXT("Madness")), 0.2f, 0.0001f));

	SinComponent->NotifySleepCompleted(TEXT("Automation.Sleep"));
	TestFalse(TEXT("Sleeping should clear Tired"), StatusComponent->IsStatusActive(TEXT("Tired")));
	TestTrue(TEXT("Sleep decay should return to normal after Tired is cleared"),
		FMath::IsNearlyEqual(StatusComponent->GetNeedDecayMultiplier(TEXT("Sleep")), 1.f, 0.0001f));
	TestTrue(TEXT("Madness gain should stop after Tired is cleared"),
		FMath::IsNearlyEqual(StatusComponent->GetSensationDeltaPerSecond(TEXT("Madness")), 0.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyLustScalingTest,
	"ACFUltimateSample.Intimacy.LustScaling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyLustScalingTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Level 1 partner Lust should be 10000"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputePartnerMaxLust(1), 10000.0f));
	TestTrue(TEXT("Level 100 partner Lust should be 50000"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputePartnerMaxLust(100), 50000.0f));
	TestTrue(TEXT("Below-range levels should clamp to the minimum Lust"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputePartnerMaxLust(-5), 10000.0f));
	TestTrue(TEXT("Above-range levels should clamp to the maximum Lust"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputePartnerMaxLust(200), 50000.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyAllureDrainAndPleaseTest,
	"ACFUltimateSample.Intimacy.AllureDrainAndPlease",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyAllureDrainAndPleaseTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Allure level 1 should drain 5 Lust per second"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputeAllureDrainPerSecond(1), 5.0f));
	TestTrue(TEXT("Allure level 20 should drain 100 Lust per second"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputeAllureDrainPerSecond(20), 100.0f));
	TestTrue(TEXT("Five Please hits should multiply the Allure base drain"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputePleaseInstantDrain(1, 5), 25.0f));
	TestTrue(TEXT("No Please hits should drain nothing instantly"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputePleaseInstantDrain(50, 0), 0.0f));
	TestTrue(TEXT("Allure should make the Please target range wider"),
		UProjectIntimacySettings::ComputePleaseTargetHalfRange(50, 50) > UProjectIntimacySettings::ComputePleaseTargetHalfRange(50, 0));
	TestTrue(TEXT("Allure should make the Please pulse slower"),
		UProjectIntimacySettings::ComputePleasePulsePeriod(50, 50) > UProjectIntimacySettings::ComputePleasePulsePeriod(50, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyClimaxTest,
	"ACFUltimateSample.Intimacy.Climax",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyClimaxTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Level 4 partner Climax threshold should be 1400"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputeClimaxThreshold(4), 1400.0f));
	TestTrue(TEXT("Level 100 partner Climax threshold should be 11000"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputeClimaxThreshold(100), 11000.0f));

	float RemainingProgress = 0.0f;
	TestEqual(TEXT("Drain below threshold should not trigger Climax"),
		UProjectIntimacySettings::ConsumeClimaxProgress(100.0f, 250.0f, 1000.0f, RemainingProgress),
		0);
	TestTrue(TEXT("Drain below threshold should accumulate remaining progress"),
		FMath::IsNearlyEqual(RemainingProgress, 350.0f));

	TestEqual(TEXT("Crossing one threshold should trigger one Climax"),
		UProjectIntimacySettings::ConsumeClimaxProgress(900.0f, 250.0f, 1000.0f, RemainingProgress),
		1);
	TestTrue(TEXT("Climax should preserve residual progress"),
		FMath::IsNearlyEqual(RemainingProgress, 150.0f));

	TestEqual(TEXT("Large effective drain should count multiple hidden Climaxes"),
		UProjectIntimacySettings::ConsumeClimaxProgress(900.0f, 2300.0f, 1000.0f, RemainingProgress),
		3);
	TestTrue(TEXT("Multiple Climaxes should preserve residual progress"),
		FMath::IsNearlyEqual(RemainingProgress, 200.0f));

	TestTrue(TEXT("Climax anticipation should start at one multiplier outside the window"),
		FMath::IsNearlyEqual(UProjectIntimacySettings::ComputeClimaxAnticipationMultiplier(799.0f, 1000.0f), 1.0f));
	TestTrue(TEXT("Climax anticipation should increase inside the final 200 drain"),
		UProjectIntimacySettings::ComputeClimaxAnticipationMultiplier(900.0f, 1000.0f) > 1.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyTalkRecruitVisibilityTest,
	"ACFUltimateSample.Intimacy.TalkRecruitVisibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyTalkRecruitVisibilityTest::RunTest(const FString& Parameters)
{
	TArray<FProjectIntimacyTalkOptionRow> LockedOptions;
	UProjectIntimacyDialogueLibrary::BuildFallbackTalkOptions(LockedOptions, false);
	TestFalse(TEXT("Recruit should be hidden before ally eligibility"),
		LockedOptions.ContainsByPredicate([](const FProjectIntimacyTalkOptionRow& Row)
		{
			return Row.Action == EProjectIntimacyTalkAction::Recruit;
		}));

	TArray<FProjectIntimacyTalkOptionRow> EligibleOptions;
	UProjectIntimacyDialogueLibrary::BuildFallbackTalkOptions(EligibleOptions, true);
	TestTrue(TEXT("Recruit should appear only when ally eligibility is met"),
		EligibleOptions.ContainsByPredicate([](const FProjectIntimacyTalkOptionRow& Row)
		{
			return Row.Action == EProjectIntimacyTalkAction::Recruit;
		}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyControlStateTest,
	"ACFUltimateSample.Intimacy.ControlState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyControlStateTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Control should clamp below zero"), UProjectIntimacySettings::ClampControlPoints(-10), 0);
	TestEqual(TEXT("Control should clamp above max"), UProjectIntimacySettings::ClampControlPoints(999), 499);
	TestEqual(TEXT("Please hit lowers Control by 10"), UProjectIntimacySettings::ApplyControlDelta(225, -10), 215);
	TestEqual(TEXT("Please miss raises Control by 10"), UProjectIntimacySettings::ApplyControlDelta(225, 10), 235);
	TestEqual(TEXT("Stallion initial Control should be Overpowering"),
		UProjectIntimacySettings::GetControlState(UProjectIntimacySettings::ComputeInitialControl(EProjectIntimacyPersonality::Stallion)),
		EProjectIntimacyControlState::Overpowering);
	TestTrue(TEXT("Overpowering should resist passive drain"),
		UProjectIntimacySettings::ComputeControlDrainMultiplier(425) < UProjectIntimacySettings::ComputeControlDrainMultiplier(50));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyRelationshipTagsTest,
	"ACFUltimateSample.Intimacy.RelationshipTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyRelationshipTagsTest::RunTest(const FString& Parameters)
{
	FGameplayTagContainer Tags;
	const FGameplayTag MaleTag = FGameplayTag::RequestGameplayTag(TEXT("Project.Gender.Male"), false);
	UProjectIntimacyDialogueLibrary::BuildRelationshipTagsFromCounters(30, 0, 0, MaleTag, false, Tags);
	TestTrue(TEXT("30 encounters should grant Devoted"),
		Tags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Relationship.Devoted"), false)));
	TestTrue(TEXT("Devoted should force Chill behavior"),
		UProjectIntimacyDialogueLibrary::RelationshipTagsForceChill(Tags));
	TestTrue(TEXT("Devoted should allow Recruit"),
		UProjectIntimacyDialogueLibrary::RelationshipTagsAllowRecruit(Tags));

	UProjectIntimacyDialogueLibrary::BuildRelationshipTagsFromCounters(2, 5, 10, MaleTag, false, Tags);
	TestTrue(TEXT("5 children should grant Bull"),
		Tags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Relationship.Bull"), false)));
	TestTrue(TEXT("10 failed encounters should grant Master"),
		Tags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Relationship.Master"), false)));
	TestFalse(TEXT("Husband should not appear without the future ring"),
		Tags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Relationship.Husband"), false)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyTalkTagsTest,
	"ACFUltimateSample.Intimacy.TalkTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyTalkTagsTest::RunTest(const FString& Parameters)
{
	FGameplayTagContainer PreferredTags;
	FGameplayTagContainer EmptyRelationships;
	UProjectIntimacyDialogueLibrary::BuildPreferredTalkTags(
		EProjectIntimacyPersonality::Stallion,
		FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Control.Overpowering"), false),
		EmptyRelationships,
		PreferredTags);
	TestTrue(TEXT("Stallion should prefer Dominant Talk"),
		PreferredTags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Talk.Tag.Dominant"), false)));
	TestTrue(TEXT("Stallion should prefer Submissive Talk as one valid RPG style"),
		PreferredTags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Project.Intimacy.Talk.Tag.Submissive"), false)));

	TArray<FProjectIntimacyTalkOptionRow> Options;
	UProjectIntimacyDialogueLibrary::BuildFallbackTalkOptions(Options, true);
	const FProjectIntimacyTalkOptionRow* MoreOption = Options.FindByPredicate([](const FProjectIntimacyTalkOptionRow& Row)
	{
		return Row.OptionId == TEXT("Talk.More");
	});
	TestNotNull(TEXT("Fallback Talk should contain More"), MoreOption);
	if (MoreOption)
	{
		TestTrue(TEXT("More should score against Stallion preferred tags"),
			UProjectIntimacyDialogueLibrary::ScoreTalkOptionForPreferredTags(*MoreOption, PreferredTags) > 0);
		TestEqual(TEXT("More should live in Submissive category"), MoreOption->CategoryId, FName(TEXT("Talk.Category.Submissive")));
		TestTrue(TEXT("More should drain 25 Lust"), FMath::IsNearlyEqual(MoreOption->LustDrain, 25.0f));
	}

	const FProjectIntimacyTalkOptionRow* PatheticOption = Options.FindByPredicate([](const FProjectIntimacyTalkOptionRow& Row)
	{
		return Row.OptionId == TEXT("Talk.Pathetic");
	});
	TestNotNull(TEXT("Fallback Talk should contain Pathetic"), PatheticOption);
	if (PatheticOption)
	{
		TestEqual(TEXT("Pathetic should live in Dominant category"), PatheticOption->CategoryId, FName(TEXT("Talk.Category.Dominant")));
		TestEqual(TEXT("Pathetic should lower Control by 25"), PatheticOption->ControlDelta, -25);
	}

	const FProjectIntimacyTalkOptionRow* ComplimentOption = Options.FindByPredicate([](const FProjectIntimacyTalkOptionRow& Row)
	{
		return Row.OptionId == TEXT("Talk.Compliment");
	});
	TestNotNull(TEXT("Fallback Talk should contain Compliment"), ComplimentOption);
	if (ComplimentOption)
	{
		TestEqual(TEXT("Compliment should live in Neutral category"), ComplimentOption->CategoryId, FName(TEXT("Talk.Category.Neutral")));
		TestEqual(TEXT("Compliment should grant 10 SXP"), ComplimentOption->SxpReward, 10);
		TestEqual(TEXT("Compliment should not change Control"), ComplimentOption->ControlDelta, 0);
		TestTrue(TEXT("Compliment should not drain Lust"), FMath::IsNearlyZero(ComplimentOption->LustDrain));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSocialCardRowsTest,
	"ACFUltimateSample.Intimacy.SocialCardRows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSocialCardRowsTest::RunTest(const FString& Parameters)
{
	TArray<FProjectSocialCardRow> Rows;
	UProjectIntimacyDialogueLibrary::BuildFallbackSocialCardRows(Rows);
	TestEqual(TEXT("Fallback Social Card should contain the requested compact row set"), Rows.Num(), 11);

	if (Rows.Num() == 11)
	{
		TestEqual(TEXT("Gender should be first"), Rows[0].ValueId, FName(TEXT("Gender")));
		TestEqual(TEXT("Personality should be second"), Rows[1].ValueId, FName(TEXT("Personality")));
		TestEqual(TEXT("Lust should be third"), Rows[2].ValueId, FName(TEXT("Lust")));
		TestEqual(TEXT("Control should be fourth"), Rows[3].ValueId, FName(TEXT("Control")));
		TestTrue(TEXT("Social Card rows should not use internal section headings"), Rows[0].Section.IsNone());
	}

	const FProjectSocialCardRow* LustRow = Rows.FindByPredicate([](const FProjectSocialCardRow& Row)
	{
		return Row.ValueId == TEXT("Lust");
	});
	TestNotNull(TEXT("Social Card should expose Lust before first encounter"), LustRow);
	if (LustRow)
	{
		TestTrue(TEXT("Lust should show before first encounter"), LustRow->bShowBeforeFirstEncounter);
	}

	const FProjectSocialCardRow* FailedRow = Rows.FindByPredicate([](const FProjectSocialCardRow& Row)
	{
		return Row.ValueId == TEXT("FailedEncounters");
	});
	TestNotNull(TEXT("Social Card should expose Failed Encounters after first encounter"), FailedRow);
	if (FailedRow)
	{
		TestFalse(TEXT("Failed Encounters should be hidden before first encounter"), FailedRow->bShowBeforeFirstEncounter);
		TestTrue(TEXT("Failed Encounters should show after first encounter"), FailedRow->bShowAfterFirstEncounter);
	}

	const FProjectSocialCardRow* ChildrenRow = Rows.FindByPredicate([](const FProjectSocialCardRow& Row)
	{
		return Row.ValueId == TEXT("Children");
	});
	TestNotNull(TEXT("Social Card should expose Childrens after first encounter"), ChildrenRow);
	if (ChildrenRow)
	{
		TestEqual(TEXT("Children row should use requested label"), ChildrenRow->Label.ToString(), FString(TEXT("Childrens")));
	}

	const FProjectSocialCardRow* ClimaxRow = Rows.FindByPredicate([](const FProjectSocialCardRow& Row)
	{
		return Row.ValueId == TEXT("ClimaxCount");
	});
	TestNotNull(TEXT("Social Card should expose Climax after first encounter"), ClimaxRow);
	if (ClimaxRow)
	{
		TestEqual(TEXT("Climax row should use requested label"), ClimaxRow->Label.ToString(), FString(TEXT("Climax")));
		TestFalse(TEXT("Climax should be hidden before first encounter"), ClimaxRow->bShowBeforeFirstEncounter);
		TestTrue(TEXT("Climax should show after first encounter"), ClimaxRow->bShowAfterFirstEncounter);
	}

	TestFalse(TEXT("Relationship should not be part of the compact Social Card"), Rows.ContainsByPredicate([](const FProjectSocialCardRow& Row)
	{
		return Row.ValueId == TEXT("Relationship");
	}));
	TestFalse(TEXT("Affect should not be part of the compact Social Card"), Rows.ContainsByPredicate([](const FProjectSocialCardRow& Row)
	{
		return Row.ValueId == TEXT("Affect");
	}));
	TestFalse(TEXT("Ally should not be part of the compact Social Card"), Rows.ContainsByPredicate([](const FProjectSocialCardRow& Row)
	{
		return Row.ValueId == TEXT("Ally");
	}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectIntimacyMediaCueRowsTest,
	"ACFUltimateSample.Intimacy.MediaCueRows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectIntimacyMediaCueRowsTest::RunTest(const FString& Parameters)
{
	TArray<FProjectIntimacyMediaCueRow> Rows;
	UProjectIntimacyDialogueLibrary::BuildFallbackMediaCues(Rows);
	TestEqual(TEXT("Fallback media cues should contain More and Climax previews"), Rows.Num(), 2);

	if (Rows.Num() >= 1)
	{
		const FProjectIntimacyMediaCueRow& Cue = Rows[0];
		TestEqual(TEXT("More preview should be bound to Talk.More"), Cue.TriggerOptionId, FName(TEXT("Talk.More")));
		TestEqual(TEXT("More preview should be an image cue"), Cue.MediaType, EProjectIntimacyMediaType::Image);
		TestEqual(TEXT("More preview should use the requested source PNG"), Cue.SourceImagePath, FString(TEXT("_Game/Images/Intimacy/Preview_IntimacyImage.png")));
		TestTrue(TEXT("More preview should fade in for one second"), FMath::IsNearlyEqual(Cue.FadeInSeconds, 1.0f));
		TestTrue(TEXT("More preview should hold for two seconds"), FMath::IsNearlyEqual(Cue.HoldSeconds, 2.0f));
		TestTrue(TEXT("More preview should fade out for one second"), FMath::IsNearlyEqual(Cue.FadeOutSeconds, 1.0f));
		TestTrue(TEXT("Media cue source size should preserve the 1080x720 authoring target"),
			Cue.SourceMediaSize.Equals(FVector2D(1080.0f, 720.0f)));
		TestTrue(TEXT("Requested preview PNG should exist in project Content"),
			FPaths::FileExists(FPaths::Combine(FPaths::ProjectContentDir(), Cue.SourceImagePath)));
	}

	const FProjectIntimacyMediaCueRow* ClimaxCue = Rows.FindByPredicate([](const FProjectIntimacyMediaCueRow& Row)
	{
		return Row.CueId == TEXT("Climax.Preview");
	});
	TestNotNull(TEXT("Fallback media cues should contain Climax preview"), ClimaxCue);
	if (ClimaxCue)
	{
		TestEqual(TEXT("Climax preview should be bound to Climax event"), ClimaxCue->TriggerEventId, FName(TEXT("Climax")));
		TestEqual(TEXT("Climax preview should use the requested source PNG"), ClimaxCue->SourceImagePath, FString(TEXT("_Game/Images/Intimacy/Preview_IntimacyImage.png")));
	}

	return true;
}

#endif
