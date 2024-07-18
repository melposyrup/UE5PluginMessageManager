// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PEMessageEditor : ModuleRules
{
	public PEMessageEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		
		PublicIncludePaths.AddRange(
			new string[]
			{
				"PEMessageEditor/Public"
			});

		PrivateIncludePaths.AddRange(
			new string[] 
			{
				"PEMessageEditor/Private"
			});
		
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"PEMessage"
				// ... add other public dependencies that you statically link with here ...
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"BlueprintGraph",
				"KismetCompiler",
				"UnrealEd"
				// ... add private dependencies that you statically link with here ...	
			}
		);
	}
}