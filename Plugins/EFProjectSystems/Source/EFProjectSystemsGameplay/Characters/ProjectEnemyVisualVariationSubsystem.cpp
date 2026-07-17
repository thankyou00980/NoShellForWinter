#include "Characters/ProjectEnemyVisualVariationSubsystem.h"

#include "Characters/ProjectEnemyLevelComponent.h"
#include "Characters/ProjectEnemyVisualVariationSelection.h"
#include "Characters/ProjectEnemyVisualVariationSettings.h"
#include "Components/SkeletalMeshComponent.h"
#include "Combat/ProjectCombatAttributeComponent.h"
#include "EFCharacterCustomizationComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogProjectEnemyVisualVariation, Log, All);

namespace ProjectEnemyVisualVariationSubsystemPrivate
{
	static TObjectKey<UObject> MakeObjectKey(const UObject* Object)
	{
		return TObjectKey<UObject>(const_cast<UObject*>(Object));
	}

	static TObjectKey<APawn> MakePawnKey(const APawn* Pawn)
	{
		return TObjectKey<APawn>(const_cast<APawn*>(Pawn));
	}

	static FString NormalizeMorphName(const FString& Value)
	{
		FString Normalized;
		Normalized.Reserve(Value.Len());
		for (const TCHAR Character : Value)
		{
			if (FChar::IsAlnum(Character))
			{
				Normalized.AppendChar(FChar::ToLower(Character));
			}
		}

		return Normalized;
	}

	static void AddMorphNameCandidate(TArray<FName>& Candidates, const FName Candidate)
	{
		if (!Candidate.IsNone())
		{
			Candidates.AddUnique(Candidate);
		}
	}

	static TArray<FName> BuildMorphNameCandidates(const FName PrimaryMorphName)
	{
		TArray<FName> Candidates;
		AddMorphNameCandidate(Candidates, PrimaryMorphName);

		const FString MorphNameString = PrimaryMorphName.ToString();
		if (MorphNameString.Contains(TEXT("Flacid"), ESearchCase::IgnoreCase))
		{
			AddMorphNameCandidate(Candidates, FName(*MorphNameString.Replace(TEXT("Flacid"), TEXT("Flaccid"), ESearchCase::IgnoreCase)));
		}

		if (MorphNameString.Contains(TEXT("Flaccid"), ESearchCase::IgnoreCase))
		{
			AddMorphNameCandidate(Candidates, FName(*MorphNameString.Replace(TEXT("Flaccid"), TEXT("Flacid"), ESearchCase::IgnoreCase)));
		}

		return Candidates;
	}

	static bool CombinedMaterialNameContainsHint(const FString& CombinedMaterialName, const FString& Hint)
	{
		return !Hint.IsEmpty() && CombinedMaterialName.Contains(Hint, ESearchCase::IgnoreCase);
	}

	static void ApplyNativeSkinColorToPawnMeshes(
		APawn* Pawn,
		const UProjectEnemyVisualVariationSettings& Settings,
		const FLinearColor& SkinColor)
	{
		if (!IsValid(Pawn)
			|| !Settings.bApplySkinColorNatively
			|| Settings.EnemySkinColorParameterNames.IsEmpty()
			|| Settings.EnemySkinMaterialHints.IsEmpty())
		{
			return;
		}

		TInlineComponentArray<USkeletalMeshComponent*> MeshComponents(Pawn);
		Pawn->GetComponents(MeshComponents);

		for (USkeletalMeshComponent* MeshComponent : MeshComponents)
		{
			if (!IsValid(MeshComponent))
			{
				continue;
			}

			const TArray<FName> MaterialSlotNames = MeshComponent->GetMaterialSlotNames();
			for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
			{
				const FString SlotName = MaterialSlotNames.IsValidIndex(MaterialIndex) ? MaterialSlotNames[MaterialIndex].ToString() : FString();
				UMaterialInterface* MaterialInterface = MeshComponent->GetMaterial(MaterialIndex);
				const FString MaterialName = IsValid(MaterialInterface) ? MaterialInterface->GetName() : FString();
				const FString MaterialPath = IsValid(MaterialInterface) ? MaterialInterface->GetPathName() : FString();
				const FString CombinedMaterialName = FString::Printf(TEXT("%s %s %s"), *SlotName, *MaterialName, *MaterialPath);

				const bool bMatchesSkinMaterial = Settings.EnemySkinMaterialHints.ContainsByPredicate(
					[&CombinedMaterialName](const FString& Hint)
					{
						return CombinedMaterialNameContainsHint(CombinedMaterialName, Hint);
					});
				if (!bMatchesSkinMaterial)
				{
					continue;
				}

				if (UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex))
				{
					for (const FName ParameterName : Settings.EnemySkinColorParameterNames)
					{
						if (!ParameterName.IsNone())
						{
							DynamicMaterial->SetVectorParameterValue(ParameterName, SkinColor);
						}
					}
				}
			}
		}
	}
}

void UProjectEnemyVisualVariationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TargetEnemyClasses.Reset();
	MaleMorphTargetEnemyClasses.Reset();
	AllowedMorphNameSet.Reset();
	ProcessedActors.Reset();
	PendingActors.Reset();
	TrackedArousalStates.Reset();
	bInitialPawnScanPending = true;
	LastSightPollTimeSeconds = -1.0;

	const UProjectEnemyVisualVariationSettings* Settings = UProjectEnemyVisualVariationSettings::Get();
	for (const TSoftClassPtr<APawn>& TargetEnemyClass : Settings->TargetEnemyClasses)
	{
		if (UClass* ResolvedClass = TargetEnemyClass.LoadSynchronous())
		{
			TargetEnemyClasses.AddUnique(ResolvedClass);
		}
		else
		{
			UE_LOG(
				LogProjectEnemyVisualVariation,
				Warning,
				TEXT("Could not resolve target enemy class '%s'."),
				*TargetEnemyClass.ToSoftObjectPath().ToString());
		}
	}

	for (const TSoftClassPtr<APawn>& TargetEnemyClass : Settings->MaleMorphTargetEnemyClasses)
	{
		if (UClass* ResolvedClass = TargetEnemyClass.LoadSynchronous())
		{
			MaleMorphTargetEnemyClasses.AddUnique(ResolvedClass);
		}
		else
		{
			UE_LOG(
				LogProjectEnemyVisualVariation,
				Warning,
				TEXT("Could not resolve male morph target enemy class '%s'."),
				*TargetEnemyClass.ToSoftObjectPath().ToString());
		}
	}

	for (const FName MorphName : Settings->AllowedMorphNames)
	{
		if (!MorphName.IsNone())
		{
			AllowedMorphNameSet.Add(MorphName);
		}
	}

	if (UWorld* World = GetWorld(); IsValid(World) && World->IsGameWorld())
	{
		ActorSpawnedHandle = World->AddOnActorSpawnedHandler(
			FOnActorSpawned::FDelegate::CreateUObject(this, &UProjectEnemyVisualVariationSubsystem::HandleActorSpawned));
	}
}

void UProjectEnemyVisualVariationSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld(); IsValid(World) && ActorSpawnedHandle.IsValid())
	{
		World->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
	}

	ActorSpawnedHandle.Reset();
	TargetEnemyClasses.Reset();
	MaleMorphTargetEnemyClasses.Reset();
	AllowedMorphNameSet.Reset();
	ProcessedActors.Reset();
	PendingActors.Reset();
	TrackedArousalStates.Reset();
	bInitialPawnScanPending = false;
	LastSightPollTimeSeconds = -1.0;

	Super::Deinitialize();
}

void UProjectEnemyVisualVariationSubsystem::Tick(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !World->IsGameWorld())
	{
		return;
	}

	if (bInitialPawnScanPending)
	{
		ProcessExistingPawns();
		bInitialPawnScanPending = false;
	}

	CleanupTrackedArousalStates();

	const UProjectEnemyVisualVariationSettings* Settings = UProjectEnemyVisualVariationSettings::Get();
	if (!Settings || !Settings->bEnableSightDrivenArousalMorphs || TrackedArousalStates.IsEmpty())
	{
		return;
	}

	const double CurrentTimeSeconds = World->GetTimeSeconds();
	const double SightPollIntervalSeconds = FMath::Max(static_cast<double>(Settings->SightCheckIntervalSeconds), 0.01);
	if (LastSightPollTimeSeconds < 0.0 || (CurrentTimeSeconds - LastSightPollTimeSeconds) >= SightPollIntervalSeconds)
	{
		UpdateSightTriggeredArousal(static_cast<float>(CurrentTimeSeconds));
		LastSightPollTimeSeconds = CurrentTimeSeconds;
	}

	AdvanceArousalMorphs(DeltaTime);
}

TStatId UProjectEnemyVisualVariationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UProjectEnemyVisualVariationSubsystem, STATGROUP_Tickables);
}

bool UProjectEnemyVisualVariationSubsystem::IsTickable() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && World->IsGameWorld() && (bInitialPawnScanPending || TrackedArousalStates.Num() > 0);
}

bool UProjectEnemyVisualVariationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UProjectEnemyVisualVariationSubsystem::ProcessExistingPawns()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		if (ShouldProcessPawn(*It))
		{
			QueueVariationApplication(*It, 0);
		}
	}
}

void UProjectEnemyVisualVariationSubsystem::HandleActorSpawned(AActor* SpawnedActor)
{
	APawn* SpawnedPawn = Cast<APawn>(SpawnedActor);
	if (!ShouldProcessPawn(SpawnedPawn))
	{
		return;
	}

	QueueVariationApplication(SpawnedPawn, 0);
}

void UProjectEnemyVisualVariationSubsystem::QueueVariationApplication(APawn* Pawn, const int32 AttemptIndex)
{
	if (!IsValid(Pawn) || IsActorProcessed(Pawn) || IsActorPending(Pawn))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	MarkActorPending(Pawn);
	World->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &UProjectEnemyVisualVariationSubsystem::TryApplyVariation, TWeakObjectPtr<APawn>(Pawn), AttemptIndex));
}

void UProjectEnemyVisualVariationSubsystem::TryApplyVariation(TWeakObjectPtr<APawn> PawnPtr, const int32 AttemptIndex)
{
	APawn* Pawn = PawnPtr.Get();
	if (!IsValid(Pawn))
	{
		return;
	}

	ClearActorPending(Pawn);

	if (IsActorProcessed(Pawn) || !ShouldProcessPawn(Pawn))
	{
		return;
	}

	const UProjectEnemyVisualVariationSettings* Settings = UProjectEnemyVisualVariationSettings::Get();
	const int32 MaxRetryCount = FMath::Max(Settings->InitializationRetryCount, 0);

	auto RetryOrFail = [this, Pawn, AttemptIndex, MaxRetryCount](const FString& FailureReason)
	{
		if (AttemptIndex < MaxRetryCount)
		{
			QueueVariationApplication(Pawn, AttemptIndex + 1);
			return;
		}

		UE_LOG(
			LogProjectEnemyVisualVariation,
			Warning,
			TEXT("Enemy visual variation skipped for %s after %d attempts: %s"),
			*GetNameSafe(Pawn),
			AttemptIndex + 1,
			*FailureReason);

		MarkActorProcessed(Pawn);
	};

	UEFCharacterCustomizationComponent* CustomizationComponent = FindOrCreateCustomizationComponent(Pawn);
	if (!IsValid(CustomizationComponent))
	{
		RetryOrFail(TEXT("Could not create or find UEFCharacterCustomizationComponent."));
		return;
	}

	FString CompatibilityFailureReason;
	if (!CustomizationComponent->EvaluateCompatibilityForActor(Pawn, CompatibilityFailureReason))
	{
		RetryOrFail(CompatibilityFailureReason.IsEmpty()
			? TEXT("The spawned pawn is not yet compatible with the customization runtime.")
			: CompatibilityFailureReason);
		return;
	}

	TArray<FMorphSliderEntry> AllowedEntries;
	if (!GatherAllowedMorphEntries(CustomizationComponent, AllowedEntries))
	{
		RetryOrFail(TEXT("None of the allowed test morphs were discovered on the spawned pawn."));
		return;
	}

	for (const FMorphSliderEntry& Entry : AllowedEntries)
	{
		CustomizationComponent->ApplyMorph(Entry, 0.0f);
	}

	FProjectEnemyMorphSelectionContext SelectionContext;
	if (const UProjectEnemyLevelComponent* LevelComponent = Pawn->FindComponentByClass<UProjectEnemyLevelComponent>())
	{
		if (LevelComponent->HasAssignedLevel())
		{
			SelectionContext.bHasNormalizedEnemyLevel = true;
			SelectionContext.NormalizedEnemyLevel = LevelComponent->GetNormalizedLevel();
		}
	}

	FProjectEnemyMorphRollResult RollResult;
	if (!FProjectEnemyVisualVariationSelection::RollMorphVariation(AllowedEntries, *Settings, SelectionContext, RollResult)
		|| !RollResult.bIsValid
		|| !AllowedEntries.IsValidIndex(RollResult.SelectedEntryIndex))
	{
		RetryOrFail(TEXT("Could not roll a valid morph variation."));
		return;
	}

	const FMorphSliderEntry& SelectedEntry = AllowedEntries[RollResult.SelectedEntryIndex];
	CustomizationComponent->ApplyMorph(SelectedEntry, RollResult.MorphValue);
	const bool bIsMaleMorphTarget = IsMaleMorphTargetClass(Pawn->GetClass());
	if (bIsMaleMorphTarget)
	{
		ApplyConfiguredMaleMorphGroups(Pawn, CustomizationComponent);
	}

	const float SkinBrightnessMin = FMath::Min(Settings->SkinBrightnessMin, Settings->SkinBrightnessMax);
	const float SkinBrightnessMax = FMath::Max(Settings->SkinBrightnessMin, Settings->SkinBrightnessMax);
	const float SkinBrightness = FMath::FRandRange(SkinBrightnessMin, SkinBrightnessMax);
	const FLinearColor SkinColor(SkinBrightness, SkinBrightness, SkinBrightness, 1.0f);
	CustomizationComponent->SetSkinColor(SkinColor);
	ProjectEnemyVisualVariationSubsystemPrivate::ApplyNativeSkinColorToPawnMeshes(Pawn, *Settings, SkinColor);

	if (bIsMaleMorphTarget)
	{
		InitializeArousalMorphState(Pawn, CustomizationComponent);
	}
	MarkActorProcessed(Pawn);
}

bool UProjectEnemyVisualVariationSubsystem::ShouldProcessPawn(const APawn* Pawn) const
{
	if (!IsValid(Pawn))
	{
		return false;
	}

	if (Pawn->IsTemplate() || Pawn->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return false;
	}

	if (Pawn->GetNetMode() == NM_Client)
	{
		return false;
	}

	if (!IsTargetEnemyClass(Pawn->GetClass()))
	{
		return false;
	}

	return true;
}

bool UProjectEnemyVisualVariationSubsystem::IsTargetEnemyClass(const UClass* ActorClass) const
{
	if (!IsValid(ActorClass))
	{
		return false;
	}

	for (const TSubclassOf<APawn>& TargetEnemyClass : TargetEnemyClasses)
	{
		if (TargetEnemyClass && ActorClass == TargetEnemyClass.Get())
		{
			return true;
		}
	}

	return false;
}

bool UProjectEnemyVisualVariationSubsystem::IsMaleMorphTargetClass(const UClass* ActorClass) const
{
	if (!IsValid(ActorClass))
	{
		return false;
	}

	for (const TSubclassOf<APawn>& TargetEnemyClass : MaleMorphTargetEnemyClasses)
	{
		if (TargetEnemyClass && ActorClass == TargetEnemyClass.Get())
		{
			return true;
		}
	}

	return false;
}

UEFCharacterCustomizationComponent* UProjectEnemyVisualVariationSubsystem::FindOrCreateCustomizationComponent(APawn* Pawn) const
{
	if (!IsValid(Pawn))
	{
		return nullptr;
	}

	if (UEFCharacterCustomizationComponent* ExistingComponent = Pawn->FindComponentByClass<UEFCharacterCustomizationComponent>())
	{
		if (!ExistingComponent->IsRegistered())
		{
			ExistingComponent->RegisterComponent();
		}

		return ExistingComponent;
	}

	UEFCharacterCustomizationComponent* NewComponent = NewObject<UEFCharacterCustomizationComponent>(Pawn, TEXT("CharacterCustomizationComponent"));
	if (!IsValid(NewComponent))
	{
		return nullptr;
	}

	Pawn->AddInstanceComponent(NewComponent);
	NewComponent->RegisterComponent();
	return NewComponent;
}

bool UProjectEnemyVisualVariationSubsystem::GatherAllowedMorphEntries(
	const UEFCharacterCustomizationComponent* CustomizationComponent,
	TArray<FMorphSliderEntry>& OutEntries) const
{
	OutEntries.Reset();

	if (!IsValid(CustomizationComponent))
	{
		return false;
	}

	for (const FMorphSliderEntry& Entry : CustomizationComponent->GetAvailableMorphEntries())
	{
		if (AllowedMorphNameSet.Contains(Entry.MorphName))
		{
			OutEntries.Add(Entry);
		}
	}

	return OutEntries.Num() > 0;
}

bool UProjectEnemyVisualVariationSubsystem::GatherMorphEntriesForConfiguredName(
	const UEFCharacterCustomizationComponent* CustomizationComponent,
	const FName MorphName,
	TArray<FMorphSliderEntry>& OutEntries) const
{
	OutEntries.Reset();

	if (!IsValid(CustomizationComponent) || MorphName.IsNone())
	{
		return false;
	}

	const TArray<FName> CandidateNames = ProjectEnemyVisualVariationSubsystemPrivate::BuildMorphNameCandidates(MorphName);
	TSet<FString> NormalizedCandidates;
	for (const FName CandidateName : CandidateNames)
	{
		NormalizedCandidates.Add(ProjectEnemyVisualVariationSubsystemPrivate::NormalizeMorphName(CandidateName.ToString()));
	}

	for (const FMorphSliderEntry& Entry : CustomizationComponent->GetAvailableMorphEntries())
	{
		if (CandidateNames.Contains(Entry.MorphName)
			|| NormalizedCandidates.Contains(ProjectEnemyVisualVariationSubsystemPrivate::NormalizeMorphName(Entry.MorphName.ToString())))
		{
			OutEntries.Add(Entry);
		}
	}

	return OutEntries.Num() > 0;
}

void UProjectEnemyVisualVariationSubsystem::ApplyConfiguredMaleMorphGroups(
	APawn* Pawn,
	UEFCharacterCustomizationComponent* CustomizationComponent) const
{
	const UProjectEnemyVisualVariationSettings* Settings = UProjectEnemyVisualVariationSettings::Get();
	if (!Settings || !IsValid(Pawn) || !IsValid(CustomizationComponent))
	{
		return;
	}

	const int32 EnemyLevel = ResolveEnemyLevel(Pawn);

	auto GatherConfiguredMorphEntries =
		[this, CustomizationComponent](const TArray<FName>& MorphNames, TMap<FName, TArray<FMorphSliderEntry>>& OutEntriesByMorph, TArray<FName>& OutAvailableMorphNames)
	{
		OutEntriesByMorph.Reset();
		OutAvailableMorphNames.Reset();

		for (const FName MorphName : MorphNames)
		{
			TArray<FMorphSliderEntry> MatchingEntries;
			if (GatherMorphEntriesForConfiguredName(CustomizationComponent, MorphName, MatchingEntries))
			{
				OutAvailableMorphNames.Add(MorphName);
				OutEntriesByMorph.Add(MorphName, MoveTemp(MatchingEntries));
			}
		}
	};

	TArray<FName> GroupOneConfiguredNames;
	GroupOneConfiguredNames.Reserve(Settings->GroupOneMorphEntries.Num());
	for (const FProjectEnemyLevelBiasedMorphEntry& GroupOneEntry : Settings->GroupOneMorphEntries)
	{
		if (!GroupOneEntry.MorphName.IsNone())
		{
			GroupOneConfiguredNames.AddUnique(GroupOneEntry.MorphName);
		}
	}

	TMap<FName, TArray<FMorphSliderEntry>> GroupOneEntriesByMorph;
	TArray<FName> GroupOneAvailableMorphNames;
	GatherConfiguredMorphEntries(GroupOneConfiguredNames, GroupOneEntriesByMorph, GroupOneAvailableMorphNames);

	FProjectEnemyNamedMorphRollResult GroupOneRollResult;
	if (FProjectEnemyVisualVariationSelection::RollLevelBiasedPositiveMorph(
		GroupOneAvailableMorphNames,
		Settings->GroupOneMorphEntries,
		EnemyLevel,
		GroupOneRollResult))
	{
		if (const TArray<FMorphSliderEntry>* SelectedEntries = GroupOneEntriesByMorph.Find(GroupOneRollResult.MorphName))
		{
			ApplyMorphEntriesWithValue(CustomizationComponent, *SelectedEntries, GroupOneRollResult.MorphValue);
		}
	}

	TMap<FName, TArray<FMorphSliderEntry>> GroupTwoEntriesByMorph;
	TArray<FName> GroupTwoAvailableMorphNames;
	GatherConfiguredMorphEntries(Settings->GroupTwoMorphNames, GroupTwoEntriesByMorph, GroupTwoAvailableMorphNames);

	FProjectEnemyNamedMorphRollResult GroupTwoRollResult;
	if (FProjectEnemyVisualVariationSelection::RollBandBiasedPositiveMorph(
		GroupTwoAvailableMorphNames,
		Settings->GroupTwoHighValueChance,
		Settings->GroupTwoHighValueMin,
		Settings->GroupTwoHighValueMax,
		Settings->GroupTwoLowValueMin,
		Settings->GroupTwoLowValueMax,
		GroupTwoRollResult))
	{
		if (const TArray<FMorphSliderEntry>* SelectedEntries = GroupTwoEntriesByMorph.Find(GroupTwoRollResult.MorphName))
		{
			ApplyMorphEntriesWithValue(CustomizationComponent, *SelectedEntries, GroupTwoRollResult.MorphValue);
		}
	}

	const float GroupThreeValue = FProjectEnemyVisualVariationSelection::EvaluateLinearLevelAlpha(
		EnemyLevel,
		Settings->GroupThreeStartLevel,
		Settings->GroupThreeMaxLevel);

	TArray<FMorphSliderEntry> GroupThreeFixedEntries;
	if (GatherMorphEntriesForConfiguredName(CustomizationComponent, Settings->GroupThreeFixedMorphName, GroupThreeFixedEntries))
	{
		ApplyMorphEntriesWithValue(CustomizationComponent, GroupThreeFixedEntries, GroupThreeValue);
	}

	TArray<FMorphSliderEntry> GroupThreeOptionalEntries;
	if (GatherMorphEntriesForConfiguredName(CustomizationComponent, Settings->GroupThreeOptionalMorphName, GroupThreeOptionalEntries))
	{
		const bool bApplyOptionalMorph = EnemyLevel >= Settings->GroupThreeStartLevel
			&& FMath::FRand() <= FMath::Clamp(Settings->GroupThreeOptionalMorphChance, 0.0f, 1.0f);
		ApplyMorphEntriesWithValue(CustomizationComponent, GroupThreeOptionalEntries, bApplyOptionalMorph ? GroupThreeValue : 0.0f);
	}
}

int32 UProjectEnemyVisualVariationSubsystem::ResolveEnemyLevel(const APawn* Pawn) const
{
	if (const UProjectEnemyLevelComponent* LevelComponent = IsValid(Pawn) ? Pawn->FindComponentByClass<UProjectEnemyLevelComponent>() : nullptr)
	{
		if (LevelComponent->HasAssignedLevel())
		{
			return FMath::Max(LevelComponent->GetAssignedLevel(), 1);
		}
	}

	return 1;
}

bool UProjectEnemyVisualVariationSubsystem::ApplyMorphEntriesWithValue(
	UEFCharacterCustomizationComponent* CustomizationComponent,
	const TArray<FMorphSliderEntry>& Entries,
	const float Value) const
{
	if (!IsValid(CustomizationComponent) || Entries.IsEmpty())
	{
		return false;
	}

	bool bAppliedAnyMorph = false;
	for (const FMorphSliderEntry& Entry : Entries)
	{
		const float EffectiveMinValue = FMath::Min(Entry.MinValue, Entry.MaxValue);
		const float EffectiveMaxValue = FMath::Max(Entry.MinValue, Entry.MaxValue);
		CustomizationComponent->ApplyMorph(Entry, FMath::Clamp(Value, EffectiveMinValue, EffectiveMaxValue));
		bAppliedAnyMorph = true;
	}

	return bAppliedAnyMorph;
}

void UProjectEnemyVisualVariationSubsystem::InitializeArousalMorphState(APawn* Pawn, UEFCharacterCustomizationComponent* CustomizationComponent)
{
	const UProjectEnemyVisualVariationSettings* Settings = UProjectEnemyVisualVariationSettings::Get();
	if (!Settings || !Settings->bEnableSightDrivenArousalMorphs || !IsValid(Pawn) || !IsValid(CustomizationComponent))
	{
		return;
	}

	FProjectEnemyArousalMorphState MorphState;
	MorphState.Pawn = Pawn;
	MorphState.CustomizationComponent = CustomizationComponent;
	MorphState.CombatComponent = Pawn->FindComponentByClass<UProjectCombatAttributeComponent>();
	MorphState.CurrentArousalAlpha = 0.0f;
	MorphState.bShouldBeAroused = false;

	const bool bFoundFlaccidMorph = GatherMorphEntriesForConfiguredName(CustomizationComponent, Settings->FlaccidMorphName, MorphState.FlaccidEntries);
	const bool bFoundErectionMorph = GatherMorphEntriesForConfiguredName(CustomizationComponent, Settings->ErectionMorphName, MorphState.ErectionEntries);
	if (!bFoundFlaccidMorph && !bFoundErectionMorph)
	{
		UE_LOG(
			LogProjectEnemyVisualVariation,
			Warning,
			TEXT("Enemy arousal morph setup skipped for %s because neither '%s' nor '%s' was found."),
			*GetNameSafe(Pawn),
			*Settings->FlaccidMorphName.ToString(),
			*Settings->ErectionMorphName.ToString());
		return;
	}

	if (!bFoundFlaccidMorph)
	{
		UE_LOG(
			LogProjectEnemyVisualVariation,
			Warning,
			TEXT("Enemy arousal morph '%s' was not found on %s. The subsystem will animate only '%s'."),
			*Settings->FlaccidMorphName.ToString(),
			*GetNameSafe(Pawn),
			*Settings->ErectionMorphName.ToString());
	}

	if (!bFoundErectionMorph)
	{
		UE_LOG(
			LogProjectEnemyVisualVariation,
			Warning,
			TEXT("Enemy arousal morph '%s' was not found on %s. The subsystem will animate only '%s'."),
			*Settings->ErectionMorphName.ToString(),
			*GetNameSafe(Pawn),
			*Settings->FlaccidMorphName.ToString());
	}

	ApplyArousalMorphState(MorphState, 0.0f);
	TrackedArousalStates.FindOrAdd(ProjectEnemyVisualVariationSubsystemPrivate::MakePawnKey(Pawn)) = MoveTemp(MorphState);
}

void UProjectEnemyVisualVariationSubsystem::UpdateSightTriggeredArousal(const float CurrentTimeSeconds)
{
	UWorld* World = GetWorld();
	APawn* PlayerPawn = World ? UGameplayStatics::GetPlayerPawn(World, 0) : nullptr;

	for (TPair<TObjectKey<APawn>, FProjectEnemyArousalMorphState>& Pair : TrackedArousalStates)
	{
		FProjectEnemyArousalMorphState& MorphState = Pair.Value;
		APawn* EnemyPawn = MorphState.Pawn.Get();
		if (!IsValid(EnemyPawn))
		{
			continue;
		}

		const bool bIsDead = IsPawnDead(EnemyPawn, MorphState.CombatComponent.Get());
		const bool bCanSeePlayer = !bIsDead && IsValid(PlayerPawn) && CanPawnSeeTrackedPlayer(EnemyPawn, PlayerPawn);
		if (MorphState.bShouldBeAroused == bCanSeePlayer)
		{
			continue;
		}

		MorphState.bShouldBeAroused = bCanSeePlayer;
		if (bCanSeePlayer)
		{
			UE_LOG(
				LogProjectEnemyVisualVariation,
				Log,
				TEXT("Enemy arousal morph transition triggered for %s at %.2fs."),
				*GetNameSafe(EnemyPawn),
				CurrentTimeSeconds);
		}
		else
		{
			UE_LOG(
				LogProjectEnemyVisualVariation,
				Log,
				TEXT("Enemy arousal morph transition reversed for %s at %.2fs."),
				*GetNameSafe(EnemyPawn),
				CurrentTimeSeconds);
		}
	}
}

void UProjectEnemyVisualVariationSubsystem::AdvanceArousalMorphs(const float DeltaTime)
{
	const UProjectEnemyVisualVariationSettings* Settings = UProjectEnemyVisualVariationSettings::Get();
	if (!Settings || DeltaTime <= 0.0f)
	{
		return;
	}

	TArray<TObjectKey<APawn>> KeysToRemove;
	for (TPair<TObjectKey<APawn>, FProjectEnemyArousalMorphState>& Pair : TrackedArousalStates)
	{
		FProjectEnemyArousalMorphState& MorphState = Pair.Value;
		if (!MorphState.Pawn.IsValid() || !MorphState.CustomizationComponent.IsValid())
		{
			KeysToRemove.Add(Pair.Key);
			continue;
		}

		const float TargetArousalAlpha = MorphState.bShouldBeAroused ? 1.0f : 0.0f;
		const float NewArousalAlpha = FMath::FInterpConstantTo(
			MorphState.CurrentArousalAlpha,
			TargetArousalAlpha,
			DeltaTime,
			Settings->MorphTransitionSpeed);
		if (!FMath::IsNearlyEqual(NewArousalAlpha, MorphState.CurrentArousalAlpha, KINDA_SMALL_NUMBER))
		{
			MorphState.CurrentArousalAlpha = NewArousalAlpha;
			ApplyArousalMorphState(MorphState, MorphState.CurrentArousalAlpha);
		}
	}

	for (const TObjectKey<APawn>& Key : KeysToRemove)
	{
		TrackedArousalStates.Remove(Key);
	}
}

bool UProjectEnemyVisualVariationSubsystem::IsPawnDead(
	const APawn* Pawn,
	const UProjectCombatAttributeComponent* CombatComponent) const
{
	const UProjectCombatAttributeComponent* EffectiveCombatComponent = CombatComponent;
	if (!IsValid(EffectiveCombatComponent) && IsValid(Pawn))
	{
		EffectiveCombatComponent = Pawn->FindComponentByClass<UProjectCombatAttributeComponent>();
	}

	return IsValid(EffectiveCombatComponent) && EffectiveCombatComponent->IsDead();
}

bool UProjectEnemyVisualVariationSubsystem::CanPawnSeeTrackedPlayer(const APawn* EnemyPawn, const APawn* PlayerPawn) const
{
	const UProjectEnemyVisualVariationSettings* Settings = UProjectEnemyVisualVariationSettings::Get();
	UWorld* World = GetWorld();
	if (!Settings || !IsValid(World) || !IsValid(EnemyPawn) || !IsValid(PlayerPawn))
	{
		return false;
	}

	FVector EnemyViewLocation = EnemyPawn->GetActorLocation();
	FRotator EnemyViewRotation = EnemyPawn->GetActorRotation();
	EnemyPawn->GetActorEyesViewPoint(EnemyViewLocation, EnemyViewRotation);

	FVector PlayerViewLocation = PlayerPawn->GetActorLocation();
	FRotator PlayerViewRotation = PlayerPawn->GetActorRotation();
	PlayerPawn->GetActorEyesViewPoint(PlayerViewLocation, PlayerViewRotation);

	const FVector ToPlayer = PlayerViewLocation - EnemyViewLocation;
	const float MaxSightRange = FMath::Max(Settings->SightRange, 0.0f);
	bool bCanSeePlayer = ToPlayer.SizeSquared() <= FMath::Square(MaxSightRange);

	if (bCanSeePlayer)
	{
		const float FacingDot = FVector::DotProduct(EnemyViewRotation.Vector().GetSafeNormal(), ToPlayer.GetSafeNormal());
		bCanSeePlayer = FacingDot >= Settings->SightDotThreshold;
	}

	if (bCanSeePlayer)
	{
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ProjectEnemyVisualVariationSight), false);
		QueryParams.AddIgnoredActor(EnemyPawn);
		QueryParams.AddIgnoredActor(PlayerPawn);

		FHitResult HitResult;
		bCanSeePlayer = !World->LineTraceSingleByChannel(HitResult, EnemyViewLocation, PlayerViewLocation, ECC_Visibility, QueryParams)
			|| HitResult.GetActor() == nullptr;
	}

	return bCanSeePlayer;
}

void UProjectEnemyVisualVariationSubsystem::ApplyArousalMorphState(FProjectEnemyArousalMorphState& MorphState, const float ArousalAlpha) const
{
	UEFCharacterCustomizationComponent* CustomizationComponent = MorphState.CustomizationComponent.Get();
	if (!IsValid(CustomizationComponent))
	{
		return;
	}

	const float ClampedArousalAlpha = FMath::Clamp(ArousalAlpha, 0.0f, 1.0f);
	const float FlaccidValue = 1.0f - ClampedArousalAlpha;
	const float ErectionValue = ClampedArousalAlpha;

	for (const FMorphSliderEntry& Entry : MorphState.FlaccidEntries)
	{
		CustomizationComponent->ApplyMorph(Entry, FlaccidValue);
	}

	for (const FMorphSliderEntry& Entry : MorphState.ErectionEntries)
	{
		CustomizationComponent->ApplyMorph(Entry, ErectionValue);
	}
}

void UProjectEnemyVisualVariationSubsystem::CleanupTrackedArousalStates()
{
	TArray<TObjectKey<APawn>> KeysToRemove;
	for (const TPair<TObjectKey<APawn>, FProjectEnemyArousalMorphState>& Pair : TrackedArousalStates)
	{
		const FProjectEnemyArousalMorphState& MorphState = Pair.Value;
		if (!MorphState.Pawn.IsValid() || !MorphState.CustomizationComponent.IsValid())
		{
			KeysToRemove.Add(Pair.Key);
		}
	}

	for (const TObjectKey<APawn>& Key : KeysToRemove)
	{
		TrackedArousalStates.Remove(Key);
	}
}

void UProjectEnemyVisualVariationSubsystem::MarkActorProcessed(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	ProcessedActors.Add(ProjectEnemyVisualVariationSubsystemPrivate::MakeObjectKey(Actor));
	PendingActors.Remove(ProjectEnemyVisualVariationSubsystemPrivate::MakeObjectKey(Actor));
}

bool UProjectEnemyVisualVariationSubsystem::IsActorProcessed(const AActor* Actor) const
{
	return IsValid(Actor) && ProcessedActors.Contains(ProjectEnemyVisualVariationSubsystemPrivate::MakeObjectKey(Actor));
}

bool UProjectEnemyVisualVariationSubsystem::IsActorPending(const AActor* Actor) const
{
	return IsValid(Actor) && PendingActors.Contains(ProjectEnemyVisualVariationSubsystemPrivate::MakeObjectKey(Actor));
}

void UProjectEnemyVisualVariationSubsystem::MarkActorPending(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	PendingActors.Add(ProjectEnemyVisualVariationSubsystemPrivate::MakeObjectKey(Actor));
}

void UProjectEnemyVisualVariationSubsystem::ClearActorPending(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	PendingActors.Remove(ProjectEnemyVisualVariationSubsystemPrivate::MakeObjectKey(Actor));
}
