// Copyright (c) Richard Meredith AB. All Rights Reserved

using System.IO;
using UnrealBuildTool;

public class ChlumskyMSDFGen : ModuleRules
{
	public ChlumskyMSDFGen(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp17;

		PublicDependencyModuleNames.Add("Core");

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"ChlumskyMSDFGen/Public/Freetype/include/",
				"ChlumskyMSDFGen/Public/Core/",
				"ChlumskyMSDFGen/Public/Ext/",
			});

		PrivateDefinitions.AddRange(
			new string[]
			{
				"MSDFGEN_USE_CPP11",
				"_CRT_SECURE_NO_WARNINGS"
			});

		if (UnrealTargetPlatform.Win64 == Target.Platform)
			PrivateDefinitions.Add("MSDFGEN_USE_CRT_SECURE");

		var freetypeLibPath = Path.Combine(ModuleDirectory, "Public", "Freetype");
		if (UnrealTargetPlatform.Win64 == Target.Platform || UnrealTargetPlatform.Linux == Target.Platform ||
		    UnrealTargetPlatform.Mac == Target.Platform)
			PublicAdditionalLibraries.Add(Path.Combine(freetypeLibPath, "win64", "freetype.lib"));
	}
}