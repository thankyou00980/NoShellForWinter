#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

struct FEFProjectAssetPathResolver
{
	static constexpr TCHAR LegacyAnimationRoot[] = TEXT("/Game/KawaiiAnimations/Exported2/");
	static constexpr TCHAR CurrentAnimationRoot[] = TEXT("/Game/ExportedAnimations/");

	static FString RemapLegacyPathString(const FString& ObjectPath)
	{
		if (ObjectPath.IsEmpty())
		{
			return ObjectPath;
		}

		FString Remapped = ObjectPath;
		Remapped.ReplaceInline(LegacyAnimationRoot, CurrentAnimationRoot, ESearchCase::IgnoreCase);
		return Remapped;
	}

	static FSoftObjectPath RemapLegacyPath(const FSoftObjectPath& ObjectPath)
	{
		if (!ObjectPath.IsValid())
		{
			return ObjectPath;
		}

		const FString OriginalPath = ObjectPath.ToString();
		const FString RemappedPath = RemapLegacyPathString(OriginalPath);
		return RemappedPath == OriginalPath ? ObjectPath : FSoftObjectPath(RemappedPath);
	}

	static FSoftObjectPath BuildExportedAnimationPath(const FString& AssetName)
	{
		return AssetName.IsEmpty()
			? FSoftObjectPath()
			: FSoftObjectPath(FString::Printf(TEXT("%s%s.%s"), CurrentAnimationRoot, *AssetName, *AssetName));
	}

	template <typename TObjectType>
	static TObjectType* LoadObjectWithLegacyFallback(const FSoftObjectPath& AssetPath)
	{
		if (!AssetPath.IsValid())
		{
			return nullptr;
		}

		if (TObjectType* Loaded = Cast<TObjectType>(AssetPath.TryLoad()))
		{
			return Loaded;
		}

		const FSoftObjectPath RemappedPath = RemapLegacyPath(AssetPath);
		return RemappedPath == AssetPath ? nullptr : Cast<TObjectType>(RemappedPath.TryLoad());
	}
};
