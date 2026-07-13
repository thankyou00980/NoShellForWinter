using System.IO;
using UnrealBuildTool;

public class EFProjectSystemsEditor : ModuleRules
{
	public EFProjectSystemsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			ModuleDirectory,
			Path.Combine(ModuleDirectory, "..", "EFProjectSystemsGameplay"),
			Path.Combine(ModuleDirectory, "..", "EFProjectSystemsCore")
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"AscentCombatFramework",
			"EFLevelFlowRuntime",
			"EFProjectSystemsCore",
			"EFProjectSystemsGameplay",
			"EFProceduralEditor"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"Json",
			"JsonUtilities",
			"BlueprintGraph",
			"AnimGraph",
			"AnimationBlueprintLibrary"
		});
	}
}
