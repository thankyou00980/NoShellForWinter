#include "EFCharacterCustomizationComponent.h"

#include "Animation/MorphTarget.h"
#include "Animation/Skeleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EFCharacterCreationSettings.h"
#include "EFCharacterCustomizationSaveGame.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/DateTime.h"
#include "Modules/ModuleManager.h"
#include "UObject/SoftObjectPath.h"

namespace CharacterCustomizationComponentPrivate
{
	static FString NormalizeForMatching(const FString& Value)
	{
		return Value.ToLower();
	}

	static bool IsHeadMorphName(const FString& Value)
	{
		const FString NormalizedValue = NormalizeForMatching(Value);
		static const TArray<FString> HeadTokens = {
			TEXT("head"),
			TEXT("face"),
			TEXT("eye"),
			TEXT("brow"),
			TEXT("cheek"),
			TEXT("chin"),
			TEXT("jaw"),
			TEXT("nose"),
			TEXT("mouth"),
			TEXT("lip"),
			TEXT("forehead"),
			TEXT("ear"),
			TEXT("eyelid"),
			TEXT("beard"),
			TEXT("lash"),
			TEXT("tongue"),
			TEXT("teeth")
		};

		for (const FString& Token : HeadTokens)
		{
			if (NormalizedValue.Contains(Token))
			{
				return true;
			}
		}

		return false;
	}

	static FString InferBodySectionName(const FString& Value)
	{
		const FString NormalizedValue = NormalizeForMatching(Value);

		static const TArray<FString> UpperTokens = {
			TEXT("breast"),
			TEXT("chest"),
			TEXT("pect"),
			TEXT("shoulder"),
			TEXT("arm"),
			TEXT("neck"),
			TEXT("collar"),
			TEXT("clavicle"),
			TEXT("back"),
			TEXT("trap")
		};

		for (const FString& Token : UpperTokens)
		{
			if (NormalizedValue.Contains(Token))
			{
				return TEXT("Upper");
			}
		}

		static const TArray<FString> LowerTokens = {
			TEXT("hip"),
			TEXT("glute"),
			TEXT("butt"),
			TEXT("thigh"),
			TEXT("leg"),
			TEXT("knee"),
			TEXT("calf"),
			TEXT("foot"),
			TEXT("toe"),
			TEXT("pelvis"),
			TEXT("groin")
		};

		for (const FString& Token : LowerTokens)
		{
			if (NormalizedValue.Contains(Token))
			{
				return TEXT("Lower");
			}
		}

		return TEXT("Middle");
	}

	static FString TargetToString(ECharacterCustomizationTarget Target)
	{
		switch (Target)
		{
		case ECharacterCustomizationTarget::Body:
			return TEXT("Body");
		case ECharacterCustomizationTarget::Clothing:
			return TEXT("Clothing");
		case ECharacterCustomizationTarget::Auto:
		default:
			return TEXT("Auto");
		}
	}

	static bool SortMeshOptions(const FCharacterSkeletalMeshOption& Left, const FCharacterSkeletalMeshOption& Right)
	{
		const FString LeftLabel = Left.DisplayName.IsEmpty() ? Left.SkeletalMesh.ToSoftObjectPath().GetAssetName() : Left.DisplayName;
		const FString RightLabel = Right.DisplayName.IsEmpty() ? Right.SkeletalMesh.ToSoftObjectPath().GetAssetName() : Right.DisplayName;
		if (LeftLabel != RightLabel)
		{
			return LeftLabel < RightLabel;
		}

		return Left.SkeletalMesh.ToSoftObjectPath().ToString() < Right.SkeletalMesh.ToSoftObjectPath().ToString();
	}
}

UEFCharacterCustomizationComponent::UEFCharacterCustomizationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UEFCharacterCustomizationComponent::InitializeForActor(AActor* InOwningActor)
{
	const bool bHadRuntimeState = IsValid(BodyMeshComponent.Get())
		|| IsValid(HairMeshComponent.Get())
		|| AvailableMorphEntries.Num() > 0
		|| !CurrentMorphValues.IsEmpty();
	const FCharacterCustomizationState PreviousState = bHadRuntimeState ? CaptureCurrentState() : FCharacterCustomizationState();
	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();

	ResetDiscoveredData();

	if (!IsValid(InOwningActor))
	{
		CompatibilityError = TEXT("No valid actor was found for character creation.");
		return false;
	}

	OwningActor = InOwningActor;
	CurrentSkinColor = Settings->DefaultSkinColor;
	CurrentIrisColor = Settings->DefaultIrisColor;

	DiscoverMeshComponents(InOwningActor);
	BuildMeshSelectionOptions();
	bMeshCompatible = ValidateCompatibility();

	if (bMeshCompatible)
	{
		BuildMorphEntries();

		if (bHadRuntimeState)
		{
			ApplyStateInternal(PreviousState, true);
		}
		else
		{
			ApplyStateInternal(BuildDefaultState(), false);
		}
	}

	return true;
}

bool UEFCharacterCustomizationComponent::EvaluateCompatibilityForActor(AActor* InOwningActor, FString& OutFailureReason)
{
	if (!InitializeForActor(InOwningActor))
	{
		OutFailureReason = CompatibilityError.IsEmpty() ? TEXT("Failed to initialize the character creation system.") : CompatibilityError;
		return false;
	}

	if (!bMeshCompatible)
	{
		OutFailureReason = CompatibilityError.IsEmpty() ? TEXT("The mesh is not compatible with the character creation system.") : CompatibilityError;
		return false;
	}

	OutFailureReason.Reset();
	return true;
}

TArray<FMorphSliderEntry> UEFCharacterCustomizationComponent::GetAvailableMorphEntriesForCategory(const FName Category, const FString& SearchFilter) const
{
	TArray<FMorphSliderEntry> FilteredEntries;
	const FString SearchFilterLower = CharacterCustomizationComponentPrivate::NormalizeForMatching(SearchFilter);

	for (const FMorphSliderEntry& Entry : AvailableMorphEntries)
	{
		if (!Category.IsNone() && Entry.Category != Category)
		{
			continue;
		}

		if (!SearchFilterLower.IsEmpty())
		{
			const FString DisplayName = Entry.DisplayName.IsEmpty() ? Entry.MorphName.ToString() : Entry.DisplayName;
			if (!CharacterCustomizationComponentPrivate::NormalizeForMatching(DisplayName).Contains(SearchFilterLower))
			{
				continue;
			}
		}

		FilteredEntries.Add(Entry);
	}

	return FilteredEntries;
}

TArray<FName> UEFCharacterCustomizationComponent::GetAvailableCategories() const
{
	TArray<FName> Categories;
	for (const FMorphSliderEntry& Entry : AvailableMorphEntries)
	{
		Categories.AddUnique(Entry.Category);
	}

	return Categories;
}

bool UEFCharacterCustomizationComponent::ApplyMorph(const FMorphSliderEntry& Entry, float NewValue)
{
	if (!bMeshCompatible)
	{
		return false;
	}

	const float ClampedValue = FMath::Clamp(NewValue, Entry.MinValue, Entry.MaxValue);
	SetCurrentMorphValue(Entry, ClampedValue);
	const bool bApplied = ApplyMorphValue(Entry, ClampedValue);
	MorphStateAppliedEvent.Broadcast();
	return bApplied;
}

bool UEFCharacterCustomizationComponent::ResetMorph(const FMorphSliderEntry& Entry)
{
	return ApplyMorph(Entry, Entry.DefaultValue);
}

void UEFCharacterCustomizationComponent::ResetAllToDefaults()
{
	ApplyStateInternal(BuildDefaultState(), false);
}

void UEFCharacterCustomizationComponent::RandomizeAll(int32 Seed)
{
	if (!bMeshCompatible)
	{
		return;
	}

	FRandomStream RandomStream(Seed == INDEX_NONE ? FMath::Rand() : Seed);
	for (const FMorphSliderEntry& Entry : AvailableMorphEntries)
	{
		if (!Entry.bAllowRandomize)
		{
			continue;
		}

		const float RandomValue = RandomStream.FRandRange(Entry.MinValue, Entry.MaxValue);
		SetCurrentMorphValue(Entry, RandomValue);
	}

	ApplyCurrentMorphState();
	MorphStateAppliedEvent.Broadcast();
}

FCharacterCustomizationState UEFCharacterCustomizationComponent::CaptureCurrentState() const
{
	FCharacterCustomizationState State;
	State.bPauseAnimation = bPauseAnimation;
	State.bShowClothes = bShowClothes;
	State.SkinColor = CurrentSkinColor;
	State.IrisColor = CurrentIrisColor;
	State.ActiveCategory = TEXT("Body");

	USkeletalMeshComponent* ActiveBodySelectionComponent = IsValid(BodyMeshSelectionComponent.Get()) ? BodyMeshSelectionComponent.Get() : BodyMeshComponent.Get();
	if (IsValid(ActiveBodySelectionComponent) && IsValid(ActiveBodySelectionComponent->GetSkeletalMeshAsset()))
	{
		State.bHasSelectedBodyMesh = true;
		State.SelectedBodyMeshAsset = FSoftObjectPath(ActiveBodySelectionComponent->GetSkeletalMeshAsset());
	}

	if (IsValid(HairMeshComponent.Get()) && IsValid(HairMeshComponent->GetSkeletalMeshAsset()))
	{
		State.bHasSelectedHairMesh = true;
		State.SelectedHairMeshAsset = FSoftObjectPath(HairMeshComponent->GetSkeletalMeshAsset());
	}

	if (IsValid(HairMeshComponent.Get()))
	{
		State.bHasHairRelativeTransform = true;
		State.HairRelativeTransform = HairMeshComponent->GetRelativeTransform();
	}

	for (const FMorphSliderEntry& Entry : AvailableMorphEntries)
	{
		FCharacterMorphValue MorphValue;
		MorphValue.MorphName = Entry.MorphName;
		MorphValue.Target = Entry.Target;
		MorphValue.TargetComponentName = Entry.TargetComponentName;
		MorphValue.Value = GetCurrentMorphValue(Entry);
		State.MorphValues.Add(MorphValue);
	}

	return State;
}

void UEFCharacterCustomizationComponent::ApplyState(const FCharacterCustomizationState& NewState)
{
	ApplyStateInternal(NewState, true);
}

void UEFCharacterCustomizationComponent::ApplyStateInternal(const FCharacterCustomizationState& NewState, bool bApplyMeshSelections)
{
	if (bApplyMeshSelections)
	{
		if (NewState.bHasSelectedBodyMesh && !NewState.SelectedBodyMeshAsset.IsNull())
		{
			if (USkeletalMesh* SelectedBodyMesh = Cast<USkeletalMesh>(NewState.SelectedBodyMeshAsset.TryLoad()))
			{
				ApplyBodySkeletalMesh(SelectedBodyMesh);
			}
		}

		if (NewState.bHasSelectedHairMesh && !NewState.SelectedHairMeshAsset.IsNull())
		{
			if (USkeletalMesh* SelectedHairMesh = Cast<USkeletalMesh>(NewState.SelectedHairMeshAsset.TryLoad()))
			{
				ApplyHairSkeletalMesh(SelectedHairMesh);
			}
		}
	}

	SetShowClothes(NewState.bShowClothes);
	SetPauseAnimation(NewState.bPauseAnimation);
	SetSkinColor(NewState.SkinColor);
	SetIrisColor(NewState.IrisColor);

	if (NewState.bHasHairRelativeTransform && IsValid(HairMeshComponent.Get()))
	{
		HairMeshComponent->SetRelativeTransform(NewState.HairRelativeTransform);
	}

	if (!bMeshCompatible)
	{
		CurrentSkinColor = NewState.SkinColor;
		CurrentIrisColor = NewState.IrisColor;
		return;
	}

	CurrentMorphValues.Reset();
	for (const FMorphSliderEntry& Entry : AvailableMorphEntries)
	{
		SetCurrentMorphValue(Entry, Entry.DefaultValue);
	}

	for (const FCharacterMorphValue& MorphValue : NewState.MorphValues)
	{
		const FMorphSliderEntry* MatchingEntry = AvailableMorphEntries.FindByPredicate([this, &MorphValue](const FMorphSliderEntry& Entry)
		{
			return MakeMorphKey(Entry) == MakeMorphKey(MorphValue);
		});

		if (MatchingEntry)
		{
			SetCurrentMorphValue(*MatchingEntry, FMath::Clamp(MorphValue.Value, MatchingEntry->MinValue, MatchingEntry->MaxValue));
		}
	}

	ApplyCurrentMorphState();
	MorphStateAppliedEvent.Broadcast();
}

bool UEFCharacterCustomizationComponent::SaveCurrentStateAsConfirmed()
{
	UEFCharacterCustomizationSaveGame* SaveGame = LoadOrCreateSaveGame();
	if (!SaveGame)
	{
		return false;
	}

	SaveGame->bHasLastConfirmedState = true;
	SaveGame->LastConfirmedState = CaptureCurrentState();

	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();
	return UGameplayStatics::SaveGameToSlot(SaveGame, Settings->SaveSlotName, Settings->SaveUserIndex);
}

bool UEFCharacterCustomizationComponent::SavePreset(const FString& PresetName)
{
	const FString TrimmedPresetName = PresetName.TrimStartAndEnd();
	if (TrimmedPresetName.IsEmpty())
	{
		return false;
	}

	FCharacterPresetData PresetData;
	PresetData.PresetName = TrimmedPresetName;
	PresetData.State = CaptureCurrentState();
	PresetData.SavedAtUtc = FDateTime::UtcNow().ToString();

	return SavePresetData(PresetData);
}

bool UEFCharacterCustomizationComponent::LoadPreset(const FString& PresetName)
{
	FCharacterPresetData PresetData;
	if (!LoadPresetData(PresetName, PresetData))
	{
		return false;
	}

	ApplyState(PresetData.State);
	return true;
}

TArray<FString> UEFCharacterCustomizationComponent::GetPresetNames() const
{
	if (UEFCharacterCustomizationSaveGame* SaveGame = LoadOrCreateSaveGame())
	{
		return SaveGame->GetPresetNames();
	}

	return {};
}

float UEFCharacterCustomizationComponent::GetCurrentMorphValue(const FMorphSliderEntry& Entry) const
{
	if (const float* StoredValue = CurrentMorphValues.Find(MakeMorphKey(Entry)))
	{
		return *StoredValue;
	}

	if (USkeletalMeshComponent* TargetMeshComponent = ResolveTargetMeshComponent(Entry))
	{
		return TargetMeshComponent->GetMorphTarget(Entry.MorphName);
	}

	return Entry.DefaultValue;
}

void UEFCharacterCustomizationComponent::SetShowClothes(bool bInShowClothes)
{
	bShowClothes = bInShowClothes;

	for (USkeletalMeshComponent* ClothingMeshComponent : ClothingMeshComponents)
	{
		if (!IsValid(ClothingMeshComponent))
		{
			continue;
		}

		ClothingMeshComponent->SetVisibility(bShowClothes, true);
		ClothingMeshComponent->SetHiddenInGame(!bShowClothes, true);
	}
}

void UEFCharacterCustomizationComponent::SetPauseAnimation(bool bInPauseAnimation)
{
	bPauseAnimation = bInPauseAnimation;

	for (USkeletalMeshComponent* MeshComponent : AllDiscoveredMeshComponents)
	{
		if (!IsValid(MeshComponent))
		{
			continue;
		}

		MeshComponent->bPauseAnims = bPauseAnimation;
	}
}

void UEFCharacterCustomizationComponent::SetSkinColor(const FLinearColor& InSkinColor)
{
	CurrentSkinColor = InSkinColor;
	for (USkeletalMeshComponent* MeshComponent : AllDiscoveredMeshComponents)
	{
		ApplySkinColorToMesh(MeshComponent);
	}
}

void UEFCharacterCustomizationComponent::SetIrisColor(const FLinearColor& InIrisColor)
{
	CurrentIrisColor = InIrisColor;
	for (USkeletalMeshComponent* MeshComponent : AllDiscoveredMeshComponents)
	{
		ApplyIrisColorToMesh(MeshComponent);
	}
}

FString UEFCharacterCustomizationComponent::GetCurrentBodyMeshDisplayName() const
{
	USkeletalMeshComponent* ActiveBodySelectionComponent = IsValid(BodyMeshSelectionComponent.Get()) ? BodyMeshSelectionComponent.Get() : BodyMeshComponent.Get();
	if (IsValid(ActiveBodySelectionComponent) && IsValid(ActiveBodySelectionComponent->GetSkeletalMeshAsset()))
	{
		const int32 MeshIndex = FindMeshOptionIndex(AvailableBodyMeshOptions, ActiveBodySelectionComponent->GetSkeletalMeshAsset());
		if (AvailableBodyMeshOptions.IsValidIndex(MeshIndex))
		{
			return GetDisplayNameForMesh(ActiveBodySelectionComponent->GetSkeletalMeshAsset(), AvailableBodyMeshOptions[MeshIndex].DisplayName);
		}

		return GetDisplayNameForMesh(ActiveBodySelectionComponent->GetSkeletalMeshAsset());
	}

	return TEXT("Unavailable");
}

FString UEFCharacterCustomizationComponent::GetCurrentHairMeshDisplayName() const
{
	if (IsValid(HairMeshComponent.Get()) && IsValid(HairMeshComponent->GetSkeletalMeshAsset()))
	{
		const int32 MeshIndex = FindMeshOptionIndex(AvailableHairMeshOptions, HairMeshComponent->GetSkeletalMeshAsset());
		if (AvailableHairMeshOptions.IsValidIndex(MeshIndex))
		{
			return GetDisplayNameForMesh(HairMeshComponent->GetSkeletalMeshAsset(), AvailableHairMeshOptions[MeshIndex].DisplayName);
		}

		return GetDisplayNameForMesh(HairMeshComponent->GetSkeletalMeshAsset());
	}

	return TEXT("Unavailable");
}

bool UEFCharacterCustomizationComponent::SelectRelativeBodyMeshOption(int32 Direction)
{
	if (AvailableBodyMeshOptions.Num() == 0)
	{
		return false;
	}

	USkeletalMeshComponent* ActiveBodySelectionComponent = IsValid(BodyMeshSelectionComponent.Get()) ? BodyMeshSelectionComponent.Get() : BodyMeshComponent.Get();
	const int32 CurrentIndex = IsValid(ActiveBodySelectionComponent) && IsValid(ActiveBodySelectionComponent->GetSkeletalMeshAsset())
		? FindMeshOptionIndex(AvailableBodyMeshOptions, ActiveBodySelectionComponent->GetSkeletalMeshAsset())
		: INDEX_NONE;
	const int32 StartIndex = CurrentIndex == INDEX_NONE ? (Direction >= 0 ? -1 : 0) : CurrentIndex;
	const int32 NewIndex = (StartIndex + Direction + AvailableBodyMeshOptions.Num()) % AvailableBodyMeshOptions.Num();

	if (!AvailableBodyMeshOptions.IsValidIndex(NewIndex))
	{
		return false;
	}

	FCharacterCustomizationState State = CaptureCurrentState();
	State.bHasSelectedBodyMesh = true;
	State.SelectedBodyMeshAsset = AvailableBodyMeshOptions[NewIndex].SkeletalMesh.ToSoftObjectPath();
	ApplyState(State);
	return true;
}

bool UEFCharacterCustomizationComponent::SelectRelativeHairMeshOption(int32 Direction)
{
	if (AvailableHairMeshOptions.Num() == 0)
	{
		return false;
	}

	const int32 CurrentIndex = IsValid(HairMeshComponent.Get()) && IsValid(HairMeshComponent->GetSkeletalMeshAsset())
		? FindMeshOptionIndex(AvailableHairMeshOptions, HairMeshComponent->GetSkeletalMeshAsset())
		: INDEX_NONE;
	const int32 StartIndex = CurrentIndex == INDEX_NONE ? (Direction >= 0 ? -1 : 0) : CurrentIndex;
	const int32 NewIndex = (StartIndex + Direction + AvailableHairMeshOptions.Num()) % AvailableHairMeshOptions.Num();

	if (!AvailableHairMeshOptions.IsValidIndex(NewIndex))
	{
		return false;
	}

	FCharacterCustomizationState State = CaptureCurrentState();
	State.bHasSelectedHairMesh = true;
	State.SelectedHairMeshAsset = AvailableHairMeshOptions[NewIndex].SkeletalMesh.ToSoftObjectPath();
	ApplyState(State);
	return true;
}

FTransform UEFCharacterCustomizationComponent::GetHairRelativeTransform() const
{
	if (IsValid(HairMeshComponent.Get()))
	{
		return HairMeshComponent->GetRelativeTransform();
	}

	return FTransform::Identity;
}

bool UEFCharacterCustomizationComponent::SetHairRelativeTransform(const FTransform& InTransform)
{
	if (!IsValid(HairMeshComponent.Get()))
	{
		return false;
	}

	HairMeshComponent->SetRelativeTransform(InTransform);
	return true;
}

void UEFCharacterCustomizationComponent::ResetHairTransformToDefault()
{
	if (IsValid(HairMeshComponent.Get()))
	{
		HairMeshComponent->SetRelativeTransform(DefaultHairRelativeTransform);
	}
}

FCharacterCustomizationState UEFCharacterCustomizationComponent::BuildDefaultState() const
{
	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();

	FCharacterCustomizationState DefaultState;
	DefaultState.bPauseAnimation = false;
	DefaultState.bShowClothes = true;
	DefaultState.SkinColor = Settings->DefaultSkinColor;
	DefaultState.IrisColor = Settings->DefaultIrisColor;
	DefaultState.ActiveCategory = TEXT("Body");

	USkeletalMeshComponent* ActiveBodySelectionComponent = IsValid(BodyMeshSelectionComponent.Get()) ? BodyMeshSelectionComponent.Get() : BodyMeshComponent.Get();
	if (IsValid(ActiveBodySelectionComponent) && IsValid(ActiveBodySelectionComponent->GetSkeletalMeshAsset()))
	{
		DefaultState.bHasSelectedBodyMesh = true;
		DefaultState.SelectedBodyMeshAsset = FSoftObjectPath(ActiveBodySelectionComponent->GetSkeletalMeshAsset());
	}

	if (IsValid(HairMeshComponent.Get()) && IsValid(HairMeshComponent->GetSkeletalMeshAsset()))
	{
		DefaultState.bHasSelectedHairMesh = true;
		DefaultState.SelectedHairMeshAsset = FSoftObjectPath(HairMeshComponent->GetSkeletalMeshAsset());
	}

	if (IsValid(HairMeshComponent.Get()))
	{
		DefaultState.bHasHairRelativeTransform = true;
		DefaultState.HairRelativeTransform = DefaultHairRelativeTransform;
	}

	for (const FMorphSliderEntry& Entry : AvailableMorphEntries)
	{
		FCharacterMorphValue MorphValue;
		MorphValue.MorphName = Entry.MorphName;
		MorphValue.Target = Entry.Target;
		MorphValue.TargetComponentName = Entry.TargetComponentName;
		MorphValue.Value = Entry.DefaultValue;
		DefaultState.MorphValues.Add(MorphValue);
	}

	return DefaultState;
}

void UEFCharacterCustomizationComponent::ResetDiscoveredData()
{
	BodyMeshComponent = nullptr;
	BodyMeshSelectionComponent = nullptr;
	DefaultBodyMeshSelectionAsset = nullptr;
	HairMeshComponent = nullptr;
	ClothingMeshComponents.Reset();
	AllDiscoveredMeshComponents.Reset();
	AvailableMorphEntries.Reset();
	AvailableBodyMeshOptions.Reset();
	AvailableHairMeshOptions.Reset();
	CurrentMorphValues.Reset();
	MeshMorphNameCache.Reset();
	MorphTargetMeshComponentCache.Reset();
	DynamicMaterialInstances.Reset();
	SkinMaterialSlotCache.Reset();
	IrisMaterialSlotCache.Reset();
	DefaultHairRelativeTransform = FTransform::Identity;
	bMeshCompatible = false;
	CompatibilityError.Reset();
	bShowClothes = true;
	bPauseAnimation = false;
	CurrentSkinColor = UEFCharacterCreationSettings::Get()->DefaultSkinColor;
	CurrentIrisColor = UEFCharacterCreationSettings::Get()->DefaultIrisColor;
}

void UEFCharacterCustomizationComponent::DiscoverMeshComponents(AActor* InOwningActor)
{
	check(InOwningActor);

	TArray<USkeletalMeshComponent*> MeshComponents;
	InOwningActor->GetComponents(MeshComponents);

	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();
	ACharacter* CharacterOwner = Cast<ACharacter>(InOwningActor);

	struct FScoredMeshComponent
	{
		USkeletalMeshComponent* Component = nullptr;
		int32 Score = 0;
	};

	TArray<FScoredMeshComponent> ScoredComponents;

	for (USkeletalMeshComponent* MeshComponent : MeshComponents)
	{
		if (!IsValid(MeshComponent) || !IsValid(MeshComponent->GetSkeletalMeshAsset()))
		{
			continue;
		}

		AllDiscoveredMeshComponents.Add(MeshComponent);

		FScoredMeshComponent ScoredComponent;
		ScoredComponent.Component = MeshComponent;
		ScoredComponent.Score += GatherMorphNames(MeshComponent).Num();

		const FString ComponentName = MeshComponent->GetName();
		if (CharacterOwner && CharacterOwner->GetMesh() == MeshComponent)
		{
			ScoredComponent.Score += 1000;
		}

		if (MatchesAnyHint(ComponentName, Settings->BodyMeshComponentHints))
		{
			ScoredComponent.Score += 250;
		}

		const FString MeshPath = MeshComponent->GetSkeletalMeshAsset()->GetPathName();
		if (MatchesAnyHint(MeshPath, Settings->DazPathTokens))
		{
			ScoredComponent.Score += 500;
		}

		ScoredComponents.Add(ScoredComponent);
	}

	ScoredComponents.Sort([](const FScoredMeshComponent& Left, const FScoredMeshComponent& Right)
	{
		return Left.Score > Right.Score;
	});

	if (ScoredComponents.Num() > 0)
	{
		BodyMeshComponent = ScoredComponents[0].Component;
	}

	int32 BestBodySelectionScore = INDEX_NONE;
	for (const FScoredMeshComponent& ScoredComponent : ScoredComponents)
	{
		if (ScoredComponent.Component == BodyMeshComponent)
		{
			continue;
		}

		const FString ComponentName = ScoredComponent.Component->GetName();
		if (!MatchesAnyHint(ComponentName, Settings->BodyMeshSelectionComponentHints))
		{
			continue;
		}

		int32 SelectionScore = 1000;
		if (ScoredComponent.Component->GetAttachParent() == BodyMeshComponent)
		{
			SelectionScore += 125;
		}

		if (IsValid(BodyMeshComponent.Get()) && IsSkeletonCompatibleWithReference(ScoredComponent.Component->GetSkeletalMeshAsset(), BodyMeshComponent->GetSkeletalMeshAsset()))
		{
			SelectionScore += 125;
		}

		if (SelectionScore > BestBodySelectionScore)
		{
			BestBodySelectionScore = SelectionScore;
			BodyMeshSelectionComponent = ScoredComponent.Component;
		}
	}

	if (!IsValid(BodyMeshSelectionComponent.Get()))
	{
		BodyMeshSelectionComponent = BodyMeshComponent;
	}

	if (IsValid(BodyMeshSelectionComponent.Get()))
	{
		if (const USkeletalMeshComponent* SelectionTemplate = Cast<USkeletalMeshComponent>(BodyMeshSelectionComponent->GetArchetype()))
		{
			DefaultBodyMeshSelectionAsset = SelectionTemplate->GetSkeletalMeshAsset();
		}

		if (!IsValid(DefaultBodyMeshSelectionAsset.Get()))
		{
			DefaultBodyMeshSelectionAsset = BodyMeshSelectionComponent->GetSkeletalMeshAsset();
		}
	}

	int32 BestHairScore = INDEX_NONE;
	for (const FScoredMeshComponent& ScoredComponent : ScoredComponents)
	{
		if (ScoredComponent.Component == BodyMeshComponent || ScoredComponent.Component == BodyMeshSelectionComponent)
		{
			continue;
		}

		int32 HairScore = 0;
		const FString ComponentName = ScoredComponent.Component->GetName();
		if (MatchesAnyHint(ComponentName, Settings->HairComponentHints))
		{
			HairScore += 1000;
		}

		if (ScoredComponent.Component->GetAttachParent() == BodyMeshSelectionComponent || ScoredComponent.Component->GetAttachParent() == BodyMeshComponent)
		{
			HairScore += 125;
		}

		USkeletalMesh* HairReferenceMesh = nullptr;
		if (IsValid(BodyMeshSelectionComponent.Get()))
		{
			HairReferenceMesh = BodyMeshSelectionComponent->GetSkeletalMeshAsset();
		}
		if (!IsValid(HairReferenceMesh) && IsValid(BodyMeshComponent.Get()))
		{
			HairReferenceMesh = BodyMeshComponent->GetSkeletalMeshAsset();
		}

		if (IsValid(HairReferenceMesh) && IsSkeletonCompatibleWithReference(ScoredComponent.Component->GetSkeletalMeshAsset(), HairReferenceMesh))
		{
			HairScore += 125;
		}

		if (HairScore > BestHairScore)
		{
			BestHairScore = HairScore;
			HairMeshComponent = ScoredComponent.Component;
		}
	}

	if (IsValid(HairMeshComponent.Get()))
	{
		if (const USceneComponent* HairTemplate = Cast<USceneComponent>(HairMeshComponent->GetArchetype()))
		{
			DefaultHairRelativeTransform = HairTemplate->GetRelativeTransform();
		}
		else
		{
			DefaultHairRelativeTransform = HairMeshComponent->GetRelativeTransform();
		}
	}

	for (const FScoredMeshComponent& ScoredComponent : ScoredComponents)
	{
		if (ScoredComponent.Component == BodyMeshComponent || ScoredComponent.Component == BodyMeshSelectionComponent || ScoredComponent.Component == HairMeshComponent)
		{
			continue;
		}

		const FString ComponentName = ScoredComponent.Component->GetName();
		const bool bLooksLikeClothing = MatchesAnyHint(ComponentName, Settings->ClothingMeshComponentHints)
			|| ScoredComponent.Component->GetAttachParent() == BodyMeshComponent
			|| ScoredComponent.Component->GetAttachParent() == BodyMeshSelectionComponent;

		if (bLooksLikeClothing)
		{
			ClothingMeshComponents.AddUnique(ScoredComponent.Component);
		}
	}
}

void UEFCharacterCustomizationComponent::BuildMeshSelectionOptions()
{
	BuildBodyMeshOptions();
	BuildHairMeshOptions();
}

void UEFCharacterCustomizationComponent::BuildBodyMeshOptions()
{
	AvailableBodyMeshOptions.Reset();

	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();
	USkeletalMeshComponent* ActiveBodySelectionComponent = IsValid(BodyMeshSelectionComponent.Get()) ? BodyMeshSelectionComponent.Get() : BodyMeshComponent.Get();
	USkeletalMesh* CurrentBodyMesh = IsValid(ActiveBodySelectionComponent) ? ActiveBodySelectionComponent->GetSkeletalMeshAsset() : nullptr;
	USkeletalMesh* ReferenceBodyMesh = DefaultBodyMeshSelectionAsset.Get();
	if (!IsValid(ReferenceBodyMesh))
	{
		ReferenceBodyMesh = CurrentBodyMesh;
	}

	auto TryAddBodyMeshOption = [this, &ReferenceBodyMesh](TArray<FCharacterSkeletalMeshOption>& Options, USkeletalMesh* CandidateMesh, const FString& PreferredLabel)
	{
		if (!IsValid(CandidateMesh))
		{
			return;
		}

		if (IsValid(ReferenceBodyMesh) && !IsSkeletonCompatibleWithReference(CandidateMesh, ReferenceBodyMesh))
		{
			return;
		}

		AddMeshOptionIfValid(Options, CandidateMesh, PreferredLabel);
	};

	TryAddBodyMeshOption(AvailableBodyMeshOptions, DefaultBodyMeshSelectionAsset.Get(), FString());

	for (const FCharacterSkeletalMeshOption& ConfiguredOption : Settings->BodyMeshOptions)
	{
		if (ConfiguredOption.SkeletalMesh.ToSoftObjectPath().IsNull())
		{
			continue;
		}

		USkeletalMesh* CandidateMesh = ConfiguredOption.SkeletalMesh.LoadSynchronous();
		if (!IsValid(CandidateMesh))
		{
			continue;
		}

		TryAddBodyMeshOption(AvailableBodyMeshOptions, CandidateMesh, ConfiguredOption.DisplayName);
	}

	TryAddBodyMeshOption(AvailableBodyMeshOptions, CurrentBodyMesh, FString());

	AvailableBodyMeshOptions.Sort(CharacterCustomizationComponentPrivate::SortMeshOptions);
}

void UEFCharacterCustomizationComponent::BuildHairMeshOptions()
{
	AvailableHairMeshOptions.Reset();

	if (!IsValid(HairMeshComponent.Get()))
	{
		return;
	}

	USkeletalMesh* ReferenceMesh = HairMeshComponent->GetSkeletalMeshAsset();
	if (!IsValid(ReferenceMesh) && IsValid(BodyMeshSelectionComponent.Get()))
	{
		ReferenceMesh = BodyMeshSelectionComponent->GetSkeletalMeshAsset();
	}
	if (!IsValid(ReferenceMesh) && IsValid(BodyMeshComponent.Get()))
	{
		ReferenceMesh = BodyMeshComponent->GetSkeletalMeshAsset();
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();

	for (const FString& SearchPath : Settings->HairMeshSearchPaths)
	{
		if (SearchPath.IsEmpty())
		{
			continue;
		}

		FARFilter Filter;
		Filter.bRecursivePaths = true;
		Filter.bRecursiveClasses = true;
		Filter.PackagePaths.Add(FName(*SearchPath));
		Filter.ClassPaths.Add(USkeletalMesh::StaticClass()->GetClassPathName());

		TArray<FAssetData> AssetDatas;
		AssetRegistryModule.Get().GetAssets(Filter, AssetDatas);

		for (const FAssetData& AssetData : AssetDatas)
		{
			const FString AssetName = AssetData.AssetName.ToString();
			const FString AssetPathString = AssetData.GetSoftObjectPath().ToString();
			if (IsHairAssetExcluded(AssetName) || IsHairAssetExcluded(AssetPathString))
			{
				continue;
			}

			USkeletalMesh* CandidateMesh = Cast<USkeletalMesh>(AssetData.GetAsset());
			if (!IsValid(CandidateMesh))
			{
				continue;
			}

			if (IsValid(ReferenceMesh) && !IsSkeletonCompatibleWithReference(CandidateMesh, ReferenceMesh))
			{
				continue;
			}

			AddMeshOptionIfValid(AvailableHairMeshOptions, CandidateMesh, AssetName);
		}
	}

	if (IsValid(HairMeshComponent->GetSkeletalMeshAsset())
		&& !IsHairAssetExcluded(HairMeshComponent->GetSkeletalMeshAsset()->GetName())
		&& FindMeshOptionIndex(AvailableHairMeshOptions, HairMeshComponent->GetSkeletalMeshAsset()) == INDEX_NONE)
	{
		AddMeshOptionIfValid(AvailableHairMeshOptions, HairMeshComponent->GetSkeletalMeshAsset(), FString());
	}

	AvailableHairMeshOptions.Sort(CharacterCustomizationComponentPrivate::SortMeshOptions);
}

void UEFCharacterCustomizationComponent::BuildMorphEntries()
{
	AvailableMorphEntries.Reset();
	CurrentMorphValues.Reset();
	MorphTargetMeshComponentCache.Reset();

	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();
	TSet<FString> AddedMorphKeys;

	auto TryAddEntry = [this, &AddedMorphKeys](FMorphSliderEntry Entry)
	{
		if (Entry.MorphName.IsNone())
		{
			return;
		}

		if (Entry.DisplayName.IsEmpty())
		{
			Entry.DisplayName = Entry.MorphName.ToString();
		}

		if (Entry.Category.IsNone())
		{
			Entry.Category = InferCategoryForMorphName(Entry.DisplayName);
		}

		if (Entry.Category == TEXT("Body"))
		{
			Entry.Section = InferSectionForMorphName(Entry.DisplayName);
		}
		else if (Entry.Section.IsEmpty())
		{
			Entry.Section = InferSectionForMorphName(Entry.DisplayName);
		}

		Entry.MinValue = -3.0f;
		Entry.MaxValue = 3.0f;
		Entry.DefaultValue = FMath::Clamp(Entry.DefaultValue, Entry.MinValue, Entry.MaxValue);

		USkeletalMeshComponent* TargetMeshComponent = ResolveTargetMeshComponent(Entry);
		if (!IsValid(TargetMeshComponent))
		{
			return;
		}

		const TArray<FName> MorphNames = GatherMorphNames(TargetMeshComponent);
		if (!MorphNames.Contains(Entry.MorphName))
		{
			return;
		}

		const FString MorphKey = MakeMorphKey(Entry);
		if (AddedMorphKeys.Contains(MorphKey))
		{
			return;
		}

		AddedMorphKeys.Add(MorphKey);
		AvailableMorphEntries.Add(Entry);
		CurrentMorphValues.Add(MorphKey, FMath::Clamp(TargetMeshComponent->GetMorphTarget(Entry.MorphName), Entry.MinValue, Entry.MaxValue));
	};

	for (const FMorphSliderEntry& ConfiguredEntry : Settings->MorphEntries)
	{
		TryAddEntry(ConfiguredEntry);
	}

	if (Settings->bAutoGenerateEntriesFromMesh && (Settings->bIncludeAutoDiscoveredMorphs || Settings->MorphEntries.Num() == 0))
	{
		struct FGeneratedTarget
		{
			USkeletalMeshComponent* MeshComponent = nullptr;
			ECharacterCustomizationTarget Target = ECharacterCustomizationTarget::Auto;
		};

		const TArray<FGeneratedTarget> GeneratedTargets = {
			{ BodyMeshComponent.Get(), ECharacterCustomizationTarget::Body }
		};

		for (const FGeneratedTarget& GeneratedTarget : GeneratedTargets)
		{
			if (!IsValid(GeneratedTarget.MeshComponent))
			{
				continue;
			}

			for (const FName MorphName : GatherMorphNames(GeneratedTarget.MeshComponent))
			{
				FMorphSliderEntry GeneratedEntry;
				GeneratedEntry.MorphName = MorphName;
				GeneratedEntry.DisplayName = MorphName.ToString();
				GeneratedEntry.Category = InferCategoryForMorphName(GeneratedEntry.DisplayName);
				GeneratedEntry.Section = InferSectionForMorphName(GeneratedEntry.DisplayName);
				GeneratedEntry.Target = GeneratedTarget.Target;
				GeneratedEntry.TargetComponentName = NAME_None;
				GeneratedEntry.MinValue = -3.0f;
				GeneratedEntry.MaxValue = 3.0f;
				GeneratedEntry.DefaultValue = 0.0f;
				GeneratedEntry.bAllowRandomize = true;
				TryAddEntry(GeneratedEntry);
			}
		}
	}

	SortMorphEntries();
}

void UEFCharacterCustomizationComponent::SortMorphEntries()
{
	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();
	TMap<FName, int32> CategorySortOrder;
	TMap<FString, int32> BodySectionSortOrder;
	for (const FCharacterCreationCategoryDefinition& CategoryDefinition : Settings->Categories)
	{
		CategorySortOrder.Add(CategoryDefinition.Category, CategoryDefinition.SortOrder);
	}
	BodySectionSortOrder.Add(TEXT("Upper"), 0);
	BodySectionSortOrder.Add(TEXT("Middle"), 1);
	BodySectionSortOrder.Add(TEXT("Lower"), 2);

	AvailableMorphEntries.Sort([&CategorySortOrder, &BodySectionSortOrder](const FMorphSliderEntry& Left, const FMorphSliderEntry& Right)
	{
		const int32 LeftOrder = CategorySortOrder.FindRef(Left.Category);
		const int32 RightOrder = CategorySortOrder.FindRef(Right.Category);
		if (LeftOrder != RightOrder)
		{
			return LeftOrder < RightOrder;
		}

		if (Left.Section != Right.Section)
		{
			if (Left.Category == TEXT("Body") && Right.Category == TEXT("Body"))
			{
				const int32 LeftSectionOrder = BodySectionSortOrder.FindRef(Left.Section);
				const int32 RightSectionOrder = BodySectionSortOrder.FindRef(Right.Section);
				if (LeftSectionOrder != RightSectionOrder)
				{
					return LeftSectionOrder < RightSectionOrder;
				}
			}

			return Left.Section < Right.Section;
		}

		return Left.DisplayName < Right.DisplayName;
	});
}

bool UEFCharacterCustomizationComponent::ValidateCompatibility()
{
	if (!IsValid(BodyMeshComponent) || !IsValid(BodyMeshComponent->GetSkeletalMeshAsset()))
	{
		CompatibilityError = TEXT("The mesh is not compatible with the character creation system.");
		return false;
	}

	FString FailureReason;
	const TArray<FName> MorphNames = GatherMorphNames(BodyMeshComponent.Get());
	if (!IsMeshCompatibleWithSystem(BodyMeshComponent->GetSkeletalMeshAsset(), MorphNames, FailureReason))
	{
		CompatibilityError = FailureReason;
		return false;
	}

	return true;
}

bool UEFCharacterCustomizationComponent::IsMeshCompatibleWithSystem(USkeletalMesh* SkeletalMesh, const TArray<FName>& MorphNames, FString& OutFailureReason) const
{
	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();

	if (!IsValid(SkeletalMesh) || MorphNames.Num() == 0)
	{
		OutFailureReason = TEXT("The mesh is not compatible with the character creation system.");
		return false;
	}

	const FString MeshPath = SkeletalMesh->GetPathName();
	if (MatchesAnyHint(MeshPath, Settings->DazPathTokens))
	{
		return true;
	}

	int32 MatchingMorphs = 0;
	for (const FName MorphName : MorphNames)
	{
		if (MatchesAnyHint(MorphName.ToString(), Settings->DazMorphTokens))
		{
			++MatchingMorphs;
			if (MatchingMorphs >= Settings->MinimumCompatibilityMorphMatches)
			{
				return true;
			}
		}
	}

	OutFailureReason = TEXT("The mesh is not compatible with the character creation system.");
	return false;
}

bool UEFCharacterCustomizationComponent::IsSkeletonCompatibleWithReference(USkeletalMesh* SkeletalMesh, USkeletalMesh* ReferenceMesh) const
{
	if (!IsValid(SkeletalMesh) || !IsValid(ReferenceMesh))
	{
		return false;
	}

	USkeleton* SkeletalMeshSkeleton = SkeletalMesh->GetSkeleton();
	USkeleton* ReferenceSkeleton = ReferenceMesh->GetSkeleton();
	if (!IsValid(SkeletalMeshSkeleton) || !IsValid(ReferenceSkeleton))
	{
		return false;
	}

	return SkeletalMeshSkeleton == ReferenceSkeleton;
}

TArray<FName> UEFCharacterCustomizationComponent::GatherMorphNames(USkeletalMeshComponent* MeshComponent) const
{
	return IsValid(MeshComponent) ? GatherMorphNames(MeshComponent->GetSkeletalMeshAsset()) : TArray<FName>();
}

TArray<FName> UEFCharacterCustomizationComponent::GatherMorphNames(USkeletalMesh* SkeletalMesh) const
{
	TArray<FName> MorphNames;

	if (!IsValid(SkeletalMesh))
	{
		return MorphNames;
	}

	const FString MeshKey = SkeletalMesh->GetPathName();

	if (const TArray<FName>* CachedMorphNames = MeshMorphNameCache.Find(MeshKey))
	{
		return *CachedMorphNames;
	}

	for (const TPair<FName, int32>& MorphTargetPair : SkeletalMesh->GetMorphTargetIndexMap())
	{
		MorphNames.AddUnique(MorphTargetPair.Key);
	}

	for (const TObjectPtr<UMorphTarget>& MorphTarget : SkeletalMesh->GetMorphTargets())
	{
		if (IsValid(MorphTarget))
		{
			MorphNames.AddUnique(MorphTarget->GetFName());
		}
	}

#if WITH_EDITOR
	if (USkeleton* Skeleton = SkeletalMesh->GetSkeleton())
	{
		TArray<FName> CurveNames;
		Skeleton->GetCurveMetaDataNames(CurveNames);
		for (const FName CurveName : CurveNames)
		{
			if (Skeleton->GetCurveMetaDataMorphTarget(CurveName))
			{
				MorphNames.AddUnique(CurveName);
			}
		}
	}
#endif

	if (MorphNames.Num() == 0)
	{
		GatherMorphNamesFromAssetRegistry(SkeletalMesh, MorphNames);
	}

	MorphNames.Sort(FNameLexicalLess());
	MeshMorphNameCache.Add(MeshKey, MorphNames);
	return MorphNames;
}

void UEFCharacterCustomizationComponent::GatherMorphNamesFromAssetRegistry(USkeletalMesh* SkeletalMesh, TArray<FName>& OutMorphNames) const
{
	if (!IsValid(SkeletalMesh))
	{
		return;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	FAssetData AssetData;
	const UE::AssetRegistry::EExists ExistsResult = AssetRegistryModule.Get().TryGetAssetByObjectPath(FSoftObjectPath(SkeletalMesh), AssetData);
	if (ExistsResult != UE::AssetRegistry::EExists::Exists)
	{
		return;
	}

	FString RawMorphTargetNames;
	if (!AssetData.GetTagValue(TEXT("MorphTargetNames"), RawMorphTargetNames) || RawMorphTargetNames.IsEmpty())
	{
		return;
	}

	TArray<FString> Tokens;
	RawMorphTargetNames.ParseIntoArray(Tokens, TEXT(";"), true);
	for (FString Token : Tokens)
	{
		Token = Token.TrimStartAndEnd();
		if (!Token.IsEmpty())
		{
			OutMorphNames.AddUnique(FName(Token));
		}
	}
}

FName UEFCharacterCustomizationComponent::InferCategoryForMorphName(const FString& MorphName) const
{
	if (CharacterCustomizationComponentPrivate::IsHeadMorphName(MorphName))
	{
		return TEXT("Head");
	}

	return TEXT("Body");
}

FString UEFCharacterCustomizationComponent::InferSectionForMorphName(const FString& MorphName) const
{
	if (InferCategoryForMorphName(MorphName) == TEXT("Body"))
	{
		return CharacterCustomizationComponentPrivate::InferBodySectionName(MorphName);
	}

	FString LeftSide;
	FString RightSide;
	if (MorphName.Split(TEXT(" "), &LeftSide, &RightSide))
	{
		return LeftSide;
	}

	return TEXT("General");
}

bool UEFCharacterCustomizationComponent::MatchesAnyHint(const FString& SourceString, const TArray<FString>& Hints) const
{
	const FString NormalizedSource = CharacterCustomizationComponentPrivate::NormalizeForMatching(SourceString);
	for (const FString& Hint : Hints)
	{
		if (!Hint.IsEmpty() && NormalizedSource.Contains(CharacterCustomizationComponentPrivate::NormalizeForMatching(Hint)))
		{
			return true;
		}
	}

	return false;
}

bool UEFCharacterCustomizationComponent::IsHairAssetExcluded(const FString& SourceString) const
{
	return MatchesAnyHint(SourceString, UEFCharacterCreationSettings::Get()->ExcludedHairNameTokens);
}

FString UEFCharacterCustomizationComponent::GetDisplayNameForMesh(USkeletalMesh* SkeletalMesh, const FString& PreferredLabel) const
{
	if (!PreferredLabel.IsEmpty())
	{
		return PreferredLabel;
	}

	return IsValid(SkeletalMesh) ? SkeletalMesh->GetName() : TEXT("Unavailable");
}

void UEFCharacterCustomizationComponent::AddMeshOptionIfValid(TArray<FCharacterSkeletalMeshOption>& Options, USkeletalMesh* SkeletalMesh, const FString& PreferredLabel) const
{
	if (!IsValid(SkeletalMesh))
	{
		return;
	}

	if (FindMeshOptionIndex(Options, SkeletalMesh) != INDEX_NONE)
	{
		return;
	}

	FCharacterSkeletalMeshOption Option;
	Option.DisplayName = GetDisplayNameForMesh(SkeletalMesh, PreferredLabel);
	Option.SkeletalMesh = TSoftObjectPtr<USkeletalMesh>(SkeletalMesh);
	Options.Add(Option);
}

int32 UEFCharacterCustomizationComponent::FindMeshOptionIndex(const TArray<FCharacterSkeletalMeshOption>& Options, USkeletalMesh* SkeletalMesh) const
{
	if (!IsValid(SkeletalMesh))
	{
		return INDEX_NONE;
	}

	const FSoftObjectPath MeshPath = FSoftObjectPath(SkeletalMesh);
	return Options.IndexOfByPredicate([&MeshPath](const FCharacterSkeletalMeshOption& Option)
	{
		return Option.SkeletalMesh.ToSoftObjectPath() == MeshPath;
	});
}

bool UEFCharacterCustomizationComponent::ApplyBodySkeletalMesh(USkeletalMesh* SkeletalMesh)
{
	USkeletalMeshComponent* ActiveBodySelectionComponent = IsValid(BodyMeshSelectionComponent.Get()) ? BodyMeshSelectionComponent.Get() : BodyMeshComponent.Get();
	if (!IsValid(ActiveBodySelectionComponent) || !IsValid(SkeletalMesh))
	{
		return false;
	}

	USkeletalMesh* CurrentBodyMesh = ActiveBodySelectionComponent->GetSkeletalMeshAsset();
	if (CurrentBodyMesh == SkeletalMesh)
	{
		BuildBodyMeshOptions();
		return true;
	}

	if (IsValid(CurrentBodyMesh) && !IsSkeletonCompatibleWithReference(SkeletalMesh, CurrentBodyMesh))
	{
		return false;
	}

	ActiveBodySelectionComponent->SetSkeletalMeshAsset(SkeletalMesh);
	ResetMeshComponentMaterialsToAssetDefaults(ActiveBodySelectionComponent);
	BuildBodyMeshOptions();
	BuildHairMeshOptions();

	if (ActiveBodySelectionComponent == BodyMeshComponent.Get())
	{
		InvalidateMeshDependentCaches();
		bMeshCompatible = ValidateCompatibility();
		if (bMeshCompatible)
		{
			BuildMorphEntries();
		}
		return bMeshCompatible;
	}

	return true;
}

bool UEFCharacterCustomizationComponent::ApplyHairSkeletalMesh(USkeletalMesh* SkeletalMesh)
{
	if (!IsValid(HairMeshComponent.Get()) || !IsValid(SkeletalMesh))
	{
		return false;
	}

	USkeletalMesh* CurrentHairMesh = HairMeshComponent->GetSkeletalMeshAsset();
	if (CurrentHairMesh == SkeletalMesh)
	{
		BuildHairMeshOptions();
		return true;
	}

	USkeletalMesh* ReferenceMesh = CurrentHairMesh;
	if (!IsValid(ReferenceMesh) && IsValid(BodyMeshComponent.Get()))
	{
		ReferenceMesh = BodyMeshComponent->GetSkeletalMeshAsset();
	}

	if (IsValid(ReferenceMesh) && !IsSkeletonCompatibleWithReference(SkeletalMesh, ReferenceMesh))
	{
		return false;
	}

	HairMeshComponent->SetSkeletalMeshAsset(SkeletalMesh);
	ResetMeshComponentMaterialsToAssetDefaults(HairMeshComponent.Get());
	BuildHairMeshOptions();
	return true;
}

void UEFCharacterCustomizationComponent::InvalidateMeshDependentCaches()
{
	MeshMorphNameCache.Reset();
	MorphTargetMeshComponentCache.Reset();
	DynamicMaterialInstances.Reset();
	SkinMaterialSlotCache.Reset();
	IrisMaterialSlotCache.Reset();
}

void UEFCharacterCustomizationComponent::ResetMeshComponentMaterialsToAssetDefaults(USkeletalMeshComponent* MeshComponent)
{
	if (!IsValid(MeshComponent))
	{
		return;
	}

	DynamicMaterialInstances.Remove(MeshComponent);
	SkinMaterialSlotCache.Remove(MeshComponent);
	IrisMaterialSlotCache.Remove(MeshComponent);
	MeshComponent->EmptyOverrideMaterials();
	MeshComponent->MarkRenderStateDirty();
	MeshComponent->MarkRenderDynamicDataDirty();
}

USkeletalMeshComponent* UEFCharacterCustomizationComponent::ResolveTargetMeshComponent(const FMorphSliderEntry& Entry) const
{
	if (!Entry.TargetComponentName.IsNone())
	{
		if (USkeletalMeshComponent* NamedComponent = ResolveComponentByName(Entry.TargetComponentName))
		{
			return NamedComponent;
		}
	}

	switch (Entry.Target)
	{
	case ECharacterCustomizationTarget::Body:
		return BodyMeshComponent.Get();

	case ECharacterCustomizationTarget::Clothing:
		return ClothingMeshComponents.Num() > 0 ? ClothingMeshComponents[0] : nullptr;

	case ECharacterCustomizationTarget::Auto:
	default:
		if (IsValid(BodyMeshComponent) && GatherMorphNames(BodyMeshComponent.Get()).Contains(Entry.MorphName))
		{
			return BodyMeshComponent.Get();
		}

		for (USkeletalMeshComponent* ClothingMeshComponent : ClothingMeshComponents)
		{
			if (IsValid(ClothingMeshComponent) && GatherMorphNames(ClothingMeshComponent).Contains(Entry.MorphName))
			{
				return ClothingMeshComponent;
			}
		}

		return BodyMeshComponent.Get();
	}
}

USkeletalMeshComponent* UEFCharacterCustomizationComponent::ResolveComponentByName(FName ComponentName) const
{
	for (USkeletalMeshComponent* MeshComponent : AllDiscoveredMeshComponents)
	{
		if (!IsValid(MeshComponent))
		{
			continue;
		}

		if (MeshComponent->GetFName() == ComponentName || MeshComponent->GetName().Equals(ComponentName.ToString(), ESearchCase::IgnoreCase))
		{
			return MeshComponent;
		}
	}

	return nullptr;
}

FString UEFCharacterCustomizationComponent::MakeMorphKey(const FMorphSliderEntry& Entry) const
{
	return FString::Printf(TEXT("%s|%s"), *CharacterCustomizationComponentPrivate::TargetToString(Entry.Target), *Entry.MorphName.ToString());
}

FString UEFCharacterCustomizationComponent::MakeMorphKey(const FCharacterMorphValue& MorphValue) const
{
	return FString::Printf(TEXT("%s|%s"), *CharacterCustomizationComponentPrivate::TargetToString(MorphValue.Target), *MorphValue.MorphName.ToString());
}

void UEFCharacterCustomizationComponent::SetCurrentMorphValue(const FMorphSliderEntry& Entry, float Value)
{
	CurrentMorphValues.FindOrAdd(MakeMorphKey(Entry)) = Value;
}

bool UEFCharacterCustomizationComponent::ApplyMorphValue(const FMorphSliderEntry& Entry, float Value)
{
	TArray<USkeletalMeshComponent*> TargetMeshComponents;
	GatherTargetMeshComponents(Entry, TargetMeshComponents);

	for (USkeletalMeshComponent* TargetMeshComponent : TargetMeshComponents)
	{
		ApplyMorphToMeshComponent(TargetMeshComponent, Entry.MorphName, Value);
	}

	for (USkeletalMeshComponent* TargetMeshComponent : TargetMeshComponents)
	{
		FinalizeMorphUpdate(TargetMeshComponent);
	}

	return TargetMeshComponents.Num() > 0;
}

void UEFCharacterCustomizationComponent::ApplyCurrentMorphState()
{
	if (!bMeshCompatible)
	{
		return;
	}

	TArray<USkeletalMeshComponent*> TargetMeshComponents;
	TArray<USkeletalMeshComponent*> UpdatedMeshComponents;
	for (const FMorphSliderEntry& Entry : AvailableMorphEntries)
	{
		const FString MorphKey = MakeMorphKey(Entry);
		const float StoredValue = CurrentMorphValues.Contains(MorphKey) ? CurrentMorphValues.FindRef(MorphKey) : Entry.DefaultValue;
		const float Value = FMath::Clamp(StoredValue, Entry.MinValue, Entry.MaxValue);
		CurrentMorphValues.FindOrAdd(MorphKey) = Value;

		GatherTargetMeshComponents(Entry, TargetMeshComponents);
		for (USkeletalMeshComponent* TargetMeshComponent : TargetMeshComponents)
		{
			ApplyMorphToMeshComponent(TargetMeshComponent, Entry.MorphName, Value);
			UpdatedMeshComponents.AddUnique(TargetMeshComponent);
		}
	}

	for (USkeletalMeshComponent* UpdatedMeshComponent : UpdatedMeshComponents)
	{
		FinalizeMorphUpdate(UpdatedMeshComponent);
	}
}

void UEFCharacterCustomizationComponent::GatherTargetMeshComponents(const FMorphSliderEntry& Entry, TArray<USkeletalMeshComponent*>& OutMeshComponents) const
{
	OutMeshComponents.Reset();
	const FString MorphKey = MakeMorphKey(Entry);

	if (const TArray<TWeakObjectPtr<USkeletalMeshComponent>>* CachedTargetMeshComponents = MorphTargetMeshComponentCache.Find(MorphKey))
	{
		bool bAllTargetsStillValid = true;

		for (const TWeakObjectPtr<USkeletalMeshComponent>& CachedTargetMeshComponent : *CachedTargetMeshComponents)
		{
			if (USkeletalMeshComponent* MeshComponent = CachedTargetMeshComponent.Get())
			{
				OutMeshComponents.AddUnique(MeshComponent);
			}
			else
			{
				bAllTargetsStillValid = false;
			}
		}

		if (bAllTargetsStillValid)
		{
			return;
		}

		OutMeshComponents.Reset();
	}

	auto TryAddMeshComponent = [this, &Entry, &OutMeshComponents](USkeletalMeshComponent* MeshComponent)
	{
		if (!IsValid(MeshComponent))
		{
			return;
		}

		if (!GatherMorphNames(MeshComponent).Contains(Entry.MorphName))
		{
			return;
		}

		OutMeshComponents.AddUnique(MeshComponent);
	};

	if (!Entry.TargetComponentName.IsNone())
	{
		TryAddMeshComponent(ResolveComponentByName(Entry.TargetComponentName));
	}

	switch (Entry.Target)
	{
	case ECharacterCustomizationTarget::Body:
		TryAddMeshComponent(BodyMeshComponent.Get());
		for (USkeletalMeshComponent* MeshComponent : AllDiscoveredMeshComponents)
		{
			TryAddMeshComponent(MeshComponent);
		}
		break;

	case ECharacterCustomizationTarget::Clothing:
		for (USkeletalMeshComponent* ClothingMeshComponent : ClothingMeshComponents)
		{
			TryAddMeshComponent(ClothingMeshComponent);
		}
		for (USkeletalMeshComponent* MeshComponent : AllDiscoveredMeshComponents)
		{
			TryAddMeshComponent(MeshComponent);
		}
		break;

	case ECharacterCustomizationTarget::Auto:
	default:
		for (USkeletalMeshComponent* MeshComponent : AllDiscoveredMeshComponents)
		{
			TryAddMeshComponent(MeshComponent);
		}
		break;
	}

	if (OutMeshComponents.Num() == 0)
	{
		TryAddMeshComponent(ResolveTargetMeshComponent(Entry));
	}

	TArray<TWeakObjectPtr<USkeletalMeshComponent>>& CachedTargetMeshComponents = MorphTargetMeshComponentCache.FindOrAdd(MorphKey);
	CachedTargetMeshComponents.Reset();
	for (USkeletalMeshComponent* MeshComponent : OutMeshComponents)
	{
		CachedTargetMeshComponents.Add(MeshComponent);
	}
}

void UEFCharacterCustomizationComponent::ApplyMorphToMeshComponent(USkeletalMeshComponent* MeshComponent, FName MorphName, float Value) const
{
	if (!IsValid(MeshComponent))
	{
		return;
	}

	MeshComponent->SetMorphTarget(MorphName, Value, FMath::IsNearlyZero(Value));
}

void UEFCharacterCustomizationComponent::FinalizeMorphUpdate(USkeletalMeshComponent* MeshComponent) const
{
	if (!IsValid(MeshComponent))
	{
		return;
	}

	if (MeshComponent->bPauseAnims)
	{
		MeshComponent->TickAnimation(0.0f, false);
		MeshComponent->RefreshBoneTransforms();
	}

	MeshComponent->MarkRenderDynamicDataDirty();
}

bool UEFCharacterCustomizationComponent::SavePresetData(const FCharacterPresetData& PresetData)
{
	UEFCharacterCustomizationSaveGame* SaveGame = LoadOrCreateSaveGame();
	if (!SaveGame)
	{
		return false;
	}

	SaveGame->AddOrUpdatePreset(PresetData);

	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();
	return UGameplayStatics::SaveGameToSlot(SaveGame, Settings->SaveSlotName, Settings->SaveUserIndex);
}

bool UEFCharacterCustomizationComponent::LoadPresetData(const FString& PresetName, FCharacterPresetData& OutPresetData) const
{
	if (UEFCharacterCustomizationSaveGame* SaveGame = LoadOrCreateSaveGame())
	{
		return SaveGame->TryGetPreset(PresetName, OutPresetData);
	}

	return false;
}

UEFCharacterCustomizationSaveGame* UEFCharacterCustomizationComponent::LoadOrCreateSaveGame() const
{
	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();

	if (UGameplayStatics::DoesSaveGameExist(Settings->SaveSlotName, Settings->SaveUserIndex))
	{
		return Cast<UEFCharacterCustomizationSaveGame>(UGameplayStatics::LoadGameFromSlot(Settings->SaveSlotName, Settings->SaveUserIndex));
	}

	return Cast<UEFCharacterCustomizationSaveGame>(UGameplayStatics::CreateSaveGameObject(UEFCharacterCustomizationSaveGame::StaticClass()));
}

void UEFCharacterCustomizationComponent::ApplySkinColorToMesh(USkeletalMeshComponent* MeshComponent)
{
	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();
	ApplyMaterialColorToMesh(
		MeshComponent,
		CurrentSkinColor,
		Settings->SkinColorParameterNames,
		Settings->SkinMaterialHints,
		SkinMaterialSlotCache,
		false);
}

void UEFCharacterCustomizationComponent::ApplyIrisColorToMesh(USkeletalMeshComponent* MeshComponent)
{
	const UEFCharacterCreationSettings* Settings = UEFCharacterCreationSettings::Get();
	ApplyMaterialColorToMesh(
		MeshComponent,
		CurrentIrisColor,
		Settings->IrisColorParameterNames,
		Settings->IrisMaterialHints,
		IrisMaterialSlotCache,
		true);
}

void UEFCharacterCustomizationComponent::ApplyMaterialColorToMesh(
	USkeletalMeshComponent* MeshComponent,
	const FLinearColor& Color,
	const TArray<FName>& ParameterNames,
	const TArray<FString>& MaterialHints,
	TMap<TObjectPtr<USkeletalMeshComponent>, TArray<int32>>& MaterialSlotCache,
	bool bExcludeEyeMoisture)
{
	if (!IsValid(MeshComponent) || ParameterNames.Num() == 0 || MaterialHints.Num() == 0)
	{
		return;
	}

	const TArray<int32>& MaterialSlots = ResolveCachedMaterialSlots(MeshComponent, MaterialHints, MaterialSlotCache, bExcludeEyeMoisture);
	if (MaterialSlots.Num() == 0)
	{
		return;
	}

	TArray<TObjectPtr<UMaterialInstanceDynamic>>& MaterialInstances = DynamicMaterialInstances.FindOrAdd(MeshComponent);
	if (MaterialInstances.Num() == 0)
	{
		MaterialInstances.SetNumZeroed(MeshComponent->GetNumMaterials());
	}

	for (const int32 MaterialIndex : MaterialSlots)
	{
		if (!MaterialInstances.IsValidIndex(MaterialIndex) || IsValid(MaterialInstances[MaterialIndex]))
		{
			continue;
		}

		MaterialInstances[MaterialIndex] = MeshComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
	}

	for (const int32 MaterialIndex : MaterialSlots)
	{
		if (!MaterialInstances.IsValidIndex(MaterialIndex))
		{
			continue;
		}

		UMaterialInstanceDynamic* DynamicMaterialInstance = MaterialInstances[MaterialIndex];
		if (!IsValid(DynamicMaterialInstance))
		{
			continue;
		}

		for (const FName ParameterName : ParameterNames)
		{
			DynamicMaterialInstance->SetVectorParameterValue(ParameterName, Color);
		}
	}
}

TArray<int32>& UEFCharacterCustomizationComponent::ResolveCachedMaterialSlots(
	USkeletalMeshComponent* MeshComponent,
	const TArray<FString>& MaterialHints,
	TMap<TObjectPtr<USkeletalMeshComponent>, TArray<int32>>& MaterialSlotCache,
	bool bExcludeEyeMoisture)
{
	TArray<int32>& CachedSlots = MaterialSlotCache.FindOrAdd(MeshComponent);
	if (CachedSlots.Num() > 0)
	{
		return CachedSlots;
	}

	const TArray<FName> MaterialSlotNames = MeshComponent->GetMaterialSlotNames();
	for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
	{
		const FString SlotName = MaterialSlotNames.IsValidIndex(MaterialIndex) ? MaterialSlotNames[MaterialIndex].ToString() : FString();
		UMaterialInterface* MaterialInterface = MeshComponent->GetMaterial(MaterialIndex);
		const FString MaterialName = IsValid(MaterialInterface) ? MaterialInterface->GetName() : FString();
		const FString MaterialPath = IsValid(MaterialInterface) ? MaterialInterface->GetPathName() : FString();
		const FString CombinedName = FString::Printf(TEXT("%s %s %s"), *SlotName, *MaterialName, *MaterialPath);

		if (bExcludeEyeMoisture && CombinedName.Contains(TEXT("Moisture"), ESearchCase::IgnoreCase))
		{
			continue;
		}

		if (MatchesAnyHint(CombinedName, MaterialHints))
		{
			CachedSlots.Add(MaterialIndex);
		}
	}

	return CachedSlots;
}
