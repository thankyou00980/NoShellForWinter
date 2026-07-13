#include "SinfulAscension/ProjectSinfulAscensionComponent.h"
#include "SinfulAscension/ProjectSinfulAscensionSettings.h"
#include "CharacterBackground/ProjectCharacterBackgroundComponent.h"
#include "CharacterBackground/ProjectCharacterBackgroundTypes.h"
#include "Combat/ProjectCombatAttributeComponent.h"
#include "Components/ACFDamageHandlerComponent.h"
#include "Defeat/ProjectDefeatInventoryBridge.h"
#include "Defeat/ProjectDefeatFlowSettings.h"
#include "GameFramework/Actor.h"
#include "Lockpicking/ProjectLockpickableComponent.h"
#include "Survival/ProjectSurvivalNeedsComponent.h"
#include "Survival/ProjectSurvivalStatusComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionUpgradeCostTest,
	"ACFUltimateSample.SinfulAscension.Progression.UpgradeCostCurve",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionUpgradeCostTest::RunTest(const FString& Parameters)
{
	const int32 Level0Cost = UProjectSinfulAscensionSettings::ComputeAttributeUpgradeCost(0);
	const int32 Level1Cost = UProjectSinfulAscensionSettings::ComputeAttributeUpgradeCost(1);
	const int32 Level4Cost = UProjectSinfulAscensionSettings::ComputeAttributeUpgradeCost(4);
	const int32 Level9Cost = UProjectSinfulAscensionSettings::ComputeAttributeUpgradeCost(9);

	TestEqual(TEXT("Level 0 -> 1 should start at 80 SXP"), Level0Cost, 80);
	TestTrue(TEXT("Level 1 cost should be higher than level 0"), Level1Cost > Level0Cost);
	TestTrue(TEXT("Level 4 cost should be higher than level 1"), Level4Cost > Level1Cost);
	TestTrue(TEXT("Level 9 cost should be higher than level 4"), Level9Cost > Level4Cost);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionSpendTest,
	"ACFUltimateSample.SinfulAscension.Progression.SpendRunSxpOnAttribute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionSpendTest::RunTest(const FString& Parameters)
{
	UProjectSinfulAscensionComponent* Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Component should be constructible"), Component);
	if (!Component)
	{
		return false;
	}

	const int32 FirstCost = Component->GetUpgradeCost(EProjectSinAttribute::Willpower);
	TestFalse(TEXT("Spend should fail without enough SXP"), Component->SpendSxpOnAttribute(EProjectSinAttribute::Willpower));
	TestEqual(TEXT("Willpower should remain level 0"), Component->GetAttributeLevel(EProjectSinAttribute::Willpower), 0);

	Component->GrantSxp(TEXT("AutomationTest"), FirstCost, false);
	TestTrue(TEXT("Spend should succeed when enough SXP is available"), Component->SpendSxpOnAttribute(EProjectSinAttribute::Willpower));
	TestEqual(TEXT("Willpower should increase to level 1"), Component->GetAttributeLevel(EProjectSinAttribute::Willpower), 1);
	TestEqual(TEXT("Run SXP should be consumed exactly"), Component->GetCurrentRunSxp(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionCunningCurveTest,
	"ACFUltimateSample.SinfulAscension.Cunning.PassiveCurve",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionCunningCurveTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	TestTrue(
		TEXT("Cunning level 0 should give no passive ratio."),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeCunningPassiveRatio(0, 10.f), 0.f, 0.0001f));
	TestTrue(
		TEXT("Cunning level 5 with pivot 5 should resolve to exactly half strength."),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeCunningPassiveRatio(5, 5.f), 0.5f, 0.0001f));
	TestTrue(
		TEXT("Cunning level 10 with pivot 10 should resolve to exactly half strength."),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeCunningPassiveRatio(10, 10.f), 0.5f, 0.0001f));

	const float HugeRatio = UProjectSinfulAscensionSettings::ComputeCunningPassiveRatio(1000000, 10.f);
	TestTrue(TEXT("Huge Cunning should approach one without reaching or exceeding it."), HugeRatio > 0.999f && HugeRatio < 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionCunningLockpickMathTest,
	"ACFUltimateSample.SinfulAscension.Cunning.LockpickMath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionCunningLockpickMathTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	constexpr int32 Difficulty = 50;
	const float PreviousBaselineSpeed = 1.f + static_cast<float>(Difficulty) * 0.025f;
	const float Level0Speed = UProjectLockpickableComponent::ComputeSpeedMultiplier(Difficulty, 0);
	TestTrue(
		TEXT("Cunning 0 should make lockpick 15 percent faster than the previous baseline."),
		FMath::IsNearlyEqual(Level0Speed, PreviousBaselineSpeed * 1.15f, 0.0001f));

	const float Level0HalfRange = UProjectLockpickableComponent::ComputeTargetHalfRange(Difficulty, 0);
	const float Level5HalfRange = UProjectLockpickableComponent::ComputeTargetHalfRange(Difficulty, 5);
	TestTrue(
		TEXT("Cunning 5 should increase the lockpick success half-range by exactly 50 percent before absolute caps."),
		FMath::IsNearlyEqual(Level5HalfRange, Level0HalfRange * 1.5f, 0.0001f));

	const float HugeSpeed = UProjectLockpickableComponent::ComputeSpeedMultiplier(Difficulty, 1000000);
	const float HugeHalfRange = UProjectLockpickableComponent::ComputeTargetHalfRange(Difficulty, 1000000);
	TestTrue(TEXT("Huge Cunning lockpick speed should stay inside gameplay clamps."), HugeSpeed >= 0.85f && HugeSpeed <= 4.0f);
	TestTrue(TEXT("Huge Cunning lockpick target range should stay inside gameplay clamps."), HugeHalfRange >= 0.035f && HugeHalfRange <= 0.18f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionCunningStruggleMathTest,
	"ACFUltimateSample.SinfulAscension.Cunning.StruggleMath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionCunningStruggleMathTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const float Level0Speed = UProjectSinfulAscensionSettings::ComputeCunningStruggleSpeedMultiplier(0, 1.30f, 0.50f, 10.f);
	TestTrue(TEXT("Cunning 0 should make struggle 30 percent faster."), FMath::IsNearlyEqual(Level0Speed, 1.30f, 0.0001f));
	TestEqual(
		TEXT("Cunning below the level 5 milestone should keep the default miss allowance."),
		UProjectSinfulAscensionSettings::ComputeCunningStruggleMaxMisses(4, 5, 5, 10),
		5);
	TestEqual(
		TEXT("Cunning level 5 should double the miss allowance from 5 to 10."),
		UProjectSinfulAscensionSettings::ComputeCunningStruggleMaxMisses(5, 5, 5, 10),
		10);

	const float HugeSpeed = UProjectSinfulAscensionSettings::ComputeCunningStruggleSpeedMultiplier(1000000, 1.30f, 0.50f, 10.f);
	TestTrue(TEXT("Huge Cunning struggle speed should approach the slow cap without dropping below it."), HugeSpeed >= 0.65f && HugeSpeed <= 1.30f);
	TestTrue(
		TEXT("Scaled hit windows should respect min and max clamps."),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeCunningScaledStruggleSeconds(0.05f, HugeSpeed, 0.14f, 0.36f), 0.14f, 0.0001f)
			&& FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeCunningScaledStruggleSeconds(1.0f, HugeSpeed, 0.14f, 0.36f), 0.36f, 0.0001f));
	TestTrue(
		TEXT("Scaled travel times should respect min and max clamps."),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeCunningScaledStruggleSeconds(0.05f, HugeSpeed, 0.85f, 2.40f), 0.85f, 0.0001f)
			&& FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeCunningScaledStruggleSeconds(4.0f, HugeSpeed, 0.85f, 2.40f), 2.40f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionCunningDefeatRetentionTest,
	"ACFUltimateSample.SinfulAscension.Cunning.DefeatRetention",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionCunningDefeatRetentionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	FProjectDefeatInventorySnapshot SourceSnapshot;
	SourceSnapshot.InventoryEntries.AddDefaulted(3);
	SourceSnapshot.EquipmentEntries.AddDefaulted(2);

	FProjectDefeatInventorySnapshot Cunning9Snapshot;
	FProjectDefeatInventoryBridge::BuildDefeatedRetainedSnapshotForCunning(
		SourceSnapshot,
		UProjectDefeatFlowSettings::Get(),
		1337,
		9,
		10,
		Cunning9Snapshot);
	TestEqual(TEXT("Cunning 9 should lose all retained equipment on defeat."), Cunning9Snapshot.EquipmentEntries.Num(), 0);
	TestTrue(TEXT("Cunning 9 may still retain the configured inventory subset."), Cunning9Snapshot.InventoryEntries.Num() <= SourceSnapshot.InventoryEntries.Num());

	FProjectDefeatInventorySnapshot Cunning10Snapshot;
	FProjectDefeatInventoryBridge::BuildDefeatedRetainedSnapshotForCunning(
		SourceSnapshot,
		UProjectDefeatFlowSettings::Get(),
		1337,
		10,
		10,
		Cunning10Snapshot);
	TestEqual(TEXT("Cunning 10 should retain all inventory entries."), Cunning10Snapshot.InventoryEntries.Num(), SourceSnapshot.InventoryEntries.Num());
	TestEqual(TEXT("Cunning 10 should retain all equipped armor/clothing entries."), Cunning10Snapshot.EquipmentEntries.Num(), SourceSnapshot.EquipmentEntries.Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionCelerityFlatSxpTest,
	"ACFUltimateSample.SinfulAscension.Celerity.FlatSxp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionCelerityFlatSxpTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const int32 FlatSxpPerLevel = Settings ? FMath::Max(0, Settings->CelerityFlatSxpPerLevel) : 2;

	UProjectSinfulAscensionComponent* Level0Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Level 0 component should be constructible"), Level0Component);
	if (!Level0Component)
	{
		return false;
	}

	TestEqual(TEXT("Celerity level 0 should not add flat SXP."), Level0Component->GrantSxp(TEXT("AutomationTest"), 100, false), 100);
	TestEqual(TEXT("Celerity level 0 should store only the base grant."), Level0Component->GetCurrentRunSxp(), 100);

	UProjectSinfulAscensionComponent* Level3Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Level 3 component should be constructible"), Level3Component);
	if (!Level3Component)
	{
		return false;
	}

	TestTrue(TEXT("Celerity should be levelable before testing the flat passive."), Level3Component->ApplyFreeAttributeLevels(EProjectSinAttribute::Celerity, 3));
	const int32 ExpectedLevel3Grant = 100 + (3 * FlatSxpPerLevel);
	TestEqual(TEXT("Celerity level 3 should add +2 SXP per level by default."), Level3Component->GrantSxp(TEXT("AutomationTest"), 100, false), ExpectedLevel3Grant);
	TestEqual(TEXT("Celerity level 3 should store the flat passive grant."), Level3Component->GetCurrentRunSxp(), ExpectedLevel3Grant);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionCelerityYMenuActionSxpTest,
	"ACFUltimateSample.SinfulAscension.Celerity.YMenuActionSxp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionCelerityYMenuActionSxpTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const int32 FlatSxpPerLevel = Settings ? FMath::Max(0, Settings->CelerityFlatSxpPerLevel) : 2;
	const float YMenuMultiplier = Settings ? FMath::Max(0.f, Settings->CelerityYMenuActionSxpMultiplier) : 1.5f;

	UProjectSinfulAscensionComponent* Level4Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Level 4 component should be constructible"), Level4Component);
	if (!Level4Component)
	{
		return false;
	}

	TestTrue(TEXT("Celerity level 4 should be available before the Y-menu milestone."), Level4Component->ApplyFreeAttributeLevels(EProjectSinAttribute::Celerity, 4));
	TestEqual(
		TEXT("Celerity level 4 should not apply the Y-menu source multiplier."),
		Level4Component->AutomationGrantYMenuActionSxp(TEXT("YMenu.Automation"), 100, false),
		100 + (4 * FlatSxpPerLevel));

	UProjectSinfulAscensionComponent* Level5Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Level 5 component should be constructible"), Level5Component);
	if (!Level5Component)
	{
		return false;
	}

	TestTrue(TEXT("Celerity level 5 should unlock the Y-menu SXP source multiplier."), Level5Component->ApplyFreeAttributeLevels(EProjectSinAttribute::Celerity, 5));
	const int32 ExpectedLevel5Grant = FMath::RoundToInt(100.f * YMenuMultiplier) + (5 * FlatSxpPerLevel);
	TestEqual(
		TEXT("Celerity level 5 should multiply Y-menu action SXP before adding the flat passive."),
		Level5Component->AutomationGrantYMenuActionSxp(TEXT("YMenu.Automation"), 100, false),
		ExpectedLevel5Grant);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionCelerityDeathRecoveryTest,
	"ACFUltimateSample.SinfulAscension.Celerity.DeathRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionCelerityDeathRecoveryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float LossRatio = Settings ? FMath::Clamp(Settings->DeathRunLossRatio, 0.f, 1.f) : 0.90f;
	const int32 SeedRunSxp = 1000;
	const int32 ExpectedNormalSavedSxp = FMath::Max(0, FMath::RoundToInt(static_cast<float>(SeedRunSxp) * (1.f - LossRatio)));

	UProjectSinfulAscensionComponent* Level9Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Level 9 component should be constructible"), Level9Component);
	if (!Level9Component)
	{
		return false;
	}

	TestEqual(TEXT("Seed SXP should be granted before Celerity passive levels are applied."), Level9Component->GrantSxp(TEXT("Death.Seed"), SeedRunSxp, false), SeedRunSxp);
	TestTrue(TEXT("Celerity level 9 should remain below the death recovery milestone."), Level9Component->ApplyFreeAttributeLevels(EProjectSinAttribute::Celerity, 9));
	Level9Component->HandleRunDeath();
	TestEqual(TEXT("Celerity level 9 should preserve the existing death save behavior."), Level9Component->GetMetaBankSxp(), ExpectedNormalSavedSxp);
	TestEqual(TEXT("Death should clear run SXP at Celerity level 9."), Level9Component->GetCurrentRunSxp(), 0);

	UProjectSinfulAscensionComponent* Level10Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Level 10 component should be constructible"), Level10Component);
	if (!Level10Component)
	{
		return false;
	}

	TestEqual(TEXT("Seed SXP should be granted before Celerity level 10 is applied."), Level10Component->GrantSxp(TEXT("Death.Seed"), SeedRunSxp, false), SeedRunSxp);
	TestTrue(TEXT("Celerity level 10 should unlock recovered run SXP on death."), Level10Component->ApplyFreeAttributeLevels(EProjectSinAttribute::Celerity, 10));
	Level10Component->HandleRunDeath();
	TestEqual(TEXT("Celerity level 10 should recover the SXP that death would have lost."), Level10Component->GetMetaBankSxp(), SeedRunSxp);
	TestEqual(TEXT("Death should clear run SXP at Celerity level 10."), Level10Component->GetCurrentRunSxp(), 0);

	UProjectSinfulAscensionComponent* SnapshotComponent = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Snapshot component should be constructible"), SnapshotComponent);
	if (!SnapshotComponent)
	{
		return false;
	}

	TestTrue(TEXT("Celerity should still be levelable for milestone snapshots."), SnapshotComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Celerity, 10));
	const FProjectSinfulAscensionSnapshot Snapshot = SnapshotComponent->BuildSnapshot();
	const FProjectSinAttributeState* CelerityState = Snapshot.Attributes.FindByPredicate([](const FProjectSinAttributeState& State)
	{
		return State.Attribute == EProjectSinAttribute::Celerity;
	});

	TestNotNull(TEXT("Celerity should appear in the attribute snapshot."), CelerityState);
	if (!CelerityState)
	{
		return false;
	}

	TestTrue(TEXT("Celerity should expose the level 5 Y-menu milestone."), CelerityState->bMilestone5Unlocked);
	TestTrue(TEXT("Celerity should expose the level 10 death recovery milestone."), CelerityState->bMilestone10Unlocked);
	TestTrue(TEXT("Celerity should expose the Y-menu milestone snapshot."), Snapshot.Milestones.ContainsByPredicate([](const FProjectSinMilestoneState& State)
	{
		return State.Attribute == EProjectSinAttribute::Celerity && State.AbilityId == FName(TEXT("YMenuSurge")) && State.RequiredLevel == 5;
	}));
	TestTrue(TEXT("Celerity should expose the death recovery milestone snapshot."), Snapshot.Milestones.ContainsByPredicate([](const FProjectSinMilestoneState& State)
	{
		return State.Attribute == EProjectSinAttribute::Celerity && State.AbilityId == FName(TEXT("RecoveredMomentum")) && State.RequiredLevel == 10;
	}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionWillpowerEnduranceMaxTableTest,
	"ACFUltimateSample.SinfulAscension.Willpower.EnduranceMaxTable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionWillpowerEnduranceMaxTableTest::RunTest(const FString& Parameters)
{
	const int32 ExpectedMaxByLevel[] = { 100, 101, 103, 104, 106, 107, 109, 110, 112, 113, 115 };
	for (int32 Level = 0; Level <= 10; ++Level)
	{
		float FlatBonusTotal = 0.f;
		float PercentMultiplier = 1.f;
		FProjectSinfulAscensionDynamicMaxRule WillpowerRule(TEXT("Hunger"), false, EProjectSinAttribute::Willpower, 1.5f, 0.f, true);
		UProjectSinfulAscensionSettings::AccumulateDynamicMaxRule(WillpowerRule, Level, FlatBonusTotal, PercentMultiplier);
		TestEqual(
			FString::Printf(TEXT("Willpower level %d should resolve the expected endurance max"), Level),
			FMath::RoundToInt(UProjectSinfulAscensionSettings::ComputeDynamicMaxValue(100.f, FlatBonusTotal, PercentMultiplier)),
			ExpectedMaxByLevel[Level]);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionWillpowerLustMaxStackTest,
	"ACFUltimateSample.SinfulAscension.Willpower.LustMaxStack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionWillpowerLustMaxStackTest::RunTest(const FString& Parameters)
{
	float FlatBonusTotal = 0.f;
	float PercentMultiplier = 1.f;
	FProjectSinfulAscensionDynamicMaxRule WillpowerRule(TEXT("Lust"), true, EProjectSinAttribute::Willpower, 1.5f, 0.f, true);
	FProjectSinfulAscensionDynamicMaxRule AllureRule(TEXT("Lust"), true, EProjectSinAttribute::Allure, 100.f, 0.f);

	UProjectSinfulAscensionSettings::AccumulateDynamicMaxRule(WillpowerRule, 10, FlatBonusTotal, PercentMultiplier);
	TestTrue(TEXT("Lust should include the Willpower endurance bonus"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeDynamicMaxValue(10000.f, FlatBonusTotal, PercentMultiplier), 10015.f, 0.0001f));

	UProjectSinfulAscensionSettings::AccumulateDynamicMaxRule(AllureRule, 5, FlatBonusTotal, PercentMultiplier);
	TestTrue(TEXT("Lust should stack Willpower endurance and Allure maximum bonuses"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeDynamicMaxValue(10000.f, FlatBonusTotal, PercentMultiplier), 10515.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionWillpowerSecondBreathTest,
	"ACFUltimateSample.SinfulAscension.Willpower.SecondBreath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionWillpowerSecondBreathTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UProjectSurvivalNeedsComponent* NeedsComponent = NewObject<UProjectSurvivalNeedsComponent>(Owner);
	UProjectSinfulAscensionComponent* SinComponent = NewObject<UProjectSinfulAscensionComponent>(Owner);
	TestNotNull(TEXT("Owner should be constructible"), Owner);
	TestNotNull(TEXT("Needs component should be constructible"), NeedsComponent);
	TestNotNull(TEXT("Sin component should be constructible"), SinComponent);
	if (!Owner || !NeedsComponent || !SinComponent)
	{
		return false;
	}

	Owner->AddInstanceComponent(NeedsComponent);
	Owner->AddInstanceComponent(SinComponent);

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Willpower, 4);
	NeedsComponent->SetNeedCurrentValue(TEXT("Hunger"), 50.f, false);
	NeedsComponent->SetSensationCurrentValue(TEXT("Pain"), 50.f, false);
	TestFalse(TEXT("Second Breath should not trigger below Willpower milestone 5"),
		SinComponent->NotifyDungeonFloorCompleted(TEXT("FloorBelowMilestone")));
	TestTrue(TEXT("Hunger should not recover below milestone 5"),
		FMath::IsNearlyEqual(NeedsComponent->GetNeedCurrentValue(TEXT("Hunger")), 50.f, 0.0001f));
	TestTrue(TEXT("Pain should not recover below milestone 5"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Pain")), 50.f, 0.0001f));

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Willpower, 6);
	NeedsComponent->SetNeedCurrentValue(TEXT("Hunger"), 50.f, false);
	NeedsComponent->SetNeedCurrentValue(TEXT("Thirst"), 50.f, false);
	NeedsComponent->SetNeedCurrentValue(TEXT("Sleep"), 50.f, false);
	NeedsComponent->SetSensationCurrentValue(TEXT("Pain"), 50.f, false);
	NeedsComponent->SetSensationCurrentValue(TEXT("Madness"), 50.f, false);
	TestTrue(TEXT("Second Breath should trigger at Willpower 10"),
		SinComponent->NotifyDungeonFloorCompleted(TEXT("FloorA")));
	TestTrue(TEXT("Willpower 10 should set Hunger max to 115"),
		FMath::IsNearlyEqual(NeedsComponent->GetNeedMaxValue(TEXT("Hunger")), 115.f, 0.0001f));
	TestTrue(TEXT("Second Breath should restore 23 Hunger at Willpower 10"),
		FMath::IsNearlyEqual(NeedsComponent->GetNeedCurrentValue(TEXT("Hunger")), 73.f, 0.0001f));
	TestTrue(TEXT("Second Breath should reduce Pain by 23 at Willpower 10"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Pain")), 27.f, 0.0001f));
	TestTrue(TEXT("Second Breath should reduce Madness by 23 at Willpower 10"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Madness")), 27.f, 0.0001f));

	const float HungerAfterFirstFloor = NeedsComponent->GetNeedCurrentValue(TEXT("Hunger"));
	TestFalse(TEXT("Second Breath should not duplicate for the same floor transition id"),
		SinComponent->NotifyDungeonFloorCompleted(TEXT("FloorA")));
	TestTrue(TEXT("Duplicate floor id should not change Hunger again"),
		FMath::IsNearlyEqual(NeedsComponent->GetNeedCurrentValue(TEXT("Hunger")), HungerAfterFirstFloor, 0.0001f));

	NeedsComponent->SetNeedCurrentValue(TEXT("Hunger"), 114.f, false);
	NeedsComponent->SetSensationCurrentValue(TEXT("Pain"), 10.f, false);
	NeedsComponent->SetSensationCurrentValue(TEXT("Lust"), 1000.f, false);
	TestTrue(TEXT("Second Breath should process a new floor id"),
		SinComponent->NotifyDungeonFloorCompleted(TEXT("FloorB")));
	TestTrue(TEXT("Second Breath should clamp needs to max"),
		FMath::IsNearlyEqual(NeedsComponent->GetNeedCurrentValue(TEXT("Hunger")), 115.f, 0.0001f));
	TestTrue(TEXT("Second Breath should clamp Pain to zero"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Pain")), 0.f, 0.0001f));
	TestTrue(TEXT("Second Breath should clamp Lust to zero"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Lust")), 0.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionWillpowerUnbrokenMindTest,
	"ACFUltimateSample.SinfulAscension.Willpower.UnbrokenMind",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionWillpowerUnbrokenMindTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UProjectSurvivalStatusComponent* StatusComponent = NewObject<UProjectSurvivalStatusComponent>(Owner);
	UProjectSinfulAscensionComponent* SinComponent = NewObject<UProjectSinfulAscensionComponent>(Owner);
	TestNotNull(TEXT("Owner should be constructible"), Owner);
	TestNotNull(TEXT("Status component should be constructible"), StatusComponent);
	TestNotNull(TEXT("Sin component should be constructible"), SinComponent);
	if (!Owner || !StatusComponent || !SinComponent)
	{
		return false;
	}

	Owner->AddInstanceComponent(StatusComponent);
	Owner->AddInstanceComponent(SinComponent);
	StatusComponent->ForceRefresh();

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Willpower, 9);
	TestTrue(TEXT("Fear should apply before Unbroken Mind"),
		StatusComponent->ApplyStatus(TEXT("Fear"), 5.f, Owner));
	TestTrue(TEXT("Fear should be active before Unbroken Mind"),
		StatusComponent->IsStatusActive(TEXT("Fear")));

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Willpower, 1);
	TestTrue(TEXT("Fear should become immune at Willpower 10"),
		StatusComponent->IsStatusImmune(TEXT("Fear")));
	TestTrue(TEXT("Dizzy should stand in for Confusion and become immune at Willpower 10"),
		StatusComponent->IsStatusImmune(TEXT("Dizzy")));
	TestFalse(TEXT("Unbroken Mind should clear active Fear"),
		StatusComponent->IsStatusActive(TEXT("Fear")));
	TestFalse(TEXT("Unbroken Mind should block future Fear"),
		StatusComponent->ApplyStatus(TEXT("Fear"), 5.f, Owner));
	TestFalse(TEXT("Unbroken Mind should block future Dizzy"),
		StatusComponent->ApplyStatus(TEXT("Dizzy"), 5.f, Owner));

	FProjectIncomingHitContext LethalHit;
	LethalHit.AppliedDamage = 100.f;
	float SurvivingHealth = 0.f;
	TestFalse(TEXT("Willpower should no longer prevent lethal damage"),
		SinComponent->TryPreventDeathFromDamage(LethalHit, 50.f, SurvivingHealth));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionMasochismFlatDamageNegationTest,
	"ACFUltimateSample.SinfulAscension.Masochism.FlatDamageNegation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionMasochismFlatDamageNegationTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Masochism level 0 should not negate flat damage"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeMasochismFlatDamageNegation(0, 4.f), 0.f, 0.0001f));
	TestTrue(TEXT("Masochism level 1 should negate 4 flat damage"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeMasochismFlatDamageNegation(1, 4.f), 4.f, 0.0001f));
	TestTrue(TEXT("Masochism level 5 should negate 20 flat damage"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeMasochismFlatDamageNegation(5, 4.f), 20.f, 0.0001f));
	TestTrue(TEXT("Masochism level 10 should negate 40 flat damage"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeMasochismFlatDamageNegation(10, 4.f), 40.f, 0.0001f));
	TestTrue(TEXT("Flat negation should clamp post-armor damage to zero"),
		FMath::IsNearlyEqual(FMath::Max(0.f, 35.f - UProjectSinfulAscensionSettings::ComputeMasochismFlatDamageNegation(10, 4.f)), 0.f, 0.0001f));
	TestTrue(TEXT("Flat negation should leave remaining post-armor damage when damage exceeds the flat amount"),
		FMath::IsNearlyEqual(FMath::Max(0.f, 60.f - UProjectSinfulAscensionSettings::ComputeMasochismFlatDamageNegation(10, 4.f)), 20.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionMasochismPainOverflowAndReflectionMathTest,
	"ACFUltimateSample.SinfulAscension.Masochism.PainOverflowAndReflectionMath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionMasochismPainOverflowAndReflectionMathTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Pain Overflow should grant 25 percent of a 10000 Lust max"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeMasochismPainOverflowLust(10000.f, 0.25f), 2500.f, 0.0001f));
	TestTrue(TEXT("Pain Overflow should use the live dynamic Lust max"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeMasochismPainOverflowLust(10515.f, 0.25f), 2628.75f, 0.0001f));
	TestTrue(TEXT("Pain reflection should convert applied Pain to five times as much Lust by default"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeMasochismPainReflectionLust(10.f, 5.f), 50.f, 0.0001f));
	TestTrue(TEXT("Pain reflection should scale from the actual applied Pain delta"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeMasochismPainReflectionLust(5.f, 5.f), 25.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionMasochismPainRaptureAbsorbTest,
	"ACFUltimateSample.SinfulAscension.Masochism.PainRaptureAbsorb",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionMasochismPainRaptureAbsorbTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UProjectSurvivalNeedsComponent* NeedsComponent = NewObject<UProjectSurvivalNeedsComponent>(Owner);
	UProjectSurvivalStatusComponent* StatusComponent = NewObject<UProjectSurvivalStatusComponent>(Owner);
	UProjectCombatAttributeComponent* CombatComponent = NewObject<UProjectCombatAttributeComponent>(Owner);
	UProjectSinfulAscensionComponent* SinComponent = NewObject<UProjectSinfulAscensionComponent>(Owner);
	TestNotNull(TEXT("Owner should be constructible"), Owner);
	TestNotNull(TEXT("Needs component should be constructible"), NeedsComponent);
	TestNotNull(TEXT("Status component should be constructible"), StatusComponent);
	TestNotNull(TEXT("Combat component should be constructible"), CombatComponent);
	TestNotNull(TEXT("Sin component should be constructible"), SinComponent);
	if (!Owner || !NeedsComponent || !StatusComponent || !CombatComponent || !SinComponent)
	{
		return false;
	}

	Owner->AddInstanceComponent(NeedsComponent);
	Owner->AddInstanceComponent(StatusComponent);
	Owner->AddInstanceComponent(CombatComponent);
	Owner->AddInstanceComponent(SinComponent);
	StatusComponent->ForceRefresh();

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Masochism, 5);
	NeedsComponent->SetSensationCurrentValue(TEXT("Pain"), NeedsComponent->GetSensationMaxValue(TEXT("Pain")), true);
	NeedsComponent->SetSensationCurrentValue(TEXT("Lust"), NeedsComponent->GetSensationMaxValue(TEXT("Lust")), true);
	TestTrue(TEXT("Pain Rapture should defer pain knockout when Pain and Lust are both maxed"),
		SinComponent->TryDeferPainKnockout());
	TestTrue(TEXT("Pain Rapture should become active"),
		SinComponent->IsMasochismPainRaptureActive());
	TestEqual(TEXT("Pain Rapture should grant the temporary flat activation SXP once"), SinComponent->GetCurrentRunSxp(), 2000);

	float FlatNegatedDamage = 0.f;
	float AbsorbedDamage = 0.f;
	const float FinalDamage = SinComponent->ModifyIncomingHealthDamage(
		Owner,
		TEXT("Physical"),
		30.f,
		FlatNegatedDamage,
		AbsorbedDamage);
	TestTrue(TEXT("Masochism 5 should negate 20 damage before rapture absorption"),
		FMath::IsNearlyEqual(FlatNegatedDamage, 20.f, 0.0001f));
	TestTrue(TEXT("Pain Rapture should absorb the remaining post-mitigation damage"),
		FMath::IsNearlyEqual(AbsorbedDamage, 10.f, 0.0001f));
	TestTrue(TEXT("Pain Rapture should reduce final incoming health damage to zero"),
		FMath::IsNearlyEqual(FinalDamage, 0.f, 0.0001f));
	TestEqual(TEXT("Absorbed damage should not add extra per-hit SXP"), SinComponent->GetCurrentRunSxp(), 2000);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionMasochismPainRaptureAcfImmortalityTest,
	"ACFUltimateSample.SinfulAscension.Masochism.PainRaptureAcfImmortality",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionMasochismPainRaptureAcfImmortalityTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UProjectSurvivalNeedsComponent* NeedsComponent = NewObject<UProjectSurvivalNeedsComponent>(Owner);
	UProjectSurvivalStatusComponent* StatusComponent = NewObject<UProjectSurvivalStatusComponent>(Owner);
	UACFDamageHandlerComponent* DamageHandlerComponent = NewObject<UACFDamageHandlerComponent>(Owner);
	UProjectSinfulAscensionComponent* SinComponent = NewObject<UProjectSinfulAscensionComponent>(Owner);
	TestNotNull(TEXT("Owner should be constructible"), Owner);
	TestNotNull(TEXT("Needs component should be constructible"), NeedsComponent);
	TestNotNull(TEXT("Status component should be constructible"), StatusComponent);
	TestNotNull(TEXT("ACF damage handler should be constructible"), DamageHandlerComponent);
	TestNotNull(TEXT("Sin component should be constructible"), SinComponent);
	if (!Owner || !NeedsComponent || !StatusComponent || !DamageHandlerComponent || !SinComponent)
	{
		return false;
	}

	Owner->AddInstanceComponent(NeedsComponent);
	Owner->AddInstanceComponent(StatusComponent);
	Owner->AddInstanceComponent(DamageHandlerComponent);
	Owner->AddInstanceComponent(SinComponent);
	StatusComponent->ForceRefresh();

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Masochism, 5);
	NeedsComponent->SetSensationCurrentValue(TEXT("Pain"), NeedsComponent->GetSensationMaxValue(TEXT("Pain")), true);
	NeedsComponent->SetSensationCurrentValue(TEXT("Lust"), NeedsComponent->GetSensationMaxValue(TEXT("Lust")), true);
	TestFalse(TEXT("ACF damage handler should start mortal in the test"),
		DamageHandlerComponent->GetIsImmortal());
	TestTrue(TEXT("Pain Rapture should activate"),
		SinComponent->TryDeferPainKnockout());
	TestTrue(TEXT("Pain Rapture should enable ACF immortality"),
		DamageHandlerComponent->GetIsImmortal());

	SinComponent->HandleRunDeath();
	TestFalse(TEXT("Pain Rapture cleanup should restore the previous ACF immortality state"),
		DamageHandlerComponent->GetIsImmortal());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionMasochismPainOverflowRuntimeTest,
	"ACFUltimateSample.SinfulAscension.Masochism.PainOverflowRuntime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionMasochismPainOverflowRuntimeTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UProjectSurvivalNeedsComponent* NeedsComponent = NewObject<UProjectSurvivalNeedsComponent>(Owner);
	UProjectSurvivalStatusComponent* StatusComponent = NewObject<UProjectSurvivalStatusComponent>(Owner);
	UProjectSinfulAscensionComponent* SinComponent = NewObject<UProjectSinfulAscensionComponent>(Owner);
	TestNotNull(TEXT("Owner should be constructible"), Owner);
	TestNotNull(TEXT("Needs component should be constructible"), NeedsComponent);
	TestNotNull(TEXT("Status component should be constructible"), StatusComponent);
	TestNotNull(TEXT("Sin component should be constructible"), SinComponent);
	if (!Owner || !NeedsComponent || !StatusComponent || !SinComponent)
	{
		return false;
	}

	Owner->AddInstanceComponent(NeedsComponent);
	Owner->AddInstanceComponent(StatusComponent);
	Owner->AddInstanceComponent(SinComponent);
	StatusComponent->ForceRefresh();

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Masochism, 10);
	NeedsComponent->SetSensationCurrentValue(TEXT("Lust"), 0.f, true);
	NeedsComponent->SetSensationCurrentValue(TEXT("Pain"), NeedsComponent->GetSensationMaxValue(TEXT("Pain")), true);

	TestTrue(TEXT("Masochism 10 should defer Pain knockout through Pain Overflow"),
		SinComponent->TryDeferPainKnockout());
	TestFalse(TEXT("Masochism 10 should not enter Pain Rapture when resolving Pain Overflow"),
		SinComponent->IsMasochismPainRaptureActive());
	TestTrue(TEXT("Pain Overflow should reset Pain to zero"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Pain")), 0.f, 0.0001f));
	TestTrue(TEXT("Pain Overflow should grant 2500 Lust at a 10000 Lust max"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Lust")), 2500.f, 0.0001f));

	NeedsComponent->SetSensationMaxValue(TEXT("Lust"), 10515.f, true);
	NeedsComponent->SetSensationCurrentValue(TEXT("Lust"), 0.f, true);
	NeedsComponent->SetSensationCurrentValue(TEXT("Pain"), NeedsComponent->GetSensationMaxValue(TEXT("Pain")), true);

	TestTrue(TEXT("Pain Overflow should still defer with a dynamic Lust max"),
		SinComponent->TryDeferPainKnockout());
	TestTrue(TEXT("Pain Overflow should use the live 10515 Lust max"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Lust")), 2628.75f, 0.0001f));
	TestTrue(TEXT("Pain should reset again after dynamic-max overflow"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Pain")), 0.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionMasochismPainReflectionAccumulationTest,
	"ACFUltimateSample.SinfulAscension.Masochism.PainReflectionAccumulation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionMasochismPainReflectionAccumulationTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UProjectSurvivalNeedsComponent* NeedsComponent = NewObject<UProjectSurvivalNeedsComponent>(Owner);
	UProjectSinfulAscensionComponent* SinComponent = NewObject<UProjectSinfulAscensionComponent>(Owner);
	TestNotNull(TEXT("Owner should be constructible"), Owner);
	TestNotNull(TEXT("Needs component should be constructible"), NeedsComponent);
	TestNotNull(TEXT("Sin component should be constructible"), SinComponent);
	if (!Owner || !NeedsComponent || !SinComponent)
	{
		return false;
	}

	Owner->AddInstanceComponent(NeedsComponent);
	Owner->AddInstanceComponent(SinComponent);

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Masochism, 10);
	NeedsComponent->SetSensationCurrentValue(TEXT("Lust"), 0.f, true);

	SinComponent->AutomationQueueMasochismPainReflection(4.f);
	SinComponent->AutomationQueueMasochismPainReflection(6.f);
	SinComponent->AutomationQueueMasochismPainReflection(3.f);

	TestTrue(TEXT("Pain reflection should accumulate all pending pain before the delayed flush"),
		FMath::IsNearlyEqual(SinComponent->AutomationGetPendingMasochismPainReflectionLust(), 65.f, 0.0001f));
	TestTrue(TEXT("Pain reflection should not apply Lust before the delayed flush"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Lust")), 0.f, 0.0001f));

	SinComponent->AutomationFlushMasochismPainReflection();
	TestTrue(TEXT("Pain reflection flush should clear the pending bucket"),
		FMath::IsNearlyEqual(SinComponent->AutomationGetPendingMasochismPainReflectionLust(), 0.f, 0.0001f));
	TestTrue(TEXT("Pain reflection should grant the accumulated Lust in one flush"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Lust")), 65.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionFaithMathTest,
	"ACFUltimateSample.SinfulAscension.Faith.Math",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionFaithMathTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Faith should add flat spell defense by level"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeFaithPassiveBonus(10, 1.f), 10.f, 0.0001f));
	TestTrue(TEXT("Faith should support decimal spell defense tuning"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeFaithPassiveBonus(10, 0.5f), 5.f, 0.0001f));
	TestTrue(TEXT("Quiet Mind should recover Madness over elapsed seconds"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeFaithMadnessRecovery(2.f, 0.5f), 1.f, 0.0001f));
	TestTrue(TEXT("Sanctified Rest should restore from the live Madness max"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeFaithSleepMadnessRestore(115.f, 0.5f), 57.5f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionFaithPassiveAndMilestoneRuntimeTest,
	"ACFUltimateSample.SinfulAscension.Faith.PassiveAndMilestones",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionFaithPassiveAndMilestoneRuntimeTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UProjectCombatAttributeComponent* CombatComponent = NewObject<UProjectCombatAttributeComponent>(Owner);
	UProjectSurvivalNeedsComponent* NeedsComponent = NewObject<UProjectSurvivalNeedsComponent>(Owner);
	UProjectSinfulAscensionComponent* SinComponent = NewObject<UProjectSinfulAscensionComponent>(Owner);
	TestNotNull(TEXT("Owner should be constructible"), Owner);
	TestNotNull(TEXT("Combat component should be constructible"), CombatComponent);
	TestNotNull(TEXT("Needs component should be constructible"), NeedsComponent);
	TestNotNull(TEXT("Sin component should be constructible"), SinComponent);
	if (!Owner || !CombatComponent || !NeedsComponent || !SinComponent)
	{
		return false;
	}

	Owner->AddInstanceComponent(CombatComponent);
	Owner->AddInstanceComponent(NeedsComponent);
	Owner->AddInstanceComponent(SinComponent);

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Faith, 5);
	TestTrue(TEXT("Faith 5 should add 5 SpellDefense by default"),
		FMath::IsNearlyEqual(CombatComponent->GetAttributeCurrentValue(TEXT("SpellDefense")), 105.f, 0.0001f));
	TestTrue(TEXT("Faith 5 should expand the SpellDefense max by the same amount"),
		FMath::IsNearlyEqual(CombatComponent->GetAttributeMaxValue(TEXT("SpellDefense")), 105.f, 0.0001f));

	FProjectCombatDamageSpec SpellSpec;
	SpellSpec.DamageType = TEXT("Spell");
	SinComponent->ModifyOutgoingDamageSpec(Owner, SpellSpec);
	TestTrue(TEXT("Faith 5 should still add +10 flat spell damage"),
		FMath::IsNearlyEqual(SpellSpec.FlatBonusDamage, 10.f, 0.0001f));

	NeedsComponent->SetSensationCurrentValue(TEXT("Madness"), 10.f, true);
	SinComponent->AutomationApplyFaithMadnessRecovery(2.f);
	TestTrue(TEXT("Quiet Mind should reduce Madness by 1 over 2 seconds at Faith 5"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Madness")), 9.f, 0.0001f));

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Willpower, 10);
	TestTrue(TEXT("Willpower should raise the live Madness max used by Sanctified Rest"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationMaxValue(TEXT("Madness")), 115.f, 0.0001f));
	NeedsComponent->SetSensationCurrentValue(TEXT("Madness"), 80.f, true);
	TestFalse(TEXT("Sanctified Rest should stay locked below Faith 10"),
		SinComponent->NotifySleepCompleted(TEXT("Automation.SleepBelowMilestone")));
	TestTrue(TEXT("Madness should not change before Sanctified Rest unlocks"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Madness")), 80.f, 0.0001f));

	SinComponent->ApplyFreeAttributeLevels(EProjectSinAttribute::Faith, 5);
	TestTrue(TEXT("Sanctified Rest should trigger at Faith 10"),
		SinComponent->NotifySleepCompleted(TEXT("Automation.Sleep")));
	TestTrue(TEXT("Sanctified Rest should restore 50 percent of the live Madness max"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Madness")), 22.5f, 0.0001f));

	NeedsComponent->SetSensationCurrentValue(TEXT("Madness"), 20.f, true);
	TestTrue(TEXT("Sanctified Rest should trigger when there is any Madness to restore"),
		SinComponent->NotifySleepCompleted(TEXT("Automation.SleepClamp")));
	TestTrue(TEXT("Sanctified Rest should clamp Madness to zero"),
		FMath::IsNearlyEqual(NeedsComponent->GetSensationCurrentValue(TEXT("Madness")), 0.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionDamageScaledRewardTest,
	"ACFUltimateSample.SinfulAscension.General.DamageScaledRewardCaps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionDamageScaledRewardTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Sadism damage-to-lust should scale linearly under the cap"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeScaledResourceGain(120.f, 0.25f, 250.f), 30.f, 0.0001f));
	TestTrue(TEXT("Sadism damage-to-lust should clamp to the per-hit cap"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeScaledResourceGain(2000.f, 0.25f, 250.f), 250.f, 0.0001f));
	TestTrue(TEXT("Generic damage-scaled resource gain should scale linearly under the cap"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeScaledResourceGain(80.f, 0.10f, 12.f), 8.f, 0.0001f));
	TestTrue(TEXT("Generic damage-scaled resource gain should clamp to the per-hit cap"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeScaledResourceGain(500.f, 0.10f, 12.f), 12.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectDefeatPainFromAppliedDamageTest,
	"ACFUltimateSample.Defeat.Pain.AppliedDamageScalar",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectDefeatPainFromAppliedDamageTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("30 applied damage should become 0.3 pain at the default 1 percent scalar"),
		FMath::IsNearlyEqual(UProjectDefeatFlowSettings::ComputePainFromAppliedDamage(30.f, 0.01f), 0.3f, 0.0001f));
	TestTrue(TEXT("Zero applied damage should not add pain"),
		FMath::IsNearlyEqual(UProjectDefeatFlowSettings::ComputePainFromAppliedDamage(0.f, 0.01f), 0.f, 0.0001f));
	TestTrue(TEXT("Disabled scalar should not add pain"),
		FMath::IsNearlyEqual(UProjectDefeatFlowSettings::ComputePainFromAppliedDamage(30.f, 0.f), 0.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionDynamicMaxFormulaTest,
	"ACFUltimateSample.SinfulAscension.DynamicMaximums.Formula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionDynamicMaxFormulaTest::RunTest(const FString& Parameters)
{
	float FlatBonusTotal = 0.f;
	float PercentMultiplier = 1.f;

	FProjectSinfulAscensionDynamicMaxRule FlatRule(TEXT("Pain"), true, EProjectSinAttribute::Masochism, 0.5f, 0.f);
	FProjectSinfulAscensionDynamicMaxRule PercentRule(TEXT("Pain"), true, EProjectSinAttribute::Masochism, 0.f, 0.01f);
	UProjectSinfulAscensionSettings::AccumulateDynamicMaxRule(FlatRule, 10, FlatBonusTotal, PercentMultiplier);
	UProjectSinfulAscensionSettings::AccumulateDynamicMaxRule(PercentRule, 10, FlatBonusTotal, PercentMultiplier);

	TestTrue(TEXT("Mixed flat and percentage dynamic maximums should apply flats before percentage multipliers"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeDynamicMaxValue(100.f, FlatBonusTotal, PercentMultiplier), 115.5f, 0.0001f));

	FlatBonusTotal = 0.f;
	PercentMultiplier = 1.f;
	FProjectSinfulAscensionDynamicMaxRule WillpowerRule(TEXT("Pain"), true, EProjectSinAttribute::Willpower, 1.5f, 0.f, true);
	FProjectSinfulAscensionDynamicMaxRule AllureRule(TEXT("Lust"), true, EProjectSinAttribute::Allure, 100.f, 0.f);
	UProjectSinfulAscensionSettings::AccumulateDynamicMaxRule(WillpowerRule, 10, FlatBonusTotal, PercentMultiplier);
	TestTrue(TEXT("Willpower level 10 should add floor(level * 1.5) max points"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeDynamicMaxValue(100.f, FlatBonusTotal, PercentMultiplier), 115.f, 0.0001f));

	FlatBonusTotal = 0.f;
	PercentMultiplier = 1.f;
	UProjectSinfulAscensionSettings::AccumulateDynamicMaxRule(AllureRule, 5, FlatBonusTotal, PercentMultiplier);
	TestTrue(TEXT("Allure level 5 should add 500 lust max points"),
		FMath::IsNearlyEqual(UProjectSinfulAscensionSettings::ComputeDynamicMaxValue(10000.f, FlatBonusTotal, PercentMultiplier), 10500.f, 0.0001f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionFreeAttributeLevelsTest,
	"ACFUltimateSample.SinfulAscension.Background.FreeAttributeLevels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionFreeAttributeLevelsTest::RunTest(const FString& Parameters)
{
	UProjectSinfulAscensionComponent* Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Component should be constructible"), Component);
	if (!Component)
	{
		return false;
	}

	Component->GrantSxp(TEXT("AutomationTest"), 250, false);
	const int32 RunSxpBefore = Component->GetCurrentRunSxp();
	const int32 MetaBefore = Component->GetMetaBankSxp();
	TestTrue(TEXT("Free levels should apply"), Component->ApplyFreeAttributeLevels(EProjectSinAttribute::Cunning, 3));
	TestEqual(TEXT("Cunning should increase for free"), Component->GetAttributeLevel(EProjectSinAttribute::Cunning), 3);
	TestEqual(TEXT("Run SXP should not be spent by free levels"), Component->GetCurrentRunSxp(), RunSxpBefore);
	TestEqual(TEXT("Meta SXP should not be touched by free levels"), Component->GetMetaBankSxp(), MetaBefore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionFreeAttributeLevelClampTest,
	"ACFUltimateSample.SinfulAscension.Background.FreeAttributeLevelClamp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionFreeAttributeLevelClampTest::RunTest(const FString& Parameters)
{
	UProjectSinfulAscensionComponent* Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Component should be constructible"), Component);
	if (!Component)
	{
		return false;
	}

	Component->ApplyFreeAttributeLevels(EProjectSinAttribute::Willpower, 150);
	TestEqual(TEXT("Free levels should clamp at the configured default cap"), Component->GetAttributeLevel(EProjectSinAttribute::Willpower), 100);
	TestFalse(TEXT("Additional free levels at cap should be ignored"), Component->ApplyFreeAttributeLevels(EProjectSinAttribute::Willpower, 1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionBackgroundResetTest,
	"ACFUltimateSample.SinfulAscension.Background.ProfileChangeReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionBackgroundResetTest::RunTest(const FString& Parameters)
{
	UProjectSinfulAscensionComponent* Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Component should be constructible"), Component);
	if (!Component)
	{
		return false;
	}

	Component->GrantSxp(TEXT("AutomationTest"), 400, false);
	Component->ApplyFreeAttributeLevels(EProjectSinAttribute::Masochism, 3);
	Component->ResetRunProgressForBackgroundChange();
	TestEqual(TEXT("Profile change should reset current Run SXP to zero"), Component->GetCurrentRunSxp(), 0);
	TestEqual(TEXT("Profile change should reset run attribute levels"), Component->GetAttributeLevel(EProjectSinAttribute::Masochism), 0);
	TestEqual(TEXT("Profile change should preserve Meta SXP"), Component->GetMetaBankSxp(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectSinfulAscensionAffinityMultiplierTest,
	"ACFUltimateSample.SinfulAscension.Background.EventAffinityMultipliers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectSinfulAscensionAffinityMultiplierTest::RunTest(const FString& Parameters)
{
	UProjectSinfulAscensionComponent* Component = NewObject<UProjectSinfulAscensionComponent>();
	TestNotNull(TEXT("Component should be constructible"), Component);
	if (!Component)
	{
		return false;
	}

	TMap<EProjectSinAttribute, float> Multipliers;
	Multipliers.Add(EProjectSinAttribute::Sadism, 1.20f);
	Multipliers.Add(EProjectSinAttribute::Faith, 0.90f);
	Component->SetSXPGainMultipliers(Multipliers);

	TestEqual(TEXT("Sadism affinity should boost granted Run SXP"), Component->GrantSxpWithAttributeAffinity(TEXT("DamageDealt"), 100, false, { EProjectSinAttribute::Sadism }), 120);
	TestEqual(TEXT("Faith affinity should reduce granted Run SXP"), Component->GrantSxpWithAttributeAffinity(TEXT("SpellDamage"), 100, false, { EProjectSinAttribute::Faith }), 90);
	TestEqual(TEXT("No active affinity should use a neutral multiplier"), Component->GrantSxpWithAttributeAffinity(TEXT("Neutral"), 100, false, { EProjectSinAttribute::Cunning }), 100);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectCharacterBackgroundFallbackRowsTest,
	"ACFUltimateSample.CharacterBackground.Data.FallbackRows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectCharacterBackgroundFallbackRowsTest::RunTest(const FString& Parameters)
{
	UProjectCharacterBackgroundComponent* Component = NewObject<UProjectCharacterBackgroundComponent>();
	TestNotNull(TEXT("Background component should be constructible"), Component);
	if (!Component)
	{
		return false;
	}

	TestTrue(TEXT("Orphan backstory should resolve"), Component->SetBackstory(TEXT("Orphan")));
	TestTrue(TEXT("Criminal profession should resolve"), Component->SetProfession(TEXT("Criminal")));
	TestTrue(TEXT("Resolved selection should be valid"), Component->IsSelectionValid());
	TestEqual(TEXT("Orphan should provide two starting modifiers"), Component->GetFinalStartingLevelModifiers().Num(), 2);
	TestEqual(TEXT("Criminal should provide three gain modifiers"), Component->GetFinalGainModifiers().Num(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProjectCharacterBackgroundInvalidAttributeTest,
	"ACFUltimateSample.CharacterBackground.Data.InvalidAttributeId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FProjectCharacterBackgroundInvalidAttributeTest::RunTest(const FString& Parameters)
{
	EProjectSinAttribute Attribute = EProjectSinAttribute::Willpower;
	TestFalse(TEXT("Invalid AttributeID should fail resolution without crashing"), ProjectCharacterBackground::TryResolveSinAttribute(TEXT("InvalidAttribute"), Attribute));
	TestTrue(TEXT("Valid AttributeID should resolve"), ProjectCharacterBackground::TryResolveSinAttribute(TEXT("Allure"), Attribute));
	TestEqual(TEXT("Allure should resolve to the existing enum value"), static_cast<int32>(Attribute), static_cast<int32>(EProjectSinAttribute::Allure));
	return true;
}

#endif
