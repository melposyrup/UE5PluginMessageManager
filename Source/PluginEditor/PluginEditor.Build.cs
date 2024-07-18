// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PluginEditor : ModuleRules
{
	public PluginEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"PEMessage"
			
		});
	}
}
