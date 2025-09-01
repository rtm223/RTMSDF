// Copyright (c) Richard Meredith AB. All Rights Reserved

using UnrealBuildTool;

public class RTMSDFEditor : ModuleRules
{
	public RTMSDFEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"ChlumskyMSDFGen",
				"Engine",
				"EditorSubsystem",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"RHI",
				"PropertyEditor",
				"RTMSDF",
				"InterchangeCore",
			});
	}
}