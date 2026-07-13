#include "Defeat/ProjectDefeatFlowLogic.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatFlowCombatSessionPersistsTest,
	"ACFUltimateSample.Defeat.Flow.CombatSessionPersistsWithNearbyEnemies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatFlowCombatSessionPersistsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const bool bKeepActive = FProjectDefeatFlowLogic::ShouldKeepCombatSessionActive(
		20.0f,
		10.0f,
		5.0f,
		1);

	TestTrue(TEXT("A combat session should stay active while a qualified enemy remains nearby."), bKeepActive);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatFlowCombatSessionEndsTest,
	"ACFUltimateSample.Defeat.Flow.CombatSessionEndsWithoutNearbyEnemies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatFlowCombatSessionEndsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const bool bKeepActive = FProjectDefeatFlowLogic::ShouldKeepCombatSessionActive(
		20.0f,
		10.0f,
		5.0f,
		0);

	TestFalse(TEXT("A combat session should end once the grace window expires and no enemies remain nearby."), bKeepActive);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatFlowRepeatKnockoutTest,
	"ACFUltimateSample.Defeat.Flow.RepeatKnockoutDirectDefeat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatFlowRepeatKnockoutTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	TestTrue(
		TEXT("Being downed again while already downed should force defeated."),
		FProjectDefeatFlowLogic::ShouldEnterRepeatKnockoutDefeat(true, 0, 0));

	TestTrue(
		TEXT("A second lethal hit inside the same combat session should force defeated."),
		FProjectDefeatFlowLogic::ShouldEnterRepeatKnockoutDefeat(false, 3, 3));

	TestFalse(
		TEXT("A new combat session should allow another knockout before defeated."),
		FProjectDefeatFlowLogic::ShouldEnterRepeatKnockoutDefeat(false, 4, 3));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatFlowMinNotesTest,
	"ACFUltimateSample.Defeat.Flow.MinimumStruggleNotes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatFlowMinNotesTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	TArray<FProjectStruggleRound> Rounds;
	FProjectStruggleRound& FirstRound = Rounds.AddDefaulted_GetRef();
	FirstRound.NoteCount = 4;
	FirstRound.TravelTimeSeconds = 1.5f;
	FirstRound.DurationSeconds = 1.5f + (FirstRound.NoteCount * 0.5f);

	FProjectStruggleRound& SecondRound = Rounds.AddDefaulted_GetRef();
	SecondRound.NoteCount = 5;
	SecondRound.TravelTimeSeconds = 1.5f;
	SecondRound.DurationSeconds = 1.5f + (SecondRound.NoteCount * 0.5f);

	FProjectDefeatFlowLogic::EnforceMinimumTotalStruggleNotes(Rounds, 20, 12, 0.5f);

	int32 TotalNotes = 0;
	for (const FProjectStruggleRound& Round : Rounds)
	{
		TotalNotes += Round.NoteCount;
		TestTrue(TEXT("No struggle round should exceed the configured per-round cap."), Round.NoteCount <= 12);
		TestTrue(TEXT("Round duration should still include note travel spacing."), Round.DurationSeconds >= Round.TravelTimeSeconds);
	}

	TestEqual(TEXT("The helper should grow the full struggle queue to the configured minimum note count."), TotalNotes, 20);
	return true;
}

#endif
