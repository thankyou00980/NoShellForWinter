using UnrealBuildTool;

public class EFProjectSystemsUI : ModuleRules
{
	public EFProjectSystemsUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			ModuleDirectory
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG",
			"DeveloperSettings",
			"EFProjectSystemsCore"
		});
	}
}
