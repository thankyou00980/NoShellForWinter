using UnrealBuildTool;

public class EFCharacterCreationRuntime : ModuleRules
{
	public EFCharacterCreationRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AssetRegistry",
			"DeveloperSettings",
			"InputCore",
			"Slate",
			"SlateCore",
			"UMG"
		});
	}
}
