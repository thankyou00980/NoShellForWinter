#include "SinfulAscension/ProjectSinfulAscensionComponent.h"

#include "ACMCollisionManagerComponent.h"
#include "ACMEffectsDispatcherComponent.h"
#include "ACMTypes.h"
#include "ARSStatisticsComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Combat/ProjectCombatAttributeComponent.h"
#include "Combat/ProjectCombatTypes.h"
#include "Components/ACFDamageHandlerComponent.h"
#include "Components/ACFEffectsManagerComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DirtyPawn/ProjectDirtyPawnEffectsBridgeComponent.h"
#include "Defeat/ProjectDefeatFlowSettings.h"
#include "Defeat/ProjectDefeatFlowComponent.h"
#include "Defeat/ProjectDefeatHitResolver.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Game/ACFDamageType.h"
#include "Game/ACFFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Locomotion/ProjectEmoteComponent.h"
#include "Locomotion/ProjectLocomotionOverrideComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/BodyInstance.h"
#include "SinfulAscension/ProjectSinfulAscensionSaveGame.h"
#include "SinfulAscension/ProjectSinfulAscensionSettings.h"
#include "Sound/SoundBase.h"
#include "Survival/ProjectRealtimeSnapshotComponent.h"
#include "Survival/ProjectRealtimeSnapshotSettings.h"
#include "Survival/ProjectSurvivalNeedsComponent.h"
#include "Survival/ProjectSurvivalNeedsSettings.h"
#include "Survival/ProjectSurvivalStatusComponent.h"
#include "TimerManager.h"
#include "UI/ACFDamageWidget.h"

#define LOCTEXT_NAMESPACE "ProjectSinfulAscensionComponent"

namespace
{
	const FName MadnessName(TEXT("Madness"));
	const FName LustName(TEXT("Lust"));
	const FName PainName(TEXT("Pain"));
	const FName HungerName(TEXT("Hunger"));
	const FName ThirstName(TEXT("Thirst"));
	const FName SleepName(TEXT("Sleep"));
	const FName DanceInteractionId(TEXT("Actions.Dance"));
	const FName TiredStatusName(TEXT("Tired"));
	const FName ExhaustedStatusName(TEXT("Exhausted"));
	const FName ExhaustedRecoveryStatusName(TEXT("ExhaustedRecovery"));
	const FName FrenzyStatusName(TEXT("Frenzy"));
	const FName OrgasmRushStatusName(TEXT("OrgasmRush"));
	const FName ExtremePainStatusName(TEXT("ExtremePain"));
	const FName HealthResourceName(TEXT("Health"));
	const FName SpellDefenseName(TEXT("SpellDefense"));
	const FName WillpowerImmunitySourceId(TEXT("Willpower.UnbrokenMind"));
	const FName CelerityYMenuSurgeAbilityId(TEXT("YMenuSurge"));
	const FName CelerityRecoveredMomentumAbilityId(TEXT("RecoveredMomentum"));

	int32 ToAttributeIndex(const EProjectSinAttribute Attribute)
	{
		return static_cast<int32>(Attribute);
	}

	bool IsSpellLikeDamage(const FName DamageType)
	{
		const FString DamageTypeString = DamageType.ToString();
		return DamageTypeString.Contains(TEXT("Spell"), ESearchCase::IgnoreCase)
			|| DamageTypeString.Contains(TEXT("Magic"), ESearchCase::IgnoreCase);
	}

	bool IsMasochismQualifiedHitDamageType(const FName DamageType)
	{
		if (DamageType.IsNone())
		{
			return true;
		}

		if (IsSpellLikeDamage(DamageType))
		{
			return true;
		}

		const FString DamageTypeString = DamageType.ToString();
		return DamageTypeString.Contains(TEXT("Physical"), ESearchCase::IgnoreCase)
			|| DamageTypeString.Contains(TEXT("Melee"), ESearchCase::IgnoreCase)
			|| DamageTypeString.Contains(TEXT("Ranged"), ESearchCase::IgnoreCase)
			|| DamageTypeString.Contains(TEXT("Projectile"), ESearchCase::IgnoreCase)
			|| DamageTypeString.Contains(TEXT("Arrow"), ESearchCase::IgnoreCase);
	}

	TArray<FString> ResolveQualifiedEnemyHints()
	{
		if (const UProjectDefeatFlowSettings* DefeatSettings = UProjectDefeatFlowSettings::Get())
		{
			if (DefeatSettings->QualifiedEnemyClassNameHints.Num() > 0)
			{
				return DefeatSettings->QualifiedEnemyClassNameHints;
			}
		}

		if (const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get())
		{
			return Settings->QualifiedMaleEnemyClassNameHints;
		}

		return TArray<FString>();
	}

	TSubclassOf<UACFDamageType> LoadAcfFeedbackDamageClass(const TCHAR* ClassPath, const TSubclassOf<UACFDamageType> FallbackClass)
	{
		if (UClass* LoadedClass = LoadClass<UACFDamageType>(nullptr, ClassPath))
		{
			return LoadedClass;
		}

		return FallbackClass;
	}
}

UProjectSinfulAscensionComponent::UProjectSinfulAscensionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.f;
	InitializeAttributeStorage();
}

void UProjectSinfulAscensionComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeAttributeStorage();
	RefreshCachedComponents();
	EnsureDirtyPawnEffectsBridge();
	LoadPersistentState();
	ApplyPassiveAttributeEffects();
	BindToRealtimeSnapshotComponent();

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	LastSensationGainTimeSeconds.Add(MadnessName, Now);
	LastSensationGainTimeSeconds.Add(LustName, Now);
	LastSensationGainTimeSeconds.Add(PainName, Now);
	LastSensationDecayTimeSeconds.Add(MadnessName, Now);
	LastSensationDecayTimeSeconds.Add(LustName, Now);
	LastSensationDecayTimeSeconds.Add(PainName, Now);
}

void UProjectSinfulAscensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndMasochismPainRaptureState();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MasochismPainReflectionTimerHandle);
	}
	PendingMasochismPainReflectionLust = 0.f;
	UnbindFromRealtimeSnapshotComponent();

	if (StatusComponent)
	{
		StatusComponent->ClearStatusImmunitySource(WillpowerImmunitySourceId);
		StatusComponent->OnBlackoutChanged.RemoveDynamic(this, &ThisClass::HandleBlackoutChanged);
	}

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			if (bHasCachedBaseMaxWalkSpeed)
			{
				MovementComponent->MaxWalkSpeed = CachedBaseMaxWalkSpeed;
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UProjectSinfulAscensionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.f)
	{
		return;
	}

	if (!RealtimeSnapshotComponent)
	{
		BindToRealtimeSnapshotComponent();
	}

	UpdateCombatState();
	MaintainMasochismPainRapture();

	AccumulatedTickSeconds += DeltaTime;
	if (AccumulatedTickSeconds < 0.25f)
	{
		return;
	}

	const float StepSeconds = AccumulatedTickSeconds;
	AccumulatedTickSeconds = 0.f;

	RefreshCachedComponents();
	ApplyPassiveAttributeEffects();
	UpdateActiveEmoteEffects(StepSeconds);
	if (bIsNude)
	{
		NudityTickAccumulatorSeconds += StepSeconds;
		while (NudityTickAccumulatorSeconds >= 4.f)
		{
			NudityTickAccumulatorSeconds -= 4.f;
			GrantSensation(TEXT("Nudity.Tick"), 3.f, 25.f, 2.f);
		}
	}
	else
	{
		NudityTickAccumulatorSeconds = 0.f;
	}
	UpdateFaithMadnessRecovery(StepSeconds);
	UpdateSensationDecay();
	UpdateSensationConsequences(StepSeconds);
	UpdateMovementBonus();
}

int32 UProjectSinfulAscensionComponent::GetCurrentRunSxp() const
{
	return CurrentRunSxp;
}

int32 UProjectSinfulAscensionComponent::GetMetaBankSxp() const
{
	return MetaBankSxp;
}

bool UProjectSinfulAscensionComponent::IsEternalSinModeEnabled() const
{
	return bEternalSinMode;
}

int32 UProjectSinfulAscensionComponent::GetAttributeLevel(const EProjectSinAttribute Attribute) const
{
	const int32 Index = ToAttributeIndex(Attribute);
	return AttributeLevels.IsValidIndex(Index) ? AttributeLevels[Index] : 0;
}

int32 UProjectSinfulAscensionComponent::GetUpgradeCost(const EProjectSinAttribute Attribute) const
{
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const int32 MaxAttributeLevel = FMath::Max(Settings ? Settings->MaxSinAttributeLevel : 100, 1);
	if (GetAttributeLevel(Attribute) >= MaxAttributeLevel)
	{
		return 0;
	}

	return UProjectSinfulAscensionSettings::ComputeAttributeUpgradeCost(GetAttributeLevel(Attribute));
}

FProjectSinfulAscensionSnapshot UProjectSinfulAscensionComponent::BuildSnapshot() const
{
	FProjectSinfulAscensionSnapshot Snapshot;
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const int32 CelerityYMenuMilestoneLevel = FMath::Max(0, Settings ? Settings->CelerityYMenuActionSxpMilestoneLevel : 5);
	const int32 CelerityDeathMilestoneLevel = FMath::Max(0, Settings ? Settings->CelerityDeathLostSxpToMetaMilestoneLevel : 10);
	Snapshot.CurrentRunSxp = CurrentRunSxp;
	Snapshot.MetaBankSxp = MetaBankSxp;
	Snapshot.bEternalSinMode = bEternalSinMode;
	Snapshot.Madness = GetSensationCurrent(MadnessName);
	Snapshot.MadnessMax = FMath::Max(GetSensationMax(MadnessName), ResolveBaseSensationMax(MadnessName));
	Snapshot.Lust = GetSensationCurrent(LustName);
	Snapshot.LustMax = FMath::Max(GetSensationMax(LustName), ResolveBaseSensationMax(LustName));
	Snapshot.Pain = GetSensationCurrent(PainName);
	Snapshot.PainMax = FMath::Max(GetSensationMax(PainName), ResolveBaseSensationMax(PainName));
	Snapshot.SensoryOverloadProgressSeconds = SensoryOverloadProgressSeconds;
	Snapshot.bSensoryOverloadActive = bOverloadBlackoutInProgress;
	Snapshot.ResolveHealthThresholdPct = 0.f;
	Snapshot.bResolveRegenActive = false;
	Snapshot.ResolveHealthRegenPerSecond = 0.f;
	Snapshot.bUnbrokenResolveAvailable = false;
	Snapshot.bUnbrokenResolveInvulnerable = false;
	Snapshot.UnbrokenResolveRemainingSeconds = 0.f;

	const int32 AttributeCount = ToAttributeIndex(EProjectSinAttribute::Count);
	Snapshot.Attributes.Reserve(AttributeCount);
	for (int32 AttributeIndex = 0; AttributeIndex < AttributeCount; ++AttributeIndex)
	{
		const EProjectSinAttribute Attribute = static_cast<EProjectSinAttribute>(AttributeIndex);
		FProjectSinAttributeState AttributeState;
		AttributeState.Attribute = Attribute;
		AttributeState.DisplayName = GetAttributeDisplayName(Attribute);
		AttributeState.Level = GetAttributeLevel(Attribute);
		AttributeState.NextLevelCost = GetUpgradeCost(Attribute);
		AttributeState.bMilestone5Unlocked = HasMilestone(
			Attribute,
			Attribute == EProjectSinAttribute::Celerity ? CelerityYMenuMilestoneLevel : 5);
		AttributeState.bMilestone10Unlocked = HasMilestone(
			Attribute,
			Attribute == EProjectSinAttribute::Celerity ? CelerityDeathMilestoneLevel : 10);
		Snapshot.Attributes.Add(AttributeState);
	}

	AddMilestoneSnapshot(Snapshot.Milestones, TEXT("SecondBreath"), EProjectSinAttribute::Willpower, 5);
	AddMilestoneSnapshot(Snapshot.Milestones, TEXT("UnbrokenMind"), EProjectSinAttribute::Willpower, 10);
	AddMilestoneSnapshot(Snapshot.Milestones, TEXT("SteadyHands"), EProjectSinAttribute::Cunning, 5);
	AddMilestoneSnapshot(Snapshot.Milestones, TEXT("CleanGetaway"), EProjectSinAttribute::Cunning, 10);
	AddMilestoneSnapshot(Snapshot.Milestones, CelerityYMenuSurgeAbilityId, EProjectSinAttribute::Celerity, CelerityYMenuMilestoneLevel);
	AddMilestoneSnapshot(Snapshot.Milestones, CelerityRecoveredMomentumAbilityId, EProjectSinAttribute::Celerity, CelerityDeathMilestoneLevel);
	return Snapshot;
}

float UProjectSinfulAscensionComponent::GetEffectiveNormalMoveSpeed() const
{
	const float BaseMoveSpeed = ResolveBaseMoveSpeedForBonuses();
	return FMath::Max(0.f, BaseMoveSpeed + CalculateDesiredMoveSpeedBonus());
}

float UProjectSinfulAscensionComponent::GetEffectiveMoveSpeedBonus() const
{
	return CalculateDesiredMoveSpeedBonus();
}

int32 UProjectSinfulAscensionComponent::GrantSxp(const FName ReasonId, const int32 Amount, const bool bSinfulEvent)
{
	return GrantSxpInternal(ReasonId, Amount, bSinfulEvent, ESxpGrantSource::Standard);
}

int32 UProjectSinfulAscensionComponent::GrantSxpInternal(
	const FName ReasonId,
	const int32 Amount,
	const bool bSinfulEvent,
	const UProjectSinfulAscensionComponent::ESxpGrantSource Source)
{
	if (Amount <= 0)
	{
		return 0;
	}

	const int32 OldCurrentRunSxp = CurrentRunSxp;
	const int32 OldMetaBankSxp = MetaBankSxp;
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	int32 FinalAmount = FMath::Max(0, FMath::RoundToInt(static_cast<float>(Amount) * ResolveSxpSourceMultiplier(Source, Settings) * ResolveExternalSxpGainMultiplier()));

	if (bSinfulEvent)
	{
		FinalAmount += GetAttributeLevel(EProjectSinAttribute::Allure) * (Settings ? Settings->AllureSxpPerSinfulLevel : 4);

		if (bAllureMarkedInteraction)
		{
			FinalAmount += Settings ? Settings->AllureMilestone10MarkedSxp : 250;
			GrantSensation(TEXT("Allure.MarkedInteraction"), 0.f, Settings ? Settings->AllureMilestone10MarkedLust : 40.f, 0.f);
			bAllureMarkedInteraction = false;
		}
	}

	FinalAmount += CalculateCelerityFlatSxpBonus(Settings);
	if (FinalAmount <= 0)
	{
		return 0;
	}

	CurrentRunSxp = FMath::Max(0, CurrentRunSxp + FinalAmount);
	BroadcastSxpChanged(OldCurrentRunSxp, OldMetaBankSxp);
	(void)ReasonId;
	return FinalAmount;
}

int32 UProjectSinfulAscensionComponent::GrantSxpWithAttributeAffinity(
	const FName ReasonId,
	const int32 Amount,
	const bool bSinfulEvent,
	const TArray<EProjectSinAttribute>& Affinities)
{
	return GrantSxpWithAttributeAffinityInternal(ReasonId, Amount, bSinfulEvent, Affinities, ESxpGrantSource::Standard);
}

int32 UProjectSinfulAscensionComponent::GrantSxpWithAttributeAffinityInternal(
	const FName ReasonId,
	const int32 Amount,
	const bool bSinfulEvent,
	const TArray<EProjectSinAttribute>& Affinities,
	const UProjectSinfulAscensionComponent::ESxpGrantSource Source)
{
	if (Amount <= 0)
	{
		return 0;
	}

	const float Multiplier = ResolveBestSXPGainMultiplier(Affinities);
	const int32 AdjustedAmount = FMath::Max(0, FMath::RoundToInt(static_cast<float>(Amount) * Multiplier));
	return GrantSxpInternal(ReasonId, AdjustedAmount, bSinfulEvent, Source);
}

float UProjectSinfulAscensionComponent::ResolveSxpSourceMultiplier(
	const UProjectSinfulAscensionComponent::ESxpGrantSource Source,
	const UProjectSinfulAscensionSettings* Settings) const
{
	if (Source == ESxpGrantSource::YMenuAction
		&& HasMilestone(EProjectSinAttribute::Celerity, FMath::Max(0, Settings ? Settings->CelerityYMenuActionSxpMilestoneLevel : 5)))
	{
		return FMath::Max(0.f, Settings ? Settings->CelerityYMenuActionSxpMultiplier : 1.5f);
	}

	return 1.f;
}

float UProjectSinfulAscensionComponent::ResolveExternalSxpGainMultiplier() const
{
	float Multiplier = 1.0f;
	for (const TPair<FName, float>& Pair : ExternalSxpGainMultipliersBySource)
	{
		Multiplier *= FMath::Max(0.0f, Pair.Value);
	}
	return Multiplier;
}

float UProjectSinfulAscensionComponent::ResolveExternalFlatDamageBonus() const
{
	float BonusDamage = 0.0f;
	for (const TPair<FName, float>& Pair : ExternalFlatDamageBonusesBySource)
	{
		BonusDamage += FMath::Max(0.0f, Pair.Value);
	}
	return BonusDamage;
}

int32 UProjectSinfulAscensionComponent::CalculateCelerityFlatSxpBonus(const UProjectSinfulAscensionSettings* Settings) const
{
	return FMath::Max(0, GetAttributeLevel(EProjectSinAttribute::Celerity))
		* FMath::Max(0, Settings ? Settings->CelerityFlatSxpPerLevel : 2);
}

bool UProjectSinfulAscensionComponent::SpendSxpOnAttribute(const EProjectSinAttribute Attribute)
{
	if (!IsAttributeIndexValid(Attribute))
	{
		return false;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const int32 MaxAttributeLevel = FMath::Max(Settings ? Settings->MaxSinAttributeLevel : 100, 1);
	const int32 AttributeIndex = ToAttributeIndex(Attribute);
	const int32 OldLevel = AttributeLevels[AttributeIndex];
	if (OldLevel >= MaxAttributeLevel)
	{
		return false;
	}

	const int32 Cost = GetUpgradeCost(Attribute);
	if (CurrentRunSxp < Cost)
	{
		return false;
	}

	const int32 OldCurrentRunSxp = CurrentRunSxp;
	const int32 OldMetaBankSxp = MetaBankSxp;
	CurrentRunSxp -= Cost;
	AttributeLevels[AttributeIndex] = FMath::Min(OldLevel + 1, MaxAttributeLevel);

	ApplyPassiveAttributeEffects();
	BroadcastSxpChanged(OldCurrentRunSxp, OldMetaBankSxp);
	OnAttributeLevelChanged.Broadcast(Attribute, OldLevel, AttributeLevels[AttributeIndex], GetUpgradeCost(Attribute));
	BroadcastMilestonesForLevelRange(Attribute, OldLevel, AttributeLevels[AttributeIndex]);

	return true;
}

bool UProjectSinfulAscensionComponent::ApplyFreeAttributeLevels(const EProjectSinAttribute Attribute, const int32 Delta)
{
	if (!IsAttributeIndexValid(Attribute) || Delta <= 0)
	{
		return false;
	}

	InitializeAttributeStorage();

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const int32 MaxAttributeLevel = FMath::Max(Settings ? Settings->MaxSinAttributeLevel : 100, 1);
	const int32 AttributeIndex = ToAttributeIndex(Attribute);
	const int32 OldLevel = AttributeLevels[AttributeIndex];
	const int32 NewLevel = FMath::Clamp(OldLevel + Delta, 0, MaxAttributeLevel);
	if (NewLevel == OldLevel)
	{
		return false;
	}

	AttributeLevels[AttributeIndex] = NewLevel;
	ApplyPassiveAttributeEffects();
	OnAttributeLevelChanged.Broadcast(Attribute, OldLevel, NewLevel, GetUpgradeCost(Attribute));
	BroadcastMilestonesForLevelRange(Attribute, OldLevel, NewLevel);
	return true;
}

void UProjectSinfulAscensionComponent::ResetRunProgressForBackgroundChange()
{
	const int32 OldCurrentRunSxp = CurrentRunSxp;
	const int32 OldMetaBankSxp = MetaBankSxp;
	CurrentRunSxp = 0;
	ResetRunAttributes();
	ClearSXPGainMultipliers();
	BroadcastSxpChanged(OldCurrentRunSxp, OldMetaBankSxp);
}

void UProjectSinfulAscensionComponent::SetSXPGainMultipliers(const TMap<EProjectSinAttribute, float>& InMultipliers)
{
	SXPGainMultipliersByAttribute.Reset();
	for (const TPair<EProjectSinAttribute, float>& Pair : InMultipliers)
	{
		if (!IsAttributeIndexValid(Pair.Key))
		{
			continue;
		}

		SXPGainMultipliersByAttribute.Add(Pair.Key, FMath::Max(0.0f, Pair.Value));
	}
}

void UProjectSinfulAscensionComponent::ClearSXPGainMultipliers()
{
	SXPGainMultipliersByAttribute.Reset();
}

void UProjectSinfulAscensionComponent::SetExternalSXPGainMultiplier(const FName SourceId, const float Multiplier)
{
	if (SourceId.IsNone())
	{
		return;
	}

	ExternalSxpGainMultipliersBySource.Add(SourceId, FMath::Max(0.0f, Multiplier));
}

void UProjectSinfulAscensionComponent::ClearExternalSXPGainMultiplier(const FName SourceId)
{
	if (!SourceId.IsNone())
	{
		ExternalSxpGainMultipliersBySource.Remove(SourceId);
	}
}

void UProjectSinfulAscensionComponent::SetExternalFlatDamageBonus(const FName SourceId, const float FlatDamageBonus)
{
	if (SourceId.IsNone())
	{
		return;
	}

	ExternalFlatDamageBonusesBySource.Add(SourceId, FMath::Max(0.0f, FlatDamageBonus));
}

void UProjectSinfulAscensionComponent::ClearExternalFlatDamageBonus(const FName SourceId)
{
	if (!SourceId.IsNone())
	{
		ExternalFlatDamageBonusesBySource.Remove(SourceId);
	}
}

float UProjectSinfulAscensionComponent::GetSXPGainMultiplierForAttribute(const EProjectSinAttribute Attribute) const
{
	if (!IsAttributeIndexValid(Attribute))
	{
		return 1.0f;
	}

	if (const float* Multiplier = SXPGainMultipliersByAttribute.Find(Attribute))
	{
		return FMath::Max(0.0f, *Multiplier);
	}

	return 1.0f;
}

int32 UProjectSinfulAscensionComponent::WithdrawMetaSxp(const int32 RequestedAmount)
{
	const int32 AmountToWithdraw = RequestedAmount <= 0 ? MetaBankSxp : FMath::Min(RequestedAmount, MetaBankSxp);
	if (AmountToWithdraw <= 0)
	{
		return 0;
	}

	const int32 OldCurrentRunSxp = CurrentRunSxp;
	const int32 OldMetaBankSxp = MetaBankSxp;
	MetaBankSxp -= AmountToWithdraw;
	CurrentRunSxp += AmountToWithdraw;
	BroadcastSxpChanged(OldCurrentRunSxp, OldMetaBankSxp);
	SavePersistentState();
	return AmountToWithdraw;
}

void UProjectSinfulAscensionComponent::HandleRunDeath()
{
	EndMasochismPainRaptureState();
	bCombatStateActive = false;
	LastCombatImpactTimeSeconds = -FLT_MAX;
	AppliedSecondBreathTransitionIds.Reset();

	const int32 OldCurrentRunSxp = CurrentRunSxp;
	const int32 OldMetaBankSxp = MetaBankSxp;
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float LossRatio = Settings ? FMath::Clamp(Settings->DeathRunLossRatio, 0.f, 1.f) : 0.90f;
	const int32 SavedAmount = bEternalSinMode
		? CurrentRunSxp
		: FMath::Max(0, FMath::RoundToInt(static_cast<float>(CurrentRunSxp) * (1.f - LossRatio)));
	const int32 LostAmount = FMath::Max(0, CurrentRunSxp - SavedAmount);
	const int32 CelerityRecoveredAmount = (!bEternalSinMode && HasMilestone(
		EProjectSinAttribute::Celerity,
		FMath::Max(0, Settings ? Settings->CelerityDeathLostSxpToMetaMilestoneLevel : 10)))
		? LostAmount
		: 0;

	MetaBankSxp += SavedAmount + CelerityRecoveredAmount;
	CurrentRunSxp = 0;
	ResetRunAttributes();
	BroadcastSxpChanged(OldCurrentRunSxp, OldMetaBankSxp);
	SavePersistentState();
}

void UProjectSinfulAscensionComponent::SetEternalSinMode(const bool bEnabled)
{
	if (bEternalSinMode == bEnabled)
	{
		return;
	}

	bEternalSinMode = bEnabled;
	SavePersistentState();
	OnSxpChanged.Broadcast(CurrentRunSxp, CurrentRunSxp, MetaBankSxp, MetaBankSxp);
}

bool UProjectSinfulAscensionComponent::ActivateMilestoneAbility(const FName AbilityId)
{
	if (AbilityId == TEXT("SirensCall") && HasMilestone(EProjectSinAttribute::Allure, 10))
	{
		bAllureMarkedInteraction = true;
		OnMilestoneTriggered.Broadcast(AbilityId, EProjectSinAttribute::Allure, 10);
		return true;
	}

	return false;
}

void UProjectSinfulAscensionComponent::GrantSensation(const FName ReasonId, float MadnessDelta, float LustDelta, float PainDelta)
{
	if (!NeedsComponent)
	{
		RefreshCachedComponents();
	}

	if (!NeedsComponent)
	{
		return;
	}

	AddSensationGainTimestamp(MadnessName, IsSensationMaxHoldActive(MadnessName) ? 0.f : NeedsComponent->ApplySensationDeltaValue(MadnessName, MadnessDelta, true, true));
	AddSensationGainTimestamp(LustName, IsSensationMaxHoldActive(LustName) ? 0.f : NeedsComponent->ApplySensationDeltaValue(LustName, LustDelta, true, true));
	AddSensationGainTimestamp(PainName, IsSensationMaxHoldActive(PainName) ? 0.f : NeedsComponent->ApplySensationDeltaValue(PainName, PainDelta, true, true));
	UpdateSensationConsequences(0.f);
	(void)ReasonId;
}

void UProjectSinfulAscensionComponent::NotifyNudityStateChanged(const bool bInIsNude)
{
	bIsNude = bInIsNude;
	if (bIsNude)
	{
		GrantSensation(TEXT("Nudity.Enabled"), 3.f, 25.f, 2.f);
	}
}

void UProjectSinfulAscensionComponent::NotifySinfulInteraction(const FName InteractionId)
{
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	GrantSxpWithAttributeAffinity(
		InteractionId.IsNone() ? TEXT("SinfulInteraction") : InteractionId,
		Settings ? Settings->SinfulInteractionSxp : 150,
		true,
		{ EProjectSinAttribute::Allure, EProjectSinAttribute::Cunning });
	GrantSensation(TEXT("SinfulInteraction"), 8.f, 180.f, 5.f);
}

void UProjectSinfulAscensionComponent::NotifyCorruptObject(const FName ObjectId)
{
	GrantSxpWithAttributeAffinity(
		ObjectId.IsNone() ? TEXT("CorruptObject") : ObjectId,
		20,
		true,
		{ EProjectSinAttribute::Allure, EProjectSinAttribute::Cunning });
	GrantSensation(TEXT("CorruptObject"), 14.f, 45.f, 7.f);
}

void UProjectSinfulAscensionComponent::NotifyForbiddenFood(const FName ItemId)
{
	GrantSxpWithAttributeAffinity(
		ItemId.IsNone() ? TEXT("ForbiddenFood") : ItemId,
		25,
		true,
		{ EProjectSinAttribute::Allure, EProjectSinAttribute::Cunning });
	GrantSensation(TEXT("ForbiddenFood"), 18.f, 30.f, 10.f);
}

void UProjectSinfulAscensionComponent::NotifyTaggedConsumable(const FName ItemId)
{
	GrantSxpWithAttributeAffinity(
		ItemId.IsNone() ? TEXT("TaggedConsumable") : ItemId,
		15,
		true,
		{ EProjectSinAttribute::Allure, EProjectSinAttribute::Cunning });
	GrantSensation(TEXT("TaggedConsumable"), 4.f, 120.f, 6.f);
}

bool UProjectSinfulAscensionComponent::NotifyDungeonFloorCompleted(const FName FloorTransitionId)
{
	if (!HasMilestone(EProjectSinAttribute::Willpower, 5))
	{
		return false;
	}

	if (!FloorTransitionId.IsNone() && AppliedSecondBreathTransitionIds.Contains(FloorTransitionId))
	{
		return false;
	}

	if (!NeedsComponent)
	{
		RefreshCachedComponents();
	}

	if (!NeedsComponent)
	{
		return false;
	}

	ApplyPassiveAttributeEffects();
	const bool bApplied = ApplyWillpowerSecondBreathRecovery();
	if (!bApplied)
	{
		return false;
	}

	if (!FloorTransitionId.IsNone())
	{
		AppliedSecondBreathTransitionIds.Add(FloorTransitionId);
	}

	OnMilestoneTriggered.Broadcast(TEXT("SecondBreath"), EProjectSinAttribute::Willpower, 5);
	return true;
}

bool UProjectSinfulAscensionComponent::NotifySleepCompleted(const FName SleepSourceId)
{
	(void)SleepSourceId;

	if (!NeedsComponent || !StatusComponent)
	{
		RefreshCachedComponents();
	}

	if (StatusComponent && StatusComponent->IsStatusActive(TiredStatusName))
	{
		StatusComponent->ClearStatus(TiredStatusName);
	}

	if (!HasMilestone(EProjectSinAttribute::Faith, 10))
	{
		return false;
	}

	if (!NeedsComponent || !NeedsComponent->HasSensation(MadnessName))
	{
		return false;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float RestoreAmount = UProjectSinfulAscensionSettings::ComputeFaithSleepMadnessRestore(
		GetSensationMax(MadnessName),
		Settings ? Settings->FaithSleepMadnessRestorePct : 0.50f);
	if (RestoreAmount <= KINDA_SMALL_NUMBER || GetSensationCurrent(MadnessName) <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	MaxSensationHoldEndTimeSeconds.Remove(MadnessName);
	SetForcedStatus(FrenzyStatusName, false);
	bFrenzyForced = false;

	const float AppliedDelta = NeedsComponent->ModifySensationValue(MadnessName, -RestoreAmount, true);
	if (FMath::IsNearlyZero(AppliedDelta))
	{
		return false;
	}

	UpdateSensationConsequences(0.f);
	OnMilestoneTriggered.Broadcast(TEXT("SanctifiedRest"), EProjectSinAttribute::Faith, 10);
	return true;
}

bool UProjectSinfulAscensionComponent::TryActivateSurrender()
{
	if (!HasMilestone(EProjectSinAttribute::Masochism, 10) || bMasochismPainRaptureActive)
	{
		return false;
	}

	if (!DefeatFlowComponent || !StatusComponent || !EmoteComponent)
	{
		RefreshCachedComponents();
	}

	if (!DefeatFlowComponent
		|| DefeatFlowComponent->GetCurrentPhase() != EProjectDefeatPhase::None
		|| (StatusComponent && StatusComponent->IsBlackoutActive())
		|| (EmoteComponent && (EmoteComponent->IsEmoteActive() || EmoteComponent->IsActiveInteractionBlueprintScene())))
	{
		return false;
	}

	const bool bActivated = DefeatFlowComponent->RequestKnockoutOrPendingCrawl(
		EProjectKnockoutReason::Surrender,
		TEXT("Masochism.Surrender"));
	if (bActivated)
	{
		OnMilestoneTriggered.Broadcast(TEXT("Surrender"), EProjectSinAttribute::Masochism, 10);
	}
	return bActivated;
}

float UProjectSinfulAscensionComponent::ModifyIncomingHealthDamage(
	AActor* SourceActor,
	const FName DamageType,
	const float PostArmorDamage,
	float& OutFlatNegatedDamage,
	float& OutRaptureAbsorbedDamage)
{
	(void)SourceActor;
	OutFlatNegatedDamage = 0.f;
	OutRaptureAbsorbedDamage = 0.f;

	float RemainingDamage = FMath::Max(PostArmorDamage, 0.f);
	if (RemainingDamage <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	if (IsMasochismQualifiedHitDamageType(DamageType))
	{
		const float FlatNegation = UProjectSinfulAscensionSettings::ComputeMasochismFlatDamageNegation(
			GetAttributeLevel(EProjectSinAttribute::Masochism),
			Settings ? Settings->MasochismFlatDamageNegationPerLevel : 4.f);
		OutFlatNegatedDamage = FMath::Min(RemainingDamage, FlatNegation);
		RemainingDamage = FMath::Max(0.f, RemainingDamage - OutFlatNegatedDamage);
	}

	if (bMasochismPainRaptureActive && RemainingDamage > KINDA_SMALL_NUMBER)
	{
		OutRaptureAbsorbedDamage = RemainingDamage;
		RemainingDamage = 0.f;
	}

	return RemainingDamage;
}

bool UProjectSinfulAscensionComponent::TryDeferPainKnockout()
{
	if (HasMilestone(EProjectSinAttribute::Masochism, 10))
	{
		return TryApplyMasochismPainOverflow();
	}

	return TryStartOrRefreshMasochismPainRapture(false);
}

bool UProjectSinfulAscensionComponent::TryPreventDeathFromDamage(
	const FProjectIncomingHitContext& HitContext,
	const float CurrentHealth,
	float& OutSurvivingHealth)
{
	if (HitContext.AppliedDamage <= 0.f || CurrentHealth > HitContext.AppliedDamage + KINDA_SMALL_NUMBER)
	{
		return false;
	}

	if (!DefeatFlowComponent)
	{
		RefreshCachedComponents();
	}

	return DefeatFlowComponent
		? DefeatFlowComponent->TryHandleLethalDamage(HitContext, OutSurvivingHealth)
		: false;
}

void UProjectSinfulAscensionComponent::ModifyOutgoingDamageSpec(AActor* TargetActor, FProjectCombatDamageSpec& InOutDamageSpec)
{
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	float BonusDamage = GetAttributeLevel(EProjectSinAttribute::Sadism) * (Settings ? Settings->SadismDamagePerLevel : 2.f);

	if (IsSpellLikeDamage(InOutDamageSpec.DamageType))
	{
		BonusDamage += GetAttributeLevel(EProjectSinAttribute::Faith) * (Settings ? Settings->FaithSpellDamagePerLevel : 2.f);
	}

	InOutDamageSpec.FlatBonusDamage += FMath::Max(0.f, BonusDamage);
	InOutDamageSpec.FlatBonusDamage += ResolveExternalFlatDamageBonus();
	(void)TargetActor;
}

void UProjectSinfulAscensionComponent::NotifyDamageDealt(AActor* TargetActor, const FProjectCombatDamageResult& DamageResult)
{
	if (DamageResult.AppliedDamage <= 0.f)
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	GrantSxpWithAttributeAffinity(
		TEXT("DamageDealt"),
		Settings ? Settings->HitSxp : 3,
		false,
		{ IsSpellLikeDamage(DamageResult.DamageType) ? EProjectSinAttribute::Faith : EProjectSinAttribute::Sadism });
	GrantSensation(TEXT("DamageDealt"), 1.f, 5.f, 3.f);
	MarkCombatImpact();
	GrantSadismLustFromDamage(TargetActor, DamageResult);

	if (DamageResult.bKilledTarget)
	{
		RewardEnemyKill(TargetActor);
		GrantSadismExecuteLust(TargetActor, DamageResult);
	}

	(void)TargetActor;
}

void UProjectSinfulAscensionComponent::NotifyDamageReceived(const FProjectIncomingHitContext& HitContext)
{
	if (HitContext.AppliedDamage <= 0.f)
	{
		return;
	}

	if (!DefeatFlowComponent)
	{
		RefreshCachedComponents();
	}

	FProjectDefeatHitResolution HitResolution;
	const bool bQualifiedEnemyHit = FProjectDefeatHitResolver::ResolveQualifiedEnemyActor(HitContext, ResolveQualifiedEnemyHints(), HitResolution);

	GrantSxpWithAttributeAffinity(
		TEXT("DamageReceived"),
		2,
		false,
		{ EProjectSinAttribute::Masochism, EProjectSinAttribute::Willpower });
	GrantSensation(
		TEXT("DamageReceived"),
		bIsNude ? 6.f : 2.f,
		bIsNude ? 40.f : 8.f,
		bQualifiedEnemyHit ? 0.f : (bIsNude ? 10.f : 6.f));
	MarkCombatImpact();
	GrantMasochismLustFromQualifiedHit(HitContext);
	if (HitContext.PainAppliedDelta > KINDA_SMALL_NUMBER)
	{
		QueueMasochismPainReflection(HitContext.PainAppliedDelta);
		LastQualifiedPainGainTimeSeconds = HitContext.WorldTimeSeconds > 0.f
			? HitContext.WorldTimeSeconds
			: (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f);
		if (bQualifiedEnemyHit && GetSensationCurrent(PainName) >= GetSensationMax(PainName) - KINDA_SMALL_NUMBER)
		{
			if (HasMilestone(EProjectSinAttribute::Masochism, 10))
			{
				TryDeferPainKnockout();
			}
			else
			{
				TryStartOrRefreshMasochismPainRapture(true);
			}
		}
	}

	if (DefeatFlowComponent)
	{
		DefeatFlowComponent->NotifyDamageReceived(HitContext);
	}

	if (HitContext.bKilledTarget)
	{
		HandleRunDeath();
	}
}

void UProjectSinfulAscensionComponent::HandleBlackoutChanged(const bool bBlackoutActive)
{
	if (bBlackoutActive || !bOverloadBlackoutInProgress)
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	if (StatusComponent && Settings && Settings->SensoryOverloadRecoveryDebuffSeconds > 0.f)
	{
		StatusComponent->ApplyStatus(ExhaustedRecoveryStatusName, Settings->SensoryOverloadRecoveryDebuffSeconds, GetOwner());
	}

	bOverloadBlackoutInProgress = false;
	SensoryOverloadProgressSeconds = 0.f;
	OnSensoryOverloadChanged.Broadcast(false, SensoryOverloadProgressSeconds);
}

void UProjectSinfulAscensionComponent::HandleRealtimeCombatImpact(const FProjectRealtimeCombatImpact& Impact)
{
	if (Impact.DamageDelta <= 0.f)
	{
		return;
	}

	MarkCombatImpact();

	if (Impact.bEnemyImpact)
	{
		if (Impact.bKilledTarget || Impact.ImpactType == EProjectRealtimeCombatImpactType::EnemyKilled)
		{
			RewardEnemyKill(Impact.Actor);
		}
	}

	if (Impact.bOwnerImpact)
	{
		if (bMasochismPainRaptureActive)
		{
			RestoreMasochismRaptureHealthFloor();
			return;
		}

		TryApplyQualifiedPainFallback(Impact);
		MaintainMasochismPainRapture();
	}
}

void UProjectSinfulAscensionComponent::BindToRealtimeSnapshotComponent()
{
	AActor* OwnerActor = GetOwner();
	UProjectRealtimeSnapshotComponent* NewSnapshotComponent = OwnerActor ? OwnerActor->FindComponentByClass<UProjectRealtimeSnapshotComponent>() : nullptr;
	if (RealtimeSnapshotComponent == NewSnapshotComponent)
	{
		return;
	}

	UnbindFromRealtimeSnapshotComponent();
	RealtimeSnapshotComponent = NewSnapshotComponent;
	if (RealtimeSnapshotComponent)
	{
		RealtimeSnapshotComponent->OnRealtimeCombatImpact.AddUniqueDynamic(this, &ThisClass::HandleRealtimeCombatImpact);
	}
}

void UProjectSinfulAscensionComponent::UnbindFromRealtimeSnapshotComponent()
{
	if (RealtimeSnapshotComponent)
	{
		RealtimeSnapshotComponent->OnRealtimeCombatImpact.RemoveDynamic(this, &ThisClass::HandleRealtimeCombatImpact);
		RealtimeSnapshotComponent = nullptr;
	}
}

void UProjectSinfulAscensionComponent::RewardEnemyKill(AActor* TargetActor)
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	const FName TargetKey = TargetActor ? TargetActor->GetFName() : NAME_None;
	if (!TargetKey.IsNone())
	{
		const float* LastRewardTime = LastEnemyKillRewardTimesByActorName.Find(TargetKey);
		if (LastRewardTime && (Now - *LastRewardTime) < 1.f)
		{
			return;
		}

		LastEnemyKillRewardTimesByActorName.Add(TargetKey, Now);
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	GrantSxpWithAttributeAffinity(
		TEXT("EnemyKilled"),
		Settings ? Settings->KillSxp : 15,
		false,
		{ EProjectSinAttribute::Willpower, EProjectSinAttribute::Cunning });
	GrantSensation(TEXT("EnemyKilled"), 5.f, 10.f, 4.f);

	if (HasMilestone(EProjectSinAttribute::Allure, 5) && (GetNormalizedSensation(LustName) > 0.5f || GetNormalizedSensation(MadnessName) > 0.5f))
	{
		GrantSxpWithAttributeAffinity(
			TEXT("Allure.HighSensationKill"),
			Settings ? Settings->AllureMilestone5KillSxp : 25,
			true,
			{ EProjectSinAttribute::Allure });
	}
}

void UProjectSinfulAscensionComponent::GrantSadismLustFromDamage(AActor* TargetActor, const FProjectCombatDamageResult& DamageResult)
{
	if (!HasMilestone(EProjectSinAttribute::Sadism, 5)
		|| !TargetActor
		|| !IsQualifiedMaleEnemyActor(TargetActor)
		|| IsSpellLikeDamage(DamageResult.DamageType))
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float LustGain = UProjectSinfulAscensionSettings::ComputeScaledResourceGain(
		DamageResult.AppliedDamage,
		Settings ? Settings->SadismMilestone5LustFromDamagePct : 0.25f,
		Settings ? Settings->SadismMilestone5LustPerHitCap : 250.f);
	if (LustGain > KINDA_SMALL_NUMBER)
	{
		GrantSensation(TEXT("Sadism.DamageLust"), 0.f, LustGain, 0.f);
	}
}

void UProjectSinfulAscensionComponent::GrantSadismExecuteLust(AActor* TargetActor, const FProjectCombatDamageResult& DamageResult)
{
	if (!HasMilestone(EProjectSinAttribute::Sadism, 10)
		|| !DamageResult.bKilledTarget
		|| !TargetActor
		|| !IsQualifiedMaleEnemyActor(TargetActor)
		|| IsSpellLikeDamage(DamageResult.DamageType))
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float MaxHealth = FMath::Max(DamageResult.TargetMaxValue, KINDA_SMALL_NUMBER);
	const float PreHitHealthRatio = FMath::Clamp(DamageResult.PreDamageTargetValue / MaxHealth, 0.f, 1.f);
	if (PreHitHealthRatio > (Settings ? Settings->SadismExecuteHealthThresholdPct : 0.20f))
	{
		return;
	}

	GrantSensation(TEXT("Sadism.ExecuteLust"), 0.f, Settings ? Settings->SadismMilestone10ExecuteLust : 500.f, 0.f);
}

void UProjectSinfulAscensionComponent::GrantMasochismLustFromQualifiedHit(const FProjectIncomingHitContext& HitContext)
{
	if (HitContext.AppliedDamage <= 0.f
		|| !HasMilestone(EProjectSinAttribute::Masochism, 5)
		|| !IsMasochismQualifiedHitDamageType(HitContext.DamageType))
	{
		return;
	}

	FProjectDefeatHitResolution HitResolution;
	if (!FProjectDefeatHitResolver::ResolveQualifiedEnemyActor(HitContext, ResolveQualifiedEnemyHints(), HitResolution))
	{
		return;
	}

	if (HitResolution.ResolvedActor.Get() == GetOwner())
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	GrantSensation(TEXT("Masochism.QualifiedHit"), 0.f, Settings ? Settings->MasochismMilestone5LustPerQualifiedHit : 50.f, 0.f);
}

bool UProjectSinfulAscensionComponent::TryApplyMasochismPainOverflow()
{
	if (!HasMilestone(EProjectSinAttribute::Masochism, 10))
	{
		return false;
	}

	const float PainMax = GetSensationMax(PainName);
	if (PainMax <= KINDA_SMALL_NUMBER || GetSensationCurrent(PainName) + KINDA_SMALL_NUMBER < PainMax)
	{
		return false;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float LustBonus = UProjectSinfulAscensionSettings::ComputeMasochismPainOverflowLust(
		GetSensationMax(LustName),
		Settings ? Settings->MasochismPainOverflowLustPct : 0.25f);
	if (LustBonus > KINDA_SMALL_NUMBER)
	{
		GrantSensation(TEXT("Masochism.PainOverflow"), 0.f, LustBonus, 0.f);
	}

	MaxSensationHoldEndTimeSeconds.Remove(PainName);
	SetSensationCurrent(PainName, 0.f);
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	LastSensationGainTimeSeconds.Add(PainName, Now);
	LastSensationDecayTimeSeconds.Add(PainName, Now);
	if (StatusComponent)
	{
		StatusComponent->ClearStatus(ExtremePainStatusName);
	}
	OnMilestoneTriggered.Broadcast(TEXT("PainOverflow"), EProjectSinAttribute::Masochism, 10);
	return true;
}

void UProjectSinfulAscensionComponent::QueueMasochismPainReflection(const float PainAppliedDelta)
{
	if (!HasMilestone(EProjectSinAttribute::Masochism, 10) || PainAppliedDelta <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float LustDelta = UProjectSinfulAscensionSettings::ComputeMasochismPainReflectionLust(
		PainAppliedDelta,
		Settings ? Settings->MasochismPainReflectionPct : 5.0f);
	if (LustDelta <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	PendingMasochismPainReflectionLust += LustDelta;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	if (TimerManager.IsTimerActive(MasochismPainReflectionTimerHandle))
	{
		return;
	}

	const float DelaySeconds = FMath::Max(Settings ? Settings->MasochismPainReflectionDelaySeconds : 0.10f, 0.f);
	if (DelaySeconds <= KINDA_SMALL_NUMBER)
	{
		FlushMasochismPainReflection();
		return;
	}

	TimerManager.SetTimer(
		MasochismPainReflectionTimerHandle,
		this,
		&ThisClass::FlushMasochismPainReflection,
		DelaySeconds,
		false);
}

void UProjectSinfulAscensionComponent::FlushMasochismPainReflection()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MasochismPainReflectionTimerHandle);
	}

	const float LustDelta = PendingMasochismPainReflectionLust;
	PendingMasochismPainReflectionLust = 0.f;
	if (LustDelta <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	GrantSensation(TEXT("Masochism.PainReflection"), 0.f, LustDelta, 0.f);
}

#if WITH_DEV_AUTOMATION_TESTS
int32 UProjectSinfulAscensionComponent::AutomationGrantYMenuActionSxp(
	const FName ReasonId,
	const int32 Amount,
	const bool bSinfulEvent)
{
	return GrantSxpInternal(ReasonId, Amount, bSinfulEvent, ESxpGrantSource::YMenuAction);
}

void UProjectSinfulAscensionComponent::AutomationApplyFaithMadnessRecovery(const float DeltaTime)
{
	UpdateFaithMadnessRecovery(DeltaTime);
}

void UProjectSinfulAscensionComponent::AutomationQueueMasochismPainReflection(const float PainAppliedDelta)
{
	QueueMasochismPainReflection(PainAppliedDelta);
}

void UProjectSinfulAscensionComponent::AutomationFlushMasochismPainReflection()
{
	FlushMasochismPainReflection();
}

float UProjectSinfulAscensionComponent::AutomationGetPendingMasochismPainReflectionLust() const
{
	return PendingMasochismPainReflectionLust;
}
#endif

void UProjectSinfulAscensionComponent::TryApplyQualifiedPainFallback(const FProjectRealtimeCombatImpact& Impact)
{
	if (!Impact.bOwnerImpact || Impact.DamageDelta <= KINDA_SMALL_NUMBER || !NeedsComponent)
	{
		return;
	}

	const AActor* OwnerActor = GetOwner();
	if (ProjectCombatTags::IsActorIntimacyShielded(OwnerActor)
		|| ProjectCombatTags::IsActorIntimacyShielded(Impact.LikelySourceActor))
	{
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if ((Now - LastQualifiedPainGainTimeSeconds) <= 0.15f)
	{
		return;
	}

	FProjectIncomingHitContext FallbackHitContext;
	FallbackHitContext.TargetActor = GetOwner();
	FallbackHitContext.SourceActor = Impact.LikelySourceActor;
	FallbackHitContext.DamageCauser = Impact.LikelySourceActor;
	FallbackHitContext.InstigatorActor = Impact.LikelySourceActor ? Impact.LikelySourceActor->GetInstigator() : nullptr;
	FallbackHitContext.OwnerActor = Impact.LikelySourceActor ? Impact.LikelySourceActor->GetOwner() : nullptr;
	FallbackHitContext.AttachParentActor = Impact.LikelySourceActor ? Impact.LikelySourceActor->GetAttachParentActor() : nullptr;
	FallbackHitContext.DamageType = TEXT("RealtimeFallback");
	FallbackHitContext.RequestedDamage = Impact.DamageDelta;
	FallbackHitContext.AppliedDamage = Impact.DamageDelta;
	FallbackHitContext.RemainingHealth = CombatAttributeComponent
		? CombatAttributeComponent->GetAttributeCurrentValue(CombatAttributeComponent->HealthAttributeName)
		: Impact.NewHealth;
	FallbackHitContext.WorldTimeSeconds = Now;

	FProjectDefeatHitResolution HitResolution;
	if (!FProjectDefeatHitResolver::ResolveQualifiedEnemyActor(FallbackHitContext, ResolveQualifiedEnemyHints(), HitResolution))
	{
		return;
	}

	const UProjectDefeatFlowSettings* Settings = UProjectDefeatFlowSettings::Get();
	const float RequestedPainDelta = UProjectDefeatFlowSettings::ComputePainFromAppliedDamage(
		FallbackHitContext.AppliedDamage,
		Settings ? Settings->PainPerAppliedDamage : 0.01f);
	if (RequestedPainDelta <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	FallbackHitContext.PainAppliedDelta = NeedsComponent->ModifySensationValue(PainName, RequestedPainDelta, true);
	if (FallbackHitContext.PainAppliedDelta <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	LastQualifiedPainGainTimeSeconds = Now;
	QueueMasochismPainReflection(FallbackHitContext.PainAppliedDelta);
	if (GetSensationCurrent(PainName) >= GetSensationMax(PainName) - KINDA_SMALL_NUMBER)
	{
		if (HasMilestone(EProjectSinAttribute::Masochism, 10))
		{
			TryDeferPainKnockout();
		}
		else
		{
			TryStartOrRefreshMasochismPainRapture(true);
		}
	}

	if (DefeatFlowComponent)
	{
		DefeatFlowComponent->NotifyDamageReceived(FallbackHitContext);
	}
}

void UProjectSinfulAscensionComponent::RefreshCachedComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		NeedsComponent = nullptr;
		StatusComponent = nullptr;
		CombatAttributeComponent = nullptr;
		RealtimeSnapshotComponent = nullptr;
		LocomotionOverrideComponent = nullptr;
		EmoteComponent = nullptr;
		DirtyPawnEffectsBridgeComponent = nullptr;
		AcfDamageHandlerComponent = nullptr;
		AcfEffectsManagerComponent = nullptr;
		return;
	}

	NeedsComponent = Owner->FindComponentByClass<UProjectSurvivalNeedsComponent>();
	BindToRealtimeSnapshotComponent();
	UProjectSurvivalStatusComponent* NewStatusComponent = Owner->FindComponentByClass<UProjectSurvivalStatusComponent>();
	if (StatusComponent != NewStatusComponent)
	{
		if (StatusComponent)
		{
			StatusComponent->ClearStatusImmunitySource(WillpowerImmunitySourceId);
			StatusComponent->OnBlackoutChanged.RemoveDynamic(this, &ThisClass::HandleBlackoutChanged);
		}

		StatusComponent = NewStatusComponent;
		if (StatusComponent)
		{
			StatusComponent->OnBlackoutChanged.AddUniqueDynamic(this, &ThisClass::HandleBlackoutChanged);
			ApplyWillpowerStatusImmunities();
		}
	}

	CombatAttributeComponent = Owner->FindComponentByClass<UProjectCombatAttributeComponent>();
	DefeatFlowComponent = Owner->FindComponentByClass<UProjectDefeatFlowComponent>();
	LocomotionOverrideComponent = Owner->FindComponentByClass<UProjectLocomotionOverrideComponent>();
	EmoteComponent = Owner->FindComponentByClass<UProjectEmoteComponent>();
	DirtyPawnEffectsBridgeComponent = Owner->FindComponentByClass<UProjectDirtyPawnEffectsBridgeComponent>();
	AcfDamageHandlerComponent = Owner->FindComponentByClass<UACFDamageHandlerComponent>();
	AcfEffectsManagerComponent = Owner->FindComponentByClass<UACFEffectsManagerComponent>();
}

void UProjectSinfulAscensionComponent::EnsureDirtyPawnEffectsBridge()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	DirtyPawnEffectsBridgeComponent = Owner->FindComponentByClass<UProjectDirtyPawnEffectsBridgeComponent>();
	if (!DirtyPawnEffectsBridgeComponent)
	{
		DirtyPawnEffectsBridgeComponent = NewObject<UProjectDirtyPawnEffectsBridgeComponent>(Owner, UProjectDirtyPawnEffectsBridgeComponent::StaticClass(), TEXT("ProjectDirtyPawnEffectsBridge"));
		if (DirtyPawnEffectsBridgeComponent)
		{
			Owner->AddInstanceComponent(DirtyPawnEffectsBridgeComponent);
			DirtyPawnEffectsBridgeComponent->RegisterComponent();
		}
	}

	if (DirtyPawnEffectsBridgeComponent)
	{
		DirtyPawnEffectsBridgeComponent->RefreshDirtyPawnEffects();
	}
}

void UProjectSinfulAscensionComponent::InitializeAttributeStorage()
{
	const int32 AttributeCount = ToAttributeIndex(EProjectSinAttribute::Count);
	if (AttributeLevels.Num() != AttributeCount)
	{
		AttributeLevels.Init(0, AttributeCount);
	}
}

void UProjectSinfulAscensionComponent::LoadPersistentState()
{
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const FString SlotName = Settings ? Settings->SaveSlotName : TEXT("ProjectSinfulAscension");
	const int32 UserIndex = Settings ? Settings->SaveUserIndex : 0;

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		return;
	}

	if (USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex))
	{
		if (const UProjectSinfulAscensionSaveGame* SinSave = Cast<UProjectSinfulAscensionSaveGame>(LoadedSave))
		{
			MetaBankSxp = FMath::Max(0, SinSave->MetaBankSxp);
			bEternalSinMode = SinSave->bEternalSinMode;
			return;
		}
	}
}

void UProjectSinfulAscensionComponent::SavePersistentState() const
{
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const FString SlotName = Settings ? Settings->SaveSlotName : TEXT("ProjectSinfulAscension");
	const int32 UserIndex = Settings ? Settings->SaveUserIndex : 0;

	UProjectSinfulAscensionSaveGame* SaveGame = Cast<UProjectSinfulAscensionSaveGame>(UGameplayStatics::CreateSaveGameObject(UProjectSinfulAscensionSaveGame::StaticClass()));
	if (!SaveGame)
	{
		return;
	}

	SaveGame->MetaBankSxp = FMath::Max(0, MetaBankSxp);
	SaveGame->bEternalSinMode = bEternalSinMode;
	UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
}

void UProjectSinfulAscensionComponent::BroadcastSxpChanged(const int32 OldCurrentRunSxp, const int32 OldMetaBankSxp)
{
	if (OldCurrentRunSxp != CurrentRunSxp || OldMetaBankSxp != MetaBankSxp)
	{
		OnSxpChanged.Broadcast(OldCurrentRunSxp, CurrentRunSxp, OldMetaBankSxp, MetaBankSxp);
	}
}

void UProjectSinfulAscensionComponent::ResetRunAttributes()
{
	for (int32 AttributeIndex = 0; AttributeIndex < AttributeLevels.Num(); ++AttributeIndex)
	{
		const int32 OldLevel = AttributeLevels[AttributeIndex];
		if (OldLevel == 0)
		{
			continue;
		}

		AttributeLevels[AttributeIndex] = 0;
		OnAttributeLevelChanged.Broadcast(static_cast<EProjectSinAttribute>(AttributeIndex), OldLevel, 0, GetUpgradeCost(static_cast<EProjectSinAttribute>(AttributeIndex)));
	}

	bAllureMarkedInteraction = false;
	bCombatStateActive = false;
	EndMasochismPainRaptureState();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MasochismPainReflectionTimerHandle);
	}
	PendingMasochismPainReflectionLust = 0.f;
	LastCombatImpactTimeSeconds = -FLT_MAX;
	AppliedSecondBreathTransitionIds.Reset();
	ApplyPassiveAttributeEffects();
}

void UProjectSinfulAscensionComponent::ApplyPassiveAttributeEffects()
{
	ApplyDynamicMaximums();
	ApplyWillpowerStatusImmunities();
	ApplyFaithPassiveAttributeEffects();
}

void UProjectSinfulAscensionComponent::ApplyFaithPassiveAttributeEffects()
{
	if (!CombatAttributeComponent)
	{
		if (AActor* OwnerActor = GetOwner())
		{
			CombatAttributeComponent = OwnerActor->FindComponentByClass<UProjectCombatAttributeComponent>();
		}
	}

	if (!CombatAttributeComponent || !CombatAttributeComponent->HasAttribute(SpellDefenseName))
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float DesiredBonus = UProjectSinfulAscensionSettings::ComputeFaithPassiveBonus(
		GetAttributeLevel(EProjectSinAttribute::Faith),
		Settings ? Settings->FaithSpellDefensePerLevel : 1.f);
	if (FMath::IsNearlyEqual(AppliedFaithSpellDefenseBonus, DesiredBonus, 0.001f))
	{
		return;
	}

	const float BaseMaxValue = FMath::Max(0.f, CombatAttributeComponent->GetAttributeMaxValue(SpellDefenseName) - AppliedFaithSpellDefenseBonus);
	const float BaseCurrentValue = FMath::Max(0.f, CombatAttributeComponent->GetAttributeCurrentValue(SpellDefenseName) - AppliedFaithSpellDefenseBonus);
	CombatAttributeComponent->SetAttributeMaxValue(SpellDefenseName, BaseMaxValue + DesiredBonus, false);
	CombatAttributeComponent->SetAttributeCurrentValue(SpellDefenseName, BaseCurrentValue + DesiredBonus);
	AppliedFaithSpellDefenseBonus = DesiredBonus;
}

void UProjectSinfulAscensionComponent::UpdateFaithMadnessRecovery(const float DeltaTime)
{
	if (!HasMilestone(EProjectSinAttribute::Faith, 5) || DeltaTime <= 0.f)
	{
		return;
	}

	if (!NeedsComponent)
	{
		RefreshCachedComponents();
	}

	if (!NeedsComponent || !NeedsComponent->HasSensation(MadnessName))
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float RecoveryAmount = UProjectSinfulAscensionSettings::ComputeFaithMadnessRecovery(
		DeltaTime,
		Settings ? Settings->FaithMilestone5MadnessRecoveryPerSecond : 0.5f);
	if (RecoveryAmount <= KINDA_SMALL_NUMBER || GetSensationCurrent(MadnessName) <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	MaxSensationHoldEndTimeSeconds.Remove(MadnessName);
	SetForcedStatus(FrenzyStatusName, false);
	bFrenzyForced = false;
	NeedsComponent->ModifySensationValue(MadnessName, -RecoveryAmount, true);
}

void UProjectSinfulAscensionComponent::UpdateCombatState()
{
	if (!bCombatStateActive)
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	const float CombatExitSeconds = FMath::Max(Settings ? Settings->CombatStateExitSeconds : 8.f, 0.1f);
	if ((Now - LastCombatImpactTimeSeconds) > CombatExitSeconds)
	{
		bCombatStateActive = false;
	}
}

float UProjectSinfulAscensionComponent::CalculateDesiredMoveSpeedBonus() const
{
	return 0.f;
}

float UProjectSinfulAscensionComponent::ResolveBaseMoveSpeedForBonuses() const
{
	if (bHasCachedBaseMaxWalkSpeed)
	{
		return CachedBaseMaxWalkSpeed;
	}

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UCharacterMovementComponent* MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
	if (!MovementComponent)
	{
		return 0.f;
	}

	return FMath::Max(0.f, MovementComponent->MaxWalkSpeed - AppliedMoveSpeedBonus);
}

void UProjectSinfulAscensionComponent::UpdateActiveEmoteEffects(const float DeltaTime)
{
	if (!EmoteComponent || DeltaTime <= 0.f)
	{
		return;
	}

	if (!EmoteComponent->IsEmotePlaybackStarted())
	{
		return;
	}

	FProjectEmoteInteractionDefinition ActiveInteraction;
	if (!EmoteComponent->FindActiveInteractionDefinition(ActiveInteraction))
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const FProjectEmoteActionEffects& Effects = ActiveInteraction.Effects;
	const bool bUseDefaultDanceEffects = Effects.bUseDefaultDanceEffects || ActiveInteraction.InteractionId == DanceInteractionId;

	if (bUseDefaultDanceEffects)
	{
		const float LustMax = FMath::Max(GetSensationMax(LustName), Settings ? Settings->LustMax : 10000.f);
		const float DanceLustDelta = LustMax * (Settings ? Settings->DanceLustPercentOfMaxPerSecond : 0.05f) * DeltaTime;
		GrantSxpWithAttributeAffinityInternal(
			TEXT("Dance"),
			FMath::RoundToInt((Settings ? Settings->DanceSxpPerSecond : 200) * DeltaTime),
			true,
			{ EProjectSinAttribute::Allure, EProjectSinAttribute::Celerity },
			ESxpGrantSource::YMenuAction);
		GrantSensation(TEXT("Dance"), 0.f, DanceLustDelta, 0.f);
		return;
	}

	const FName ReasonId = Effects.SxpReasonId.IsNone() ? ActiveInteraction.InteractionId : Effects.SxpReasonId;
	if (!FMath::IsNearlyZero(Effects.SxpPerSecond))
	{
		GrantSxpInternal(
			ReasonId,
			FMath::RoundToInt(Effects.SxpPerSecond * DeltaTime),
			true,
			ESxpGrantSource::YMenuAction);
	}

	const float LustMax = FMath::Max(GetSensationMax(LustName), Settings ? Settings->LustMax : 10000.f);
	const float MadnessDelta = Effects.MadnessPerSecond * DeltaTime;
	const float LustDelta = (Effects.LustFlatPerSecond + (Effects.LustPercentOfMaxPerSecond * LustMax)) * DeltaTime;
	const float PainDelta = Effects.PainPerSecond * DeltaTime;
	if (!FMath::IsNearlyZero(MadnessDelta) || !FMath::IsNearlyZero(LustDelta) || !FMath::IsNearlyZero(PainDelta))
	{
		GrantSensation(ReasonId, MadnessDelta, LustDelta, PainDelta);
	}
}

void UProjectSinfulAscensionComponent::UpdateSensationDecay()
{
	if (!NeedsComponent)
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float IdleSeconds = Settings ? Settings->SensationDecayIdleSeconds : 60.f;
	const float DecayIntervalSeconds = Settings ? Settings->SensationDecayIntervalSeconds : 8.f;
	const float DecayRatio = Settings ? Settings->SensationDecayRatio : 0.05f;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	const TArray<FName> SensationNames = { MadnessName, LustName };

	for (const FName SensationName : SensationNames)
	{
		if (IsSensationMaxHoldActive(SensationName))
		{
			SetSensationCurrent(SensationName, GetSensationMax(SensationName));
			LastSensationDecayTimeSeconds.Add(SensationName, Now);
			continue;
		}

		const float LastGainTime = LastSensationGainTimeSeconds.FindRef(SensationName);
		const float LastDecayTime = LastSensationDecayTimeSeconds.FindRef(SensationName);
		if ((Now - LastGainTime) < IdleSeconds || (Now - LastDecayTime) < DecayIntervalSeconds)
		{
			continue;
		}

		const float CurrentValue = GetSensationCurrent(SensationName);
		if (CurrentValue <= KINDA_SMALL_NUMBER)
		{
			LastSensationDecayTimeSeconds.Add(SensationName, Now);
			continue;
		}

		SetSensationCurrent(SensationName, CurrentValue - (CurrentValue * DecayRatio));
		LastSensationDecayTimeSeconds.Add(SensationName, Now);
	}
}

void UProjectSinfulAscensionComponent::UpdateSensationConsequences(const float DeltaTime)
{
	UpdateSensationHoldState();

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float MadnessMax = GetSensationMax(MadnessName);
	const float LustMax = GetSensationMax(LustName);

	if (MadnessMax > KINDA_SMALL_NUMBER && GetSensationCurrent(MadnessName) >= MadnessMax - KINDA_SMALL_NUMBER)
	{
		StartMaxSensationHold(MadnessName, MadnessMax);
	}

	if (LustMax > KINDA_SMALL_NUMBER && GetSensationCurrent(LustName) >= LustMax - KINDA_SMALL_NUMBER)
	{
		StartMaxSensationHold(LustName, LustMax);
	}

	const bool bLustHeld = IsSensationMaxHoldActive(LustName);
	const bool bPainHeld = false;

	if (bFrenzyForced)
	{
		SetForcedStatus(FrenzyStatusName, false);
		bFrenzyForced = false;
	}

	const int32 MaxedCount = (bLustHeld ? 1 : 0) + (bPainHeld ? 1 : 0);
	if (MaxedCount >= 2 && !bOverloadBlackoutInProgress)
	{
		SensoryOverloadProgressSeconds += FMath::Max(0.f, DeltaTime);
		if (!bOverloadProgressBroadcastState)
		{
			bOverloadProgressBroadcastState = true;
			OnSensoryOverloadChanged.Broadcast(true, SensoryOverloadProgressSeconds);
		}

		if (SensoryOverloadProgressSeconds >= (Settings ? Settings->SensoryOverloadRequiredSeconds : 60.f))
		{
			bOverloadBlackoutInProgress = true;
			if (StatusComponent)
			{
				StatusComponent->ApplyStatus(ExhaustedStatusName, Settings ? Settings->SensoryOverloadBlackoutSeconds : 15.f, GetOwner());
				StatusComponent->TriggerExhaustionSequence(Settings ? Settings->SensoryOverloadBlackoutSeconds : 15.f);
			}
			OnSensoryOverloadChanged.Broadcast(true, SensoryOverloadProgressSeconds);
		}
	}
	else if (MaxedCount < 2 && !bOverloadBlackoutInProgress)
	{
		if (SensoryOverloadProgressSeconds > 0.f || bOverloadProgressBroadcastState)
		{
			SensoryOverloadProgressSeconds = 0.f;
			bOverloadProgressBroadcastState = false;
			OnSensoryOverloadChanged.Broadcast(false, SensoryOverloadProgressSeconds);
		}
	}
}

void UProjectSinfulAscensionComponent::UpdateSensationHoldState()
{
	if (MaxSensationHoldEndTimeSeconds.Num() == 0 || !NeedsComponent)
	{
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	TArray<FName> ExpiredSensationNames;

	for (const TPair<FName, float>& HoldPair : MaxSensationHoldEndTimeSeconds)
	{
		if (Now >= HoldPair.Value)
		{
			ExpiredSensationNames.Add(HoldPair.Key);
			continue;
		}

		SetSensationCurrent(HoldPair.Key, GetSensationMax(HoldPair.Key));
	}

	for (const FName SensationName : ExpiredSensationNames)
	{
		EndSensationHold(SensationName);
	}
}

void UProjectSinfulAscensionComponent::StartMaxSensationHold(const FName SensationName, const float MaxValue)
{
	if (SensationName.IsNone())
	{
		return;
	}

	if (IsSensationMaxHoldActive(SensationName))
	{
		SetSensationCurrent(SensationName, MaxValue);
		return;
	}

	RefreshMaxSensationHold(SensationName);
}

void UProjectSinfulAscensionComponent::RefreshMaxSensationHold(const FName SensationName)
{
	if (SensationName.IsNone())
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float HoldSeconds = Settings ? Settings->SensationMaxHoldSeconds : 10.f;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	const float MaxValue = GetSensationMax(SensationName);

	MaxSensationHoldEndTimeSeconds.Add(SensationName, Now + HoldSeconds);
	LastSensationGainTimeSeconds.Add(SensationName, Now + HoldSeconds);
	LastSensationDecayTimeSeconds.Add(SensationName, Now + HoldSeconds);
	SetSensationCurrent(SensationName, MaxValue);

	if (SensationName == LustName)
	{
		if (StatusComponent)
		{
			StatusComponent->ApplyStatus(OrgasmRushStatusName, HoldSeconds, GetOwner());
		}
	}
	else if (SensationName == PainName)
	{
		if (StatusComponent)
		{
			StatusComponent->ApplyStatus(ExtremePainStatusName, HoldSeconds, GetOwner());
		}
	}
}

void UProjectSinfulAscensionComponent::EndSensationHold(const FName SensationName)
{
	MaxSensationHoldEndTimeSeconds.Remove(SensationName);
	SetSensationCurrent(SensationName, 0.f);

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	LastSensationGainTimeSeconds.Add(SensationName, Now);
	LastSensationDecayTimeSeconds.Add(SensationName, Now);

	if (SensationName == LustName)
	{
		if (StatusComponent)
		{
			StatusComponent->ClearStatus(OrgasmRushStatusName);
		}
		CompleteMasochismPainRapture();
	}
	else if (SensationName == PainName)
	{
		if (StatusComponent)
		{
			StatusComponent->ClearStatus(ExtremePainStatusName);
		}

		if (LocomotionOverrideComponent && bPainForcedCrawl)
		{
			LocomotionOverrideComponent->SetCrawlModeEnabled(false);
			if (!bPainForcedWalkWasEnabled)
			{
				LocomotionOverrideComponent->SetWalkModeEnabled(false);
			}
		}

		bPainForcedCrawl = false;
	}
}

void UProjectSinfulAscensionComponent::MarkCombatImpact()
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	bCombatStateActive = true;
	LastCombatImpactTimeSeconds = Now;
}

bool UProjectSinfulAscensionComponent::IsMasochismPainRaptureActive() const
{
	return bMasochismPainRaptureActive;
}

bool UProjectSinfulAscensionComponent::IsOrgasmStateActive() const
{
	if (IsSensationMaxHoldActive(LustName))
	{
		return true;
	}

	if (StatusComponent && StatusComponent->IsStatusActive(OrgasmRushStatusName))
	{
		return true;
	}

	return GetSensationMax(LustName) > KINDA_SMALL_NUMBER
		&& GetSensationCurrent(LustName) >= GetSensationMax(LustName) - KINDA_SMALL_NUMBER;
}

bool UProjectSinfulAscensionComponent::TryStartOrRefreshMasochismPainRapture(const bool bRefreshHold)
{
	if (!HasMilestone(EProjectSinAttribute::Masochism, 5)
		|| GetSensationMax(PainName) <= KINDA_SMALL_NUMBER
		|| GetSensationCurrent(PainName) + KINDA_SMALL_NUMBER < GetSensationMax(PainName)
		|| !IsOrgasmStateActive())
	{
		return false;
	}

	if (bRefreshHold || !bMasochismPainRaptureActive)
	{
		RefreshMaxSensationHold(LustName);
	}

	if (!bMasochismPainRaptureActive)
	{
		if (!CombatAttributeComponent)
		{
			RefreshCachedComponents();
		}

		MasochismRaptureCapturedHealth = ResolveCurrentOwnerHealthForMasochismRapture();
		bMasochismPainRaptureActive = true;
		ApplyMasochismRaptureAcfImmortality(true);
		const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
		GrantSxp(
			TEXT("Masochism.PainRapture"),
			Settings ? Settings->MasochismPainRaptureActivationSxp : 2000,
			false);
		RegisterMasochismRaptureHitFeedback();
		RefreshMasochismRaptureHitFeedbackBindings(true);
		OnMilestoneTriggered.Broadcast(TEXT("PainRapture"), EProjectSinAttribute::Masochism, 5);
	}

	MaintainMasochismPainRapture();
	return true;
}

void UProjectSinfulAscensionComponent::MaintainMasochismPainRapture()
{
	if (!bMasochismPainRaptureActive)
	{
		return;
	}

	if (!IsOrgasmStateActive())
	{
		CompleteMasochismPainRapture();
		return;
	}

	if (!CombatAttributeComponent)
	{
		RefreshCachedComponents();
	}

	ApplyMasochismRaptureAcfImmortality(true);
	RefreshMasochismRaptureHitFeedbackBindings(false);
	if (CombatAttributeComponent && !CombatAttributeComponent->HealthAttributeName.IsNone())
	{
		const float CurrentHealth = CombatAttributeComponent->GetAttributeCurrentValue(CombatAttributeComponent->HealthAttributeName);
		if (CurrentHealth + KINDA_SMALL_NUMBER < MasochismRaptureCapturedHealth)
		{
			CombatAttributeComponent->SetAttributeCurrentValue(
				CombatAttributeComponent->HealthAttributeName,
				MasochismRaptureCapturedHealth);
		}
	}
}

void UProjectSinfulAscensionComponent::CompleteMasochismPainRapture()
{
	if (!bMasochismPainRaptureActive)
	{
		return;
	}

	const bool bPainOverflowUnlocked = HasMilestone(EProjectSinAttribute::Masochism, 10);
	EndMasochismPainRaptureState();

	if (bPainOverflowUnlocked)
	{
		SetSensationCurrent(PainName, 0.f);
		return;
	}

	if (!DefeatFlowComponent)
	{
		RefreshCachedComponents();
	}

	if (DefeatFlowComponent)
	{
		DefeatFlowComponent->RequestKnockoutOrPendingCrawl(
			EProjectKnockoutReason::MasochismRapture,
			TEXT("Masochism.PainRapture"));
	}
}

void UProjectSinfulAscensionComponent::EndMasochismPainRaptureState()
{
	ApplyMasochismRaptureAcfImmortality(false);
	UnbindMasochismRaptureHitFeedback();
	bMasochismPainRaptureActive = false;
	MasochismRaptureCapturedHealth = 0.f;
}

float UProjectSinfulAscensionComponent::ResolveCurrentOwnerHealthForMasochismRapture()
{
	if (!RealtimeSnapshotComponent)
	{
		BindToRealtimeSnapshotComponent();
	}

	float CurrentHealth = 0.f;
	float MaxHealth = 0.f;
	if (RealtimeSnapshotComponent && RealtimeSnapshotComponent->TryReadOwnerResource(HealthResourceName, CurrentHealth, MaxHealth))
	{
		return CurrentHealth;
	}

	if (!CombatAttributeComponent)
	{
		RefreshCachedComponents();
	}

	return CombatAttributeComponent && !CombatAttributeComponent->HealthAttributeName.IsNone()
		? CombatAttributeComponent->GetAttributeCurrentValue(CombatAttributeComponent->HealthAttributeName)
		: 0.f;
}

void UProjectSinfulAscensionComponent::RestoreMasochismRaptureHealthFloor()
{
	if (MasochismRaptureCapturedHealth <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	if (!RealtimeSnapshotComponent)
	{
		BindToRealtimeSnapshotComponent();
	}

	if (RealtimeSnapshotComponent)
	{
		RealtimeSnapshotComponent->SetOwnerResourceFloor(HealthResourceName, MasochismRaptureCapturedHealth);
	}

	if (!CombatAttributeComponent)
	{
		RefreshCachedComponents();
	}

	if (CombatAttributeComponent && !CombatAttributeComponent->HealthAttributeName.IsNone())
	{
		const float CurrentHealth = CombatAttributeComponent->GetAttributeCurrentValue(CombatAttributeComponent->HealthAttributeName);
		if (CurrentHealth + KINDA_SMALL_NUMBER < MasochismRaptureCapturedHealth)
		{
			CombatAttributeComponent->SetAttributeCurrentValue(
				CombatAttributeComponent->HealthAttributeName,
				MasochismRaptureCapturedHealth);
		}
	}
}

void UProjectSinfulAscensionComponent::ApplyMasochismRaptureAcfImmortality(const bool bEnabled)
{
	if (!AcfDamageHandlerComponent)
	{
		if (AActor* OwnerActor = GetOwner())
		{
			AcfDamageHandlerComponent = OwnerActor->FindComponentByClass<UACFDamageHandlerComponent>();
		}
	}

	if (!AcfDamageHandlerComponent)
	{
		return;
	}

	if (bEnabled)
	{
		if (!bMasochismRaptureAcfImmortalityApplied)
		{
			bMasochismRapturePreviousAcfImmortal = AcfDamageHandlerComponent->GetIsImmortal();
			bMasochismRaptureAcfImmortalityApplied = true;
		}

		if (!AcfDamageHandlerComponent->GetIsImmortal())
		{
			AcfDamageHandlerComponent->SetIsImmortal(true);
		}
		return;
	}

	if (!bMasochismRaptureAcfImmortalityApplied)
	{
		return;
	}

	AcfDamageHandlerComponent->SetIsImmortal(bMasochismRapturePreviousAcfImmortal);
	bMasochismRaptureAcfImmortalityApplied = false;
	bMasochismRapturePreviousAcfImmortal = false;
}

void UProjectSinfulAscensionComponent::RegisterMasochismRaptureHitFeedback()
{
	if (MasochismRaptureActorSpawnedHandle.IsValid())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		MasochismRaptureActorSpawnedHandle = World->AddOnActorSpawnedHandler(
			FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleMasochismRaptureActorSpawned));
	}
}

void UProjectSinfulAscensionComponent::RefreshMasochismRaptureHitFeedbackBindings(const bool bForce)
{
	if (!bMasochismPainRaptureActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float RefreshSeconds = FMath::Max(Settings ? Settings->MasochismRaptureHitFeedbackBindRefreshSeconds : 0.50f, 0.05f);
	const float Now = World->GetTimeSeconds();
	if (!bForce && Now + KINDA_SMALL_NUMBER < NextMasochismRaptureFeedbackBindingRefreshTimeSeconds)
	{
		return;
	}
	NextMasochismRaptureFeedbackBindingRefreshTimeSeconds = Now + RefreshSeconds;

	for (int32 Index = MasochismRaptureCollisionManagers.Num() - 1; Index >= 0; --Index)
	{
		if (!MasochismRaptureCollisionManagers[Index].IsValid())
		{
			MasochismRaptureCollisionManagers.RemoveAtSwap(Index);
		}
	}

	bool bBoundFromSnapshot = false;
	if (RealtimeSnapshotComponent)
	{
		const FProjectUnifiedRuntimeSnapshot Snapshot = RealtimeSnapshotComponent->BuildUnifiedRuntimeSnapshot();
		for (const FProjectRealtimeActorHealthSnapshot& EnemySnapshot : Snapshot.ObservedEnemies)
		{
			if (EnemySnapshot.Actor && EnemySnapshot.bRelevantEnemy)
			{
				BindMasochismRaptureCollisionManagersOnActor(EnemySnapshot.Actor);
				bBoundFromSnapshot = true;
			}
		}
	}

	if (bBoundFromSnapshot)
	{
		return;
	}

	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		if (Actor && Actor != GetOwner() && IsRelevantEnemyActor(Actor))
		{
			BindMasochismRaptureCollisionManagersOnActor(Actor);
		}
	}
}

void UProjectSinfulAscensionComponent::BindMasochismRaptureCollisionManagersOnActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	TArray<AActor*> ActorsToScan;
	ActorsToScan.Add(Actor);

	TArray<AActor*> AttachedActors;
	Actor->GetAttachedActors(AttachedActors, true);
	ActorsToScan.Append(AttachedActors);

	for (AActor* ActorToScan : ActorsToScan)
	{
		if (!ActorToScan)
		{
			continue;
		}

		TInlineComponentArray<UACMCollisionManagerComponent*> CollisionManagers;
		ActorToScan->GetComponents(CollisionManagers);
		for (UACMCollisionManagerComponent* CollisionManager : CollisionManagers)
		{
			if (!CollisionManager)
			{
				continue;
			}

			AActor* CollisionOwner = CollisionManager->GetActorOwner();
			if (CollisionOwner == GetOwner())
			{
				continue;
			}

			BindMasochismRaptureCollisionManager(CollisionManager);
		}
	}
}

void UProjectSinfulAscensionComponent::BindMasochismRaptureCollisionManager(UACMCollisionManagerComponent* CollisionManager)
{
	if (!CollisionManager)
	{
		return;
	}

	for (const TWeakObjectPtr<UACMCollisionManagerComponent>& BoundCollisionManager : MasochismRaptureCollisionManagers)
	{
		if (BoundCollisionManager.Get() == CollisionManager)
		{
			return;
		}
	}

	CollisionManager->OnActorDamaged.AddUniqueDynamic(this, &ThisClass::HandleMasochismRaptureAcfActorDamaged);
	CollisionManager->OnCollisionDetected.AddUniqueDynamic(this, &ThisClass::HandleMasochismRaptureAcfCollisionDetected);
	MasochismRaptureCollisionManagers.Add(CollisionManager);
}

void UProjectSinfulAscensionComponent::UnbindMasochismRaptureHitFeedback()
{
	for (const TWeakObjectPtr<UACMCollisionManagerComponent>& BoundCollisionManager : MasochismRaptureCollisionManagers)
	{
		if (UACMCollisionManagerComponent* CollisionManager = BoundCollisionManager.Get())
		{
			CollisionManager->OnActorDamaged.RemoveDynamic(this, &ThisClass::HandleMasochismRaptureAcfActorDamaged);
			CollisionManager->OnCollisionDetected.RemoveDynamic(this, &ThisClass::HandleMasochismRaptureAcfCollisionDetected);
		}
	}
	MasochismRaptureCollisionManagers.Reset();

	if (MasochismRaptureActorSpawnedHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->RemoveOnActorSpawnedHandler(MasochismRaptureActorSpawnedHandle);
		}
		MasochismRaptureActorSpawnedHandle.Reset();
	}

	NextMasochismRaptureFeedbackBindingRefreshTimeSeconds = -FLT_MAX;
	LastMasochismRaptureHitFeedbackTimeSeconds = -FLT_MAX;
}

void UProjectSinfulAscensionComponent::HandleMasochismRaptureActorSpawned(AActor* SpawnedActor)
{
	if (!bMasochismPainRaptureActive || !SpawnedActor || SpawnedActor == GetOwner())
	{
		return;
	}

	if (IsRelevantEnemyActor(SpawnedActor))
	{
		BindMasochismRaptureCollisionManagersOnActor(SpawnedActor);
		return;
	}

	TInlineComponentArray<UACMCollisionManagerComponent*> CollisionManagers;
	SpawnedActor->GetComponents(CollisionManagers);
	for (UACMCollisionManagerComponent* CollisionManager : CollisionManagers)
	{
		AActor* CollisionOwner = CollisionManager ? CollisionManager->GetActorOwner() : nullptr;
		if (CollisionOwner != GetOwner())
		{
			BindMasochismRaptureCollisionManager(CollisionManager);
		}
	}
}

void UProjectSinfulAscensionComponent::HandleMasochismRaptureAcfActorDamaged(AActor* DamageReceiver)
{
	if (!bMasochismPainRaptureActive || DamageReceiver != GetOwner())
	{
		return;
	}

	MarkCombatImpact();
	RestoreMasochismRaptureHealthFloor();
	TriggerMasochismRaptureHitFeedback(nullptr, nullptr);
}

void UProjectSinfulAscensionComponent::HandleMasochismRaptureAcfCollisionDetected(const FHitResult& HitResult)
{
	if (!bMasochismPainRaptureActive || HitResult.GetActor() != GetOwner())
	{
		return;
	}

	MarkCombatImpact();
	TriggerMasochismRaptureHitFeedback(nullptr, &HitResult);
}

void UProjectSinfulAscensionComponent::TriggerMasochismRaptureHitFeedback(AActor* SourceActor, const FHitResult* HitResult)
{
	if (!bMasochismPainRaptureActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float CooldownSeconds = FMath::Max(Settings ? Settings->MasochismRaptureHitFeedbackCooldownSeconds : 0.08f, 0.f);
	if (CooldownSeconds > 0.f && Now - LastMasochismRaptureHitFeedbackTimeSeconds < CooldownSeconds)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (!AcfEffectsManagerComponent)
	{
		AcfEffectsManagerComponent = OwnerActor->FindComponentByClass<UACFEffectsManagerComponent>();
	}

	if (!AcfEffectsManagerComponent)
	{
		return;
	}

	if (!SourceActor && RealtimeSnapshotComponent)
	{
		SourceActor = RealtimeSnapshotComponent->FindNearestRelevantEnemyToOwner();
	}

	FACFDamageEvent DamageEvent;
	DamageEvent.DamageDealer = SourceActor;
	DamageEvent.DamageReceiver = OwnerActor;
	DamageEvent.FinalDamage = 1.f;
	DamageEvent.contextString = TEXT("Masochism.PainRaptureFeedback");
	DamageEvent.DamageClass = ResolveMasochismRaptureFeedbackDamageClass(SourceActor);
	DamageEvent.HitResponseAction = UACFFunctionLibrary::GetDefaultHitState();

	if (HitResult)
	{
		DamageEvent.hitResult = *HitResult;
	}
	else
	{
		const FVector OwnerLocation = OwnerActor->GetActorLocation();
		const FVector SourceLocation = SourceActor ? SourceActor->GetActorLocation() : OwnerLocation - OwnerActor->GetActorForwardVector() * 100.f;
		const FVector HitDirection = (OwnerLocation - SourceLocation).GetSafeNormal();
		const FVector ImpactNormal = HitDirection.IsNearlyZero() ? -OwnerActor->GetActorForwardVector() : -HitDirection;

		FHitResult SyntheticHit;
		SyntheticHit.Location = OwnerLocation;
		SyntheticHit.ImpactPoint = OwnerLocation;
		SyntheticHit.TraceStart = SourceLocation;
		SyntheticHit.TraceEnd = OwnerLocation;
		SyntheticHit.Normal = ImpactNormal;
		SyntheticHit.ImpactNormal = ImpactNormal;
		SyntheticHit.Distance = FVector::Distance(SourceLocation, OwnerLocation);
		SyntheticHit.HitObjectHandle = FActorInstanceHandle(OwnerActor);
		DamageEvent.hitResult = SyntheticHit;
		DamageEvent.hitDirection = HitDirection;
	}

	if (DamageEvent.hitResult.BoneName.IsNone())
	{
		DamageEvent.hitResult.BoneName = TEXT("pelvis");
	}

	if (!DamageEvent.hitResult.Component.IsValid())
	{
		if (const ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor))
		{
			DamageEvent.hitResult.Component = OwnerCharacter->GetMesh();
		}
		else
		{
			DamageEvent.hitResult.Component = Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent());
		}
	}

	DamageEvent.PhysMaterial = DamageEvent.hitResult.PhysMaterial.Get();
	if (!DamageEvent.PhysMaterial)
	{
		if (const ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor))
		{
			if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
			{
				if (FBodyInstance* BodyInstance = Mesh->GetBodyInstance(DamageEvent.hitResult.BoneName))
				{
					DamageEvent.PhysMaterial = BodyInstance->GetSimplePhysicalMaterial();
					DamageEvent.hitResult.PhysMaterial = DamageEvent.PhysMaterial;
				}
			}
		}
	}

	FACFDamageEvent FeedbackEvent = DamageEvent;
	const bool bSuppressDamageText = Settings ? Settings->bMasochismRaptureSuppressDamageText : true;
	if (bSuppressDamageText)
	{
		FeedbackEvent.FinalDamage = 0.f;
		FeedbackEvent.DamageZone = EDamageZone::ENormal;
		FeedbackEvent.bIsCritical = false;
		FeedbackEvent.hitResult.BoneName = TEXT("pelvis");
	}

	if (OwnerActor->HasAuthority())
	{
		AcfEffectsManagerComponent->PlayHitReactionEffect(FeedbackEvent);
	}
	AcfEffectsManagerComponent->OnDamageImpactReceived(FeedbackEvent);
	PlayMasochismRaptureHitSound(DamageEvent);
	if (bSuppressDamageText)
	{
		QueueMasochismRaptureDamageTextSuppression();
	}
	LastMasochismRaptureHitFeedbackTimeSeconds = Now;
}

void UProjectSinfulAscensionComponent::QueueMasochismRaptureDamageTextSuppression()
{
	if (!bMasochismPainRaptureActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SuppressMasochismRaptureDamageTextWidgets();

	FTimerDelegate NextTickDelegate;
	NextTickDelegate.BindWeakLambda(this, [this]()
	{
		SuppressMasochismRaptureDamageTextWidgets();
	});
	World->GetTimerManager().SetTimerForNextTick(NextTickDelegate);

	FTimerHandle DelayedSuppressionHandle;
	FTimerDelegate DelayedDelegate;
	DelayedDelegate.BindWeakLambda(this, [this]()
	{
		SuppressMasochismRaptureDamageTextWidgets();
	});
	World->GetTimerManager().SetTimer(DelayedSuppressionHandle, DelayedDelegate, 0.05f, false);
}

void UProjectSinfulAscensionComponent::SuppressMasochismRaptureDamageTextWidgets()
{
	if (!bMasochismPainRaptureActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<UUserWidget*> DamageWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, DamageWidgets, UACFDamageWidget::StaticClass(), false);
	for (UUserWidget* DamageWidget : DamageWidgets)
	{
		if (!DamageWidget || DamageWidget->GetWorld() != World)
		{
			continue;
		}

		DamageWidget->SetVisibility(ESlateVisibility::Collapsed);
		DamageWidget->RemoveFromParent();
	}
}

void UProjectSinfulAscensionComponent::PlayMasochismRaptureHitSound(const FACFDamageEvent& DamageEvent)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World || !AcfEffectsManagerComponent)
	{
		return;
	}

	USoundBase* SoundToPlay = nullptr;
	UACMEffectsDispatcherComponent* EffectsDispatcher = nullptr;
	if (AGameStateBase* GameState = UGameplayStatics::GetGameState(World))
	{
		EffectsDispatcher = GameState->FindComponentByClass<UACMEffectsDispatcherComponent>();
	}

	FBaseFX HitFX;
	if (AcfEffectsManagerComponent->TryGetDamageFX(DamageEvent.HitResponseAction, DamageEvent.DamageClass, HitFX))
	{
		SoundToPlay = HitFX.ActionSound;
	}

	if (!SoundToPlay)
	{
		if (EffectsDispatcher)
		{
			FBaseFX ImpactFX;
			UPhysicalMaterial* ImpactMaterial = DamageEvent.hitResult.PhysMaterial.Get();
			if (!ImpactMaterial)
			{
				ImpactMaterial = DamageEvent.PhysMaterial;
			}
			if (!ImpactMaterial)
			{
				ImpactMaterial = LoadObject<UPhysicalMaterial>(
					nullptr,
					TEXT("/Engine/EngineMaterials/DefaultPhysicalMaterial.DefaultPhysicalMaterial"));
			}

			if (EffectsDispatcher->TryGetImpactFX(DamageEvent.DamageClass, ImpactMaterial, ImpactFX))
			{
				SoundToPlay = ImpactFX.ActionSound;
			}
		}
	}

	if (!SoundToPlay)
	{
		return;
	}

	const FName BoneName = DamageEvent.hitResult.BoneName.IsNone() ? FName(TEXT("pelvis")) : DamageEvent.hitResult.BoneName;
	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	if (OwnerActor->HasAuthority() && EffectsDispatcher && OwnerCharacter)
	{
		FActionEffect SoundEffect;
		SoundEffect.ActionSound = SoundToPlay;
		SoundEffect.SpawnLocation = ESpawnFXLocation::ESpawnAttachedToSocketOrBone;
		SoundEffect.SocketOrBoneName = BoneName;
		FComponentFX SpawnedComponents;
		EffectsDispatcher->PlayReplicatedActionEffect(SoundEffect, OwnerCharacter, SpawnedComponents);
		return;
	}

	if (OwnerCharacter)
	{
		if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
		{
			UGameplayStatics::SpawnSoundAttached(SoundToPlay, Mesh, BoneName);
			return;
		}
	}

	FVector SoundLocation = DamageEvent.hitResult.ImpactPoint;
	if (SoundLocation.IsNearlyZero())
	{
		SoundLocation = OwnerActor->GetActorLocation();
	}
	UGameplayStatics::SpawnSoundAtLocation(World, SoundToPlay, SoundLocation);
}

TSubclassOf<UACFDamageType> UProjectSinfulAscensionComponent::ResolveMasochismRaptureFeedbackDamageClass(AActor* SourceActor) const
{
	const FString SourceName = SourceActor ? SourceActor->GetName() + TEXT(" ") + SourceActor->GetClass()->GetName() : FString();
	if (SourceName.Contains(TEXT("Mage"), ESearchCase::IgnoreCase)
		|| SourceName.Contains(TEXT("Magic"), ESearchCase::IgnoreCase)
		|| SourceName.Contains(TEXT("Spell"), ESearchCase::IgnoreCase))
	{
		return USpellDamageType::StaticClass();
	}

	if (SourceName.Contains(TEXT("Ranged"), ESearchCase::IgnoreCase)
		|| SourceName.Contains(TEXT("Projectile"), ESearchCase::IgnoreCase)
		|| SourceName.Contains(TEXT("Arrow"), ESearchCase::IgnoreCase))
	{
		return LoadAcfFeedbackDamageClass(
			TEXT("/AscentCombatFramework/Blueprints/DamageTypes/ACFProjectileDamageType.ACFProjectileDamageType_C"),
			URangedDamageType::StaticClass());
	}

	if (SourceName.Contains(TEXT("Impact"), ESearchCase::IgnoreCase)
		|| SourceName.Contains(TEXT("Blunt"), ESearchCase::IgnoreCase)
		|| SourceName.Contains(TEXT("Unarmed"), ESearchCase::IgnoreCase)
		|| SourceName.Contains(TEXT("Fist"), ESearchCase::IgnoreCase))
	{
		return LoadAcfFeedbackDamageClass(
			TEXT("/AscentCombatFramework/Blueprints/DamageTypes/ACFImpactDamageType.ACFImpactDamageType_C"),
			UMeleeDamageType::StaticClass());
	}

	return LoadAcfFeedbackDamageClass(
		TEXT("/AscentCombatFramework/Blueprints/DamageTypes/ACFCutDamageType.ACFCutDamageType_C"),
		UMeleeDamageType::StaticClass());
}

void UProjectSinfulAscensionComponent::ApplyWillpowerStatusImmunities()
{
	if (!StatusComponent)
	{
		if (AActor* OwnerActor = GetOwner())
		{
			StatusComponent = OwnerActor->FindComponentByClass<UProjectSurvivalStatusComponent>();
			if (StatusComponent)
			{
				StatusComponent->OnBlackoutChanged.AddUniqueDynamic(this, &ThisClass::HandleBlackoutChanged);
			}
		}
	}

	if (!StatusComponent)
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	if (!HasMilestone(EProjectSinAttribute::Willpower, 10))
	{
		StatusComponent->ClearStatusImmunitySource(WillpowerImmunitySourceId);
		return;
	}

	const TArray<FName> DefaultImmunities = {
		TEXT("Fear"),
		TEXT("Dizzy")
	};
	StatusComponent->SetStatusImmunitySource(
		WillpowerImmunitySourceId,
		Settings && Settings->WillpowerImmunityStatusNames.Num() > 0
			? Settings->WillpowerImmunityStatusNames
			: DefaultImmunities);
}

bool UProjectSinfulAscensionComponent::ApplyWillpowerSecondBreathRecovery()
{
	if (!NeedsComponent)
	{
		return false;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const float RecoveryPct = Settings ? FMath::Clamp(Settings->WillpowerSecondBreathRecoveryPct, 0.f, 1.f) : 0.20f;
	if (RecoveryPct <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const TArray<FName> DefaultNeedNames = {
		HungerName,
		ThirstName,
		SleepName
	};
	const TArray<FName> DefaultSensationNames = {
		LustName,
		PainName,
		MadnessName
	};
	const TArray<FName>& NeedNames = Settings && Settings->WillpowerSecondBreathNeedNames.Num() > 0
		? Settings->WillpowerSecondBreathNeedNames
		: DefaultNeedNames;
	const TArray<FName>& SensationNames = Settings && Settings->WillpowerSecondBreathSensationNames.Num() > 0
		? Settings->WillpowerSecondBreathSensationNames
		: DefaultSensationNames;

	bool bTouchedAnyEntry = false;
	for (const FName NeedName : NeedNames)
	{
		if (NeedName.IsNone() || !NeedsComponent->HasNeed(NeedName))
		{
			continue;
		}

		const float RecoveryAmount = NeedsComponent->GetNeedMaxValue(NeedName) * RecoveryPct;
		NeedsComponent->ModifyNeedValue(NeedName, RecoveryAmount, true);
		bTouchedAnyEntry = true;
	}

	for (const FName SensationName : SensationNames)
	{
		if (SensationName.IsNone() || !NeedsComponent->HasSensation(SensationName))
		{
			continue;
		}

		const float RecoveryAmount = NeedsComponent->GetSensationMaxValue(SensationName) * RecoveryPct;
		MaxSensationHoldEndTimeSeconds.Remove(SensationName);
		NeedsComponent->ModifySensationValue(SensationName, -RecoveryAmount, true);
		bTouchedAnyEntry = true;
	}

	if (bTouchedAnyEntry)
	{
		UpdateSensationConsequences(0.f);
	}

	return bTouchedAnyEntry;
}

bool UProjectSinfulAscensionComponent::IsSensationMaxHoldActive(const FName SensationName) const
{
	const float* HoldEndTime = MaxSensationHoldEndTimeSeconds.Find(SensationName);
	if (!HoldEndTime)
	{
		return false;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	return Now < *HoldEndTime;
}

void UProjectSinfulAscensionComponent::UpdateMovementBonus()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
	if (!MovementComponent)
	{
		return;
	}

	const bool bLocomotionOverrideActive = LocomotionOverrideComponent && LocomotionOverrideComponent->IsWalkModeEnabled();
	if (!bHasCachedBaseMaxWalkSpeed && !bLocomotionOverrideActive)
	{
		CachedBaseMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
		bHasCachedBaseMaxWalkSpeed = true;
	}

	const float DesiredBonus = CalculateDesiredMoveSpeedBonus();
	if (FMath::IsNearlyEqual(DesiredBonus, AppliedMoveSpeedBonus))
	{
		return;
	}

	AppliedMoveSpeedBonus = DesiredBonus;

	if (bPainForcedCrawl || bLocomotionOverrideActive || !bHasCachedBaseMaxWalkSpeed)
	{
		return;
	}

	MovementComponent->MaxWalkSpeed = FMath::Max(0.f, CachedBaseMaxWalkSpeed + DesiredBonus);
}

void UProjectSinfulAscensionComponent::ApplyDynamicMaximums()
{
	if (!NeedsComponent)
	{
		if (AActor* OwnerActor = GetOwner())
		{
			NeedsComponent = OwnerActor->FindComponentByClass<UProjectSurvivalNeedsComponent>();
		}
	}

	if (!NeedsComponent)
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const UProjectSurvivalNeedsSettings* NeedsSettings = UProjectSurvivalNeedsSettings::Get();

	TSet<FName> NeedNames;
	TSet<FName> SensationNames;
	if (NeedsSettings)
	{
		for (const FProjectSurvivalNeedState& Need : NeedsSettings->DefaultNeeds)
		{
			if (!Need.NeedName.IsNone())
			{
				NeedNames.Add(Need.NeedName);
			}
		}

		for (const FProjectSurvivalSensationState& Sensation : NeedsSettings->DefaultSensations)
		{
			if (!Sensation.SensationName.IsNone())
			{
				SensationNames.Add(Sensation.SensationName);
			}
		}
	}

	for (const FProjectSurvivalNeedState& Need : NeedsComponent->Needs)
	{
		if (!Need.NeedName.IsNone())
		{
			NeedNames.Add(Need.NeedName);
			if (!RuntimeBaseNeedMaxByName.Contains(Need.NeedName))
			{
				RuntimeBaseNeedMaxByName.Add(Need.NeedName, FMath::Max(Need.MaxValue, 0.001f));
			}
		}
	}

	for (const FProjectSurvivalSensationState& Sensation : NeedsComponent->Sensations)
	{
		if (!Sensation.SensationName.IsNone())
		{
			SensationNames.Add(Sensation.SensationName);
			if (!RuntimeBaseSensationMaxByName.Contains(Sensation.SensationName))
			{
				RuntimeBaseSensationMaxByName.Add(Sensation.SensationName, FMath::Max(Sensation.MaxValue, 0.001f));
			}
		}
	}

	if (Settings)
	{
		for (const FProjectSinfulAscensionDynamicMaxRule& Rule : Settings->DynamicMaximumRules)
		{
			if (Rule.EntryName.IsNone())
			{
				continue;
			}

			if (Rule.bIsSensation)
			{
				SensationNames.Add(Rule.EntryName);
			}
			else
			{
				NeedNames.Add(Rule.EntryName);
			}
		}
	}

	for (const FName NeedName : NeedNames)
	{
		if (NeedsComponent->HasNeed(NeedName))
		{
			ApplyDynamicNeedMaximum(NeedName, CalculateDynamicMaximum(NeedName, false, ResolveBaseNeedMax(NeedName)));
		}
	}

	for (const FName SensationName : SensationNames)
	{
		if (NeedsComponent->HasSensation(SensationName))
		{
			ApplyDynamicSensationMaximum(SensationName, CalculateDynamicMaximum(SensationName, true, ResolveBaseSensationMax(SensationName)));
		}
	}
}

void UProjectSinfulAscensionComponent::ApplyDynamicNeedMaximum(const FName NeedName, const float NewMaxValue)
{
	if (!NeedsComponent || !NeedsComponent->HasNeed(NeedName))
	{
		return;
	}

	const float OldMaxValue = NeedsComponent->GetNeedMaxValue(NeedName);
	if (FMath::IsNearlyEqual(OldMaxValue, NewMaxValue))
	{
		return;
	}

	const float OldCurrentValue = NeedsComponent->GetNeedCurrentValue(NeedName);
	const float OldNormalizedValue = OldMaxValue > KINDA_SMALL_NUMBER
		? FMath::Clamp(OldCurrentValue / OldMaxValue, 0.f, 1.f)
		: 0.f;

	NeedsComponent->SetNeedMaxValue(NeedName, NewMaxValue, true);
	NeedsComponent->SetNeedCurrentValue(NeedName, NewMaxValue * OldNormalizedValue, true);
}

void UProjectSinfulAscensionComponent::ApplyDynamicSensationMaximum(const FName SensationName, const float NewMaxValue)
{
	if (!NeedsComponent || !NeedsComponent->HasSensation(SensationName))
	{
		return;
	}

	if (FMath::IsNearlyEqual(NeedsComponent->GetSensationMaxValue(SensationName), NewMaxValue))
	{
		return;
	}

	NeedsComponent->SetSensationMaxValue(SensationName, NewMaxValue, true);
}

float UProjectSinfulAscensionComponent::CalculateDynamicMaximum(const FName EntryName, const bool bIsSensation, const float BaseMax) const
{
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	if (!Settings)
	{
		return FMath::Max(BaseMax, 0.001f);
	}

	float FlatBonusTotal = 0.f;
	float PercentMultiplier = 1.f;
	for (const FProjectSinfulAscensionDynamicMaxRule& Rule : Settings->DynamicMaximumRules)
	{
		if (Rule.EntryName != EntryName || Rule.bIsSensation != bIsSensation || !IsAttributeIndexValid(Rule.Attribute))
		{
			continue;
		}

		UProjectSinfulAscensionSettings::AccumulateDynamicMaxRule(
			Rule,
			GetAttributeLevel(Rule.Attribute),
			FlatBonusTotal,
			PercentMultiplier);
	}

	return UProjectSinfulAscensionSettings::ComputeDynamicMaxValue(BaseMax, FlatBonusTotal, PercentMultiplier);
}

float UProjectSinfulAscensionComponent::ResolveBaseNeedMax(const FName NeedName) const
{
	const UProjectSurvivalNeedsSettings* NeedsSettings = UProjectSurvivalNeedsSettings::Get();
	if (NeedsSettings)
	{
		for (const FProjectSurvivalNeedState& Need : NeedsSettings->DefaultNeeds)
		{
			if (Need.NeedName == NeedName)
			{
				return FMath::Max(Need.MaxValue, 0.001f);
			}
		}
	}

	if (const float* RuntimeBaseMax = RuntimeBaseNeedMaxByName.Find(NeedName))
	{
		return FMath::Max(*RuntimeBaseMax, 0.001f);
	}

	return 100.f;
}

float UProjectSinfulAscensionComponent::ResolveBaseSensationMax(const FName SensationName) const
{
	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	if (SensationName == MadnessName)
	{
		return Settings ? FMath::Max(Settings->MadnessMax, 0.001f) : 100.f;
	}
	if (SensationName == PainName)
	{
		return Settings ? FMath::Max(Settings->PainMax, 0.001f) : 100.f;
	}
	if (SensationName == LustName)
	{
		return Settings ? FMath::Max(Settings->LustMax, 0.001f) : 10000.f;
	}

	const UProjectSurvivalNeedsSettings* NeedsSettings = UProjectSurvivalNeedsSettings::Get();
	if (NeedsSettings)
	{
		for (const FProjectSurvivalSensationState& Sensation : NeedsSettings->DefaultSensations)
		{
			if (Sensation.SensationName == SensationName)
			{
				return FMath::Max(Sensation.MaxValue, 0.001f);
			}
		}
	}

	if (const float* RuntimeBaseMax = RuntimeBaseSensationMaxByName.Find(SensationName))
	{
		return FMath::Max(*RuntimeBaseMax, 0.001f);
	}

	return 100.f;
}

void UProjectSinfulAscensionComponent::SetForcedStatus(const FName StatusName, const bool bActive)
{
	if (StatusComponent)
	{
		StatusComponent->SetForcedStatusActive(StatusName, bActive);
	}
}

float UProjectSinfulAscensionComponent::ApplyResourceDelta(const FName ResourceName, const float DeltaAmount)
{
	if (FMath::IsNearlyZero(DeltaAmount))
	{
		return 0.f;
	}

	if (!RealtimeSnapshotComponent)
	{
		BindToRealtimeSnapshotComponent();
	}

	if (RealtimeSnapshotComponent)
	{
		return RealtimeSnapshotComponent->ApplyOwnerResourceDelta(ResourceName, DeltaAmount, true);
	}

	if (!CombatAttributeComponent)
	{
		RefreshCachedComponents();
	}

	if (CombatAttributeComponent)
	{
		const FName CombatAttributeName = ResourceName == HealthResourceName
			? CombatAttributeComponent->HealthAttributeName
			: ResourceName;
		if (CombatAttributeComponent->HasAttribute(CombatAttributeName))
		{
			return CombatAttributeComponent->ModifyAttribute(CombatAttributeName, DeltaAmount);
		}
	}

	if (AActor* OwnerActor = GetOwner())
	{
		if (UARSStatisticsComponent* StatisticsComponent = OwnerActor->FindComponentByClass<UARSStatisticsComponent>())
		{
			const FGameplayTag StatisticTag = FGameplayTag::RequestGameplayTag(
				FName(*FString::Printf(TEXT("RPG.Statistics.%s"), *ResourceName.ToString())),
				false);
			if (StatisticTag.IsValid())
			{
				StatisticsComponent->ModifyStatistic(StatisticTag, DeltaAmount);
				return DeltaAmount;
			}
		}
	}

	return 0.f;
}

void UProjectSinfulAscensionComponent::ApplyHealthDelta(const float DeltaAmount)
{
	(void)ApplyResourceDelta(HealthResourceName, DeltaAmount);
}

void UProjectSinfulAscensionComponent::AddSensationGainTimestamp(const FName SensationName, const float AppliedDelta)
{
	if (AppliedDelta > KINDA_SMALL_NUMBER)
	{
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		LastSensationGainTimeSeconds.Add(SensationName, Now);
		LastSensationDecayTimeSeconds.Add(SensationName, Now);
	}
}

void UProjectSinfulAscensionComponent::AddMilestoneSnapshot(
	TArray<FProjectSinMilestoneState>& OutMilestones,
	const FName AbilityId,
	const EProjectSinAttribute Attribute,
	const int32 RequiredLevel) const
{
	FProjectSinMilestoneState State;
	State.AbilityId = AbilityId;
	State.Attribute = Attribute;
	State.RequiredLevel = RequiredLevel;
	State.bUnlocked = HasMilestone(Attribute, RequiredLevel);
	State.bOnCooldown = false;
	State.CooldownRemainingSeconds = 0.f;
	OutMilestones.Add(State);
}

void UProjectSinfulAscensionComponent::BroadcastMilestonesForLevelRange(
	const EProjectSinAttribute Attribute,
	const int32 OldLevel,
	const int32 NewLevel)
{
	if (NewLevel <= OldLevel)
	{
		return;
	}

	const UProjectSinfulAscensionSettings* Settings = UProjectSinfulAscensionSettings::Get();
	const int32 MilestoneFiveLevel = Attribute == EProjectSinAttribute::Celerity
		? FMath::Max(0, Settings ? Settings->CelerityYMenuActionSxpMilestoneLevel : 5)
		: 5;
	const int32 MilestoneTenLevel = Attribute == EProjectSinAttribute::Celerity
		? FMath::Max(0, Settings ? Settings->CelerityDeathLostSxpToMetaMilestoneLevel : 10)
		: 10;
	const FName MilestoneFiveAbilityId = Attribute == EProjectSinAttribute::Celerity
		? CelerityYMenuSurgeAbilityId
		: FName(*FString::Printf(TEXT("%s_%d"), *GetAttributeDisplayName(Attribute).ToString(), 5));
	const FName MilestoneTenAbilityId = Attribute == EProjectSinAttribute::Celerity
		? CelerityRecoveredMomentumAbilityId
		: FName(*FString::Printf(TEXT("%s_%d"), *GetAttributeDisplayName(Attribute).ToString(), 10));

	if (OldLevel < MilestoneFiveLevel && NewLevel >= MilestoneFiveLevel)
	{
		OnMilestoneTriggered.Broadcast(MilestoneFiveAbilityId, Attribute, MilestoneFiveLevel);
	}

	if (OldLevel < MilestoneTenLevel && NewLevel >= MilestoneTenLevel)
	{
		OnMilestoneTriggered.Broadcast(MilestoneTenAbilityId, Attribute, MilestoneTenLevel);
	}
}

float UProjectSinfulAscensionComponent::ResolveBestSXPGainMultiplier(const TArray<EProjectSinAttribute>& Affinities) const
{
	bool bFoundActiveMultiplier = false;
	float BestMultiplier = 1.0f;
	for (const EProjectSinAttribute Attribute : Affinities)
	{
		if (const float* Multiplier = SXPGainMultipliersByAttribute.Find(Attribute))
		{
			BestMultiplier = bFoundActiveMultiplier
				? FMath::Max(BestMultiplier, FMath::Max(0.0f, *Multiplier))
				: FMath::Max(0.0f, *Multiplier);
			bFoundActiveMultiplier = true;
		}
	}

	return BestMultiplier;
}

bool UProjectSinfulAscensionComponent::IsAttributeIndexValid(const EProjectSinAttribute Attribute) const
{
	const int32 Index = ToAttributeIndex(Attribute);
	return Index >= 0 && Index < ToAttributeIndex(EProjectSinAttribute::Count);
}

bool UProjectSinfulAscensionComponent::IsRelevantEnemyActor(const AActor* Actor) const
{
	const UProjectRealtimeSnapshotSettings* Settings = UProjectRealtimeSnapshotSettings::Get();
	return FProjectDefeatHitResolver::DoesActorMatchClassHintsRecursive(Actor, Settings ? Settings->RelevantEnemyClassNameHints : TArray<FString>());
}

bool UProjectSinfulAscensionComponent::IsQualifiedMaleEnemyActor(const AActor* Actor) const
{
	return FProjectDefeatHitResolver::DoesActorMatchClassHintsRecursive(Actor, ResolveQualifiedEnemyHints());
}

bool UProjectSinfulAscensionComponent::IsActorMatchingClassHints(const AActor* Actor, const TArray<FString>& Hints) const
{
	return FProjectDefeatHitResolver::DoesActorMatchClassHintsRecursive(Actor, Hints);
}

FText UProjectSinfulAscensionComponent::GetAttributeDisplayName(const EProjectSinAttribute Attribute) const
{
	switch (Attribute)
	{
	case EProjectSinAttribute::Willpower:
		return LOCTEXT("WillpowerAttribute", "Willpower");
	case EProjectSinAttribute::Sadism:
		return LOCTEXT("SadismAttribute", "Sadism");
	case EProjectSinAttribute::Masochism:
		return LOCTEXT("MasochismAttribute", "Masochism");
	case EProjectSinAttribute::Faith:
		return LOCTEXT("FaithAttribute", "Faith");
	case EProjectSinAttribute::Cunning:
		return LOCTEXT("CunningAttribute", "Cunning");
	case EProjectSinAttribute::Celerity:
		return LOCTEXT("CelerityAttribute", "Celerity");
	case EProjectSinAttribute::Allure:
		return LOCTEXT("AllureAttribute", "Allure");
	default:
		return LOCTEXT("UnknownAttribute", "Unknown");
	}
}

float UProjectSinfulAscensionComponent::GetSensationCurrent(const FName SensationName) const
{
	return NeedsComponent ? NeedsComponent->GetSensationCurrentValue(SensationName) : 0.f;
}

float UProjectSinfulAscensionComponent::GetSensationMax(const FName SensationName) const
{
	return NeedsComponent ? NeedsComponent->GetSensationMaxValue(SensationName) : 0.f;
}

void UProjectSinfulAscensionComponent::SetSensationCurrent(const FName SensationName, const float NewValue)
{
	if (NeedsComponent)
	{
		NeedsComponent->SetSensationCurrentValue(SensationName, NewValue, true);
	}
}

float UProjectSinfulAscensionComponent::GetNormalizedSensation(const FName SensationName) const
{
	const float MaxValue = FMath::Max(GetSensationMax(SensationName), 0.001f);
	return FMath::Clamp(GetSensationCurrent(SensationName) / MaxValue, 0.f, 1.f);
}

bool UProjectSinfulAscensionComponent::HasMilestone(const EProjectSinAttribute Attribute, const int32 Level) const
{
	if (Attribute == EProjectSinAttribute::Allure && (Level == 5 || Level == 10))
	{
		return false;
	}

	return GetAttributeLevel(Attribute) >= Level;
}

#undef LOCTEXT_NAMESPACE
