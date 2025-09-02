// Copyright (c) Richard Meredith AB. All Rights Reserved

using UnrealBuildTool;

public class RTMSDF : ModuleRules
{
	public RTMSDF(ReadOnlyTargetRules Target) : base(Target)
	{
		bool testIWYU = false;
		PCHUsage = testIWYU ? PCHUsageMode.NoPCHs : PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = !testIWYU;

		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			});
	}
}