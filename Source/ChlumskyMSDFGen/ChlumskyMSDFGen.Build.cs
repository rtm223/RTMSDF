// Copyright (c) Richard Meredith AB. All Rights Reserved

using UnrealBuildTool;
using EpicGames.Core;
using UnrealBuildBase;

public class ChlumskyMSDFGen : ModuleRules
{
	public ChlumskyMSDFGen(ReadOnlyTargetRules Target) : base(Target)
	{
		bool testIWYU = false;
		PCHUsage = testIWYU ? PCHUsageMode.NoPCHs : PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = !testIWYU;

		CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.Add("Core");

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"ChlumskyMSDFGen/Public/Core/",
				"ChlumskyMSDFGen/Public/Ext/",
			});

		PrivateDefinitions.AddRange(
			new string[]
			{
				"MSDFGEN_USE_CPP11",
				"_CRT_SECURE_NO_WARNINGS",
				"MSDFGEN_USE_DROPXML",
				"M_PI=3.14159265358979323846"
			});

		// SKIA only available on windows for now
		if(Target.Platform == UnrealTargetPlatform.Win64)
			PrivateDefinitions.Add("MSDFGEN_USE_SKIA");

		/* Tests for MSDF and SKIA in source.
		 * Currently we can't do much about this, as the source versions are included via cpp and not distributed with launcher builds, so we maintain our own versions */
		FileReference msdfSourcePath = FileReference.Combine(Unreal.RootDirectory, "Engine", "Source", "ThirdParty", "msdfgen", "msdfgen.h");
		if (FileReference.Exists(msdfSourcePath))
			PublicDefinitions.Add("WITH_MSDF_SOURCE=1");
		else
			PublicDefinitions.Add("WITH_MSDF_SOURCE=0");

		FileReference skiaSourcePath = FileReference.Combine(Unreal.RootDirectory, "Engine", "Source", "ThirdParty", "skia", "skia-simplify.h");
		if (FileReference.Exists(skiaSourcePath))
			PublicDefinitions.Add("WITH_SKIA_SOURCE=1");
		else
			PublicDefinitions.Add("WITH_SKIA_SOURCE=0");
	}
}