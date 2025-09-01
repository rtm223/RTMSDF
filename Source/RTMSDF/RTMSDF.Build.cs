// Copyright (c) Richard Meredith AB. All Rights Reserved

using UnrealBuildTool;

public class RTMSDF : ModuleRules
{
	public RTMSDF(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;

		bUseUnity = false;
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}