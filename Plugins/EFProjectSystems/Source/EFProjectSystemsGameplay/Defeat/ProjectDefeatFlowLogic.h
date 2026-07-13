#pragma once

#include "CoreMinimal.h"
#include "Defeat/ProjectDefeatTypes.h"

class EFPROJECTSYSTEMSGAMEPLAY_API FProjectDefeatFlowLogic
{
public:
	static bool ShouldKeepCombatSessionActive(
		float CurrentWorldTimeSeconds,
		float LastCombatEventTimeSeconds,
		float CombatWindowSeconds,
		int32 NearbyQualifiedEnemyCount);

	static bool IsCombatPressureActive(
		bool bQualifiedEnemyHit,
		bool bHasActiveCombatSession,
		int32 NearbyQualifiedEnemyCount);

	static bool ShouldEnterRepeatKnockoutDefeat(
		bool bAlreadyDowned,
		int32 ActiveCombatSessionId,
		int32 LastKnockoutCombatSessionId);

	static void EnforceMinimumTotalStruggleNotes(
		TArray<FProjectStruggleRound>& InOutRounds,
		int32 MinimumTotalNotes,
		int32 MaxNotesPerRound,
		float MaxSpacingSeconds);
};
