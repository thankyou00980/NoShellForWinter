#include "Defeat/ProjectDefeatFlowLogic.h"

bool FProjectDefeatFlowLogic::ShouldKeepCombatSessionActive(
	const float CurrentWorldTimeSeconds,
	const float LastCombatEventTimeSeconds,
	const float CombatWindowSeconds,
	const int32 NearbyQualifiedEnemyCount)
{
	return NearbyQualifiedEnemyCount > 0
		|| (CurrentWorldTimeSeconds - LastCombatEventTimeSeconds) <= CombatWindowSeconds;
}

bool FProjectDefeatFlowLogic::IsCombatPressureActive(
	const bool bQualifiedEnemyHit,
	const bool bHasActiveCombatSession,
	const int32 NearbyQualifiedEnemyCount)
{
	return bQualifiedEnemyHit || bHasActiveCombatSession || NearbyQualifiedEnemyCount > 0;
}

bool FProjectDefeatFlowLogic::ShouldEnterRepeatKnockoutDefeat(
	const bool bAlreadyDowned,
	const int32 ActiveCombatSessionId,
	const int32 LastKnockoutCombatSessionId)
{
	return bAlreadyDowned
		|| (ActiveCombatSessionId != 0 && LastKnockoutCombatSessionId == ActiveCombatSessionId);
}

void FProjectDefeatFlowLogic::EnforceMinimumTotalStruggleNotes(
	TArray<FProjectStruggleRound>& InOutRounds,
	const int32 MinimumTotalNotes,
	const int32 MaxNotesPerRound,
	const float MaxSpacingSeconds)
{
	if (InOutRounds.Num() <= 0)
	{
		return;
	}

	int32 CurrentTotalNotes = 0;
	for (const FProjectStruggleRound& Round : InOutRounds)
	{
		CurrentTotalNotes += Round.NoteCount;
	}

	int32 RemainingNotesToAdd = FMath::Max(0, MinimumTotalNotes - CurrentTotalNotes);
	while (RemainingNotesToAdd > 0)
	{
		bool bAddedNoteThisPass = false;

		for (FProjectStruggleRound& Round : InOutRounds)
		{
			if (RemainingNotesToAdd <= 0)
			{
				break;
			}

			if (Round.NoteCount >= MaxNotesPerRound)
			{
				continue;
			}

			++Round.NoteCount;
			Round.DurationSeconds = Round.TravelTimeSeconds + (Round.NoteCount * MaxSpacingSeconds);
			--RemainingNotesToAdd;
			bAddedNoteThisPass = true;
		}

		if (!bAddedNoteThisPass)
		{
			break;
		}
	}
}
