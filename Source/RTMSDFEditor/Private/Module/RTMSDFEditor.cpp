// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDFEditor.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Config/RTMSDFConfig.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Thumbnails/RTMSDF_ThumbnailRenderer.h"

#define LOCTEXT_NAMESPACE "RTMSDFEditorModule"
DEFINE_LOG_CATEGORY(RTMSDFEditor);

IMPLEMENT_MODULE(FRTMSDFEditorModule, RTMSDFEditor)

void FRTMSDFEditorModule::StartupModule()
{
	if(auto* settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsSectionPtr settingsSection = settingsModule->RegisterSettings("Project", "Plugins", "RTM SDF",
			LOCTEXT("SettingsName", "RTM SDF"),
			LOCTEXT("SettingsDescription", "Configure Defaults for newly imported .svg to SDF Textures"),
			GetMutableDefault<URTMSDFConfig>());
	}

	auto& tnManager = UThumbnailManager::Get();
	tnManager.UnregisterCustomRenderer(UTexture2D::StaticClass());
	tnManager.RegisterCustomRenderer(UTexture2D::StaticClass(), URTMSDF_ThumbnailRenderer::StaticClass());

	// TODO - reimplement in module to make slightly nicer
	// Commented out to remove dependency on RTMCommon
	//RegisterStructDetailsCustomization<FRTMInlineStructCustomization, FRTMSDF_SVGImportSettings>();
	//RegisterStructDetailsCustomization<FRTMInlineStructCustomization, FRTMSDF_BitmapImportSettings>();
}

#undef LOCTEXT_NAMESPACE
