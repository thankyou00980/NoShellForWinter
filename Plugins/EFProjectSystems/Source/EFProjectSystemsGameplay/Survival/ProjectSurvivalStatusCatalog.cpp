#include "Survival/ProjectSurvivalStatusCatalog.h"

namespace
{
	FProjectSurvivalStatusDefinition MakeStatus(
		const FName StatusName,
		const TCHAR* DisplayName,
		const FName MinimalIconName,
		const float DamagePerSecond = 0.f,
		const float DurationSeconds = 0.f,
		const int32 HudPriority = 0,
		const TCHAR* Description = TEXT(""))
	{
		FProjectSurvivalStatusDefinition Definition;
		Definition.StatusName = StatusName;
		Definition.DisplayName = DisplayName;
		Definition.Description = Description;
		Definition.MinimalIconName = MinimalIconName;
		Definition.DamagePerSecond = DamagePerSecond;
		Definition.DurationSeconds = DurationSeconds;
		Definition.MovementInputScale = 1.f;
		Definition.ReapplyPolicy = EProjectSurvivalStatusRefreshPolicy::RefreshDuration;
		Definition.Tint = FLinearColor::White;
		Definition.HudPriority = HudPriority;
		Definition.HudSlotSize = FVector2D(300.f, 110.f);
		Definition.HudIconSize = FVector2D(70.f, 70.f);
		Definition.HudIconSlotOffset = FVector2D(0.f, -17.f);
		return Definition;
	}
}

const FProjectSurvivalStatusCatalog& GetProjectSurvivalStatusCatalog()
{
	static const FProjectSurvivalStatusCatalog Catalog = []()
	{
		FProjectSurvivalStatusCatalog Data;
		Data.StatusHudOffset = FVector2D(456.f, 466.f);
		Data.StatusIconSize = FVector2D(70.f, 70.f);
		Data.StatusIconSpacing = 8.f;
		Data.StatusIconsPerRow = 5;
		Data.bEnableDebugStatusCycling = true;
		Data.DebugCycleStatusNames = {
			TEXT("Bleeding"),
			TEXT("Dizzy"),
			TEXT("Fear"),
			TEXT("Tired"),
			TEXT("SleepDeprived"),
			TEXT("OrgasmRush"),
			TEXT("Frenzy"),
			TEXT("ExtremePain"),
			TEXT("Dirty"),
			TEXT("Sweaty"),
			TEXT("KnockedOut")
		};

		FProjectSurvivalStatusDefinition Starving = MakeStatus(TEXT("Starving"), TEXT("STARVING"), TEXT("Status.Starving"), 1.f, 0.f, 620, TEXT("Hunger is empty; health recovery is blocked and damage starts ticking."));
		Starving.SourceNeedName = TEXT("Hunger");
		Starving.bBlocksHealthRecovery = true;
		Starving.bTriggerAtNeedEmpty = true;

		FProjectSurvivalStatusDefinition Thirst = MakeStatus(TEXT("Thirst"), TEXT("THIRST"), TEXT("Status.Thirst"), 1.f, 0.f, 630, TEXT("Thirst is empty; health recovery is blocked and damage starts ticking."));
		Thirst.SourceNeedName = TEXT("Thirst");
		Thirst.bBlocksHealthRecovery = true;
		Thirst.bTriggerAtNeedEmpty = true;

		FProjectSurvivalStatusDefinition SleepDeprived = MakeStatus(TEXT("SleepDeprived"), TEXT("SLEEP DEPRIVED"), TEXT("Status.Exhausted"), 0.f, 0.f, 520, TEXT("Sleep is empty and exhaustion pressure is rising."));
		SleepDeprived.SourceNeedName = TEXT("Sleep");
		SleepDeprived.bTriggerAtNeedEmpty = true;

		FProjectSurvivalStatusDefinition Tired = MakeStatus(TEXT("Tired"), TEXT("TIRED"), TEXT("Status.Exhausted"), 0.f, 0.f, 510, TEXT("Sleep drains faster and Madness rises until the character sleeps."));
		Tired.NeedDecayModifiers = {
			FProjectSurvivalStatusNeedDecayModifier(TEXT("Sleep"), 1.25f)
		};
		Tired.SensationModifiers = {
			FProjectSurvivalStatusSensationModifier(TEXT("Madness"), 0.2f)
		};

		FProjectSurvivalStatusDefinition Exhausted = MakeStatus(TEXT("Exhausted"), TEXT("EXHAUSTED"), TEXT("Status.Exhausted"), 0.f, 15.f, 760, TEXT("The character is collapsing into an exhaustion blackout."));
		Exhausted.bTriggerAtNeedEmpty = false;
		Exhausted.bTriggersExhaustionSequence = true;

		FProjectSurvivalStatusDefinition ExhaustedRecovery = MakeStatus(TEXT("ExhaustedRecovery"), TEXT("RECOVERY"), TEXT("Status.Exhausted"), 0.f, 8.f, 500, TEXT("Temporary weakness after recovering from exhaustion."));
		ExhaustedRecovery.AttributeModifiers = {
			FProjectSurvivalStatusAttributeModifier(TEXT("MeleeDamage"), 0.85f),
			FProjectSurvivalStatusAttributeModifier(TEXT("RangedDamage"), 0.85f),
			FProjectSurvivalStatusAttributeModifier(TEXT("SpellDamage"), 0.85f)
		};

		FProjectSurvivalStatusDefinition Frenzy = MakeStatus(TEXT("Frenzy"), TEXT("FRENZY"), TEXT("Status.Dizzy"), 0.f, 0.f, 820, TEXT("Movement is unstable and partially inverted."));
		Frenzy.bInvertMovementInput = true;
		Frenzy.MovementInputScale = 0.80f;
		Frenzy.Tint = FLinearColor(0.90f, 0.18f, 0.12f, 1.f);

		FProjectSurvivalStatusDefinition OrgasmRush = MakeStatus(TEXT("OrgasmRush"), TEXT("ORGASM RUSH"), TEXT("Status.Fear"), 0.f, 3.f, 800, TEXT("A brief lust spike is overriding normal control."));
		OrgasmRush.Tint = FLinearColor(0.92f, 0.25f, 0.38f, 1.f);

		FProjectSurvivalStatusDefinition ExtremePain = MakeStatus(TEXT("ExtremePain"), TEXT("EXTREME PAIN"), TEXT("Status.Bleeding"), 0.f, 10.f, 900, TEXT("Pain has reached a dangerous overload state."));
		ExtremePain.Tint = FLinearColor(0.84f, 0.82f, 0.72f, 1.f);

		FProjectSurvivalStatusDefinition GraceStep = MakeStatus(TEXT("GraceStep"), TEXT("GRACE STEP"), TEXT("Status.Dizzy"), 0.f, 2.f, 420, TEXT("A brief movement grace window is active."));
		GraceStep.Tint = FLinearColor(0.44f, 0.80f, 0.96f, 1.f);

		FProjectSurvivalStatusDefinition KnockedOut = MakeStatus(TEXT("KnockedOut"), TEXT("KNOCKED OUT"), TEXT("Status.Exhausted"), 0.f, 10.f, 1000, TEXT("The character is incapacitated."));
		KnockedOut.Tint = FLinearColor(0.52f, 0.48f, 0.42f, 1.f);

		FProjectSurvivalStatusDefinition Bleeding = MakeStatus(TEXT("Bleeding"), TEXT("BLEEDING"), TEXT("Status.Bleeding"), 1.f, 5.f, 850, TEXT("Taking periodic damage from an open wound."));
		Bleeding.bTriggerAtNeedEmpty = false;

		FProjectSurvivalStatusDefinition Dizzy = MakeStatus(TEXT("Dizzy"), TEXT("DIZZY"), TEXT("Status.Dizzy"), 0.f, 5.f, 700, TEXT("Movement direction is confused and heavily reduced."));
		Dizzy.bTriggerAtNeedEmpty = false;
		Dizzy.bInvertMovementInput = true;
		Dizzy.MovementInputScale = 0.55f;

		FProjectSurvivalStatusDefinition Fear = MakeStatus(TEXT("Fear"), TEXT("FEAR"), TEXT("Status.Fear"), 0.f, 5.f, 680, TEXT("Combat damage output is reduced by fear."));
		Fear.bTriggerAtNeedEmpty = false;
		Fear.AttributeModifiers = {
			FProjectSurvivalStatusAttributeModifier(TEXT("MeleeDamage"), 0.8f),
			FProjectSurvivalStatusAttributeModifier(TEXT("RangedDamage"), 0.8f),
			FProjectSurvivalStatusAttributeModifier(TEXT("SpellDamage"), 0.8f)
		};

		FProjectSurvivalStatusDefinition Dirty = MakeStatus(TEXT("Dirty"), TEXT("DIRTY"), TEXT("Status.Dirty"), 0.f, 0.f, 300, TEXT("Dirt or grime is active on the character."));
		Dirty.bTriggerAtNeedEmpty = false;
		Dirty.Tint = FLinearColor(0.34f, 0.25f, 0.16f, 1.f);

		FProjectSurvivalStatusDefinition Sweaty = MakeStatus(TEXT("Sweaty"), TEXT("SWEATY"), TEXT("Status.Dirty"), 0.f, 0.f, 310, TEXT("Sweat has fully built up and will remain noticeable until washed away."));
		Sweaty.bTriggerAtNeedEmpty = false;
		Sweaty.Tint = FLinearColor(0.35f, 0.58f, 0.64f, 1.f);

		Data.StatusDefinitions = {
			Starving,
			Thirst,
			Tired,
			SleepDeprived,
			Exhausted,
			ExhaustedRecovery,
			Frenzy,
			OrgasmRush,
			ExtremePain,
			GraceStep,
			KnockedOut,
			Bleeding,
			Dizzy,
			Fear,
			Dirty,
			Sweaty
		};

		FProjectSurvivalStatusIncomingHitRule BleedingRule;
		BleedingRule.StatusName = TEXT("Bleeding");
		BleedingRule.SourceClassNameHints = { TEXT("MeleeMale"), TEXT("ACFMeleeEnemyBP") };
		BleedingRule.ApplyChance = 0.10f;

		FProjectSurvivalStatusIncomingHitRule DizzyRule;
		DizzyRule.StatusName = TEXT("Dizzy");
		DizzyRule.SourceClassNameHints = { TEXT("MageMale"), TEXT("ACFMageEnemyBP") };
		DizzyRule.ApplyChance = 1.f;

		Data.IncomingHitRules = {
			BleedingRule,
			DizzyRule
		};

		return Data;
	}();

	return Catalog;
}
