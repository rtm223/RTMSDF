// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDFEditor.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "DetailsCustomization/RTMSDF_SettingsStructCustomization.h"
#include "Settings/RTMSDF_PerUserEditorSettings.h"
#include "Settings/RTMSDF_ProjectSettings.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Thumbnails/RTMSDF_ThumbnailRenderer.h"

#define LOCTEXT_NAMESPACE "RTMSDFEditorModule"
DEFINE_LOG_CATEGORY(RTMSDFEditor);

IMPLEMENT_MODULE(FRTMSDFEditorModule, RTMSDFEditor)

namespace RTM::SDF::EditorModuleStatics
{
	// Copied across from RTMCommonEditor, RTMDetailsCustomizationHelpers

	template<typename TCustomization>
	void RegisterDetailsCustomization(const FName& propertyTypeName)
	{
		auto propertyTypeLayoutDelegate = FOnGetPropertyTypeCustomizationInstance::CreateStatic([]() { return TSharedRef<IPropertyTypeCustomization>(MakeShareable(new TCustomization)); });

		FPropertyEditorModule& propertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		propertyModule.RegisterCustomPropertyTypeLayout(propertyTypeName, propertyTypeLayoutDelegate);
	}

	template<typename TCustomization, typename TStruct> void RegisterStructDetailsCustomization() { RegisterDetailsCustomization<TCustomization>(TStruct::StaticStruct()->GetFName()); }
}

void FRTMSDFEditorModule::StartupModule()
{
	using namespace RTM::SDF::EditorModuleStatics;
	if(auto* settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsSectionPtr settingsSection = settingsModule->RegisterSettings("Project", "Plugins", "RTMSDF",
			LOCTEXT("SettingsName", "RTMSDF"),
			LOCTEXT("SettingsDescription", "Configure Defaults for newly imported .svg to SDF Textures"),
			GetMutableDefault<URTMSDF_ProjectSettings>());

		auto* editorSettings =GetMutableDefault<URTMSDF_PerUserEditorSettings>();
		// TODO - reinvestigate if we can get per user settings working by manually iterating through the GPluginLayers
		// NOTE: The code here successfully loads from the plugin per-user settings, but actually needs to walk the hierarchy to work properly
		// const FString pluginDir = FPaths::ConvertRelativePathToFull(*IPluginManager::Get().FindPlugin(UE_PLUGIN_NAME)->GetBaseDir());
		// 	const FString iniPath = FPaths::Combine(pluginDir, TEXT("Config"), TEXT("DefaultEditorPerProjectUserSettings.ini"));
		// 	editorSettings->LoadConfig(nullptr, *iniPath);
		// for(int i=0; i<UE_ARRAY_COUNT(GPluginLayers);++i)
		// {
		// }
		ISettingsSectionPtr editorSettingsSection = settingsModule->RegisterSettings("Editor", "Plugins", "RTMSDF",
			LOCTEXT("EditorSettingsName", "RTMSDF Editor Settings"),
			LOCTEXT("EditorSettingsDescription", "Setup editor behaviour for RTMSDF Plugin"),
			editorSettings);
	}

	auto& tnManager = UThumbnailManager::Get();
	tnManager.UnregisterCustomRenderer(UTexture2D::StaticClass());
	tnManager.RegisterCustomRenderer(UTexture2D::StaticClass(), URTMSDF_ThumbnailRenderer::StaticClass());

	RegisterStructDetailsCustomization<FRTMSDF_SettingsStructCustomization, FRTMSDF_SVGGenerationSettings>();
	RegisterStructDetailsCustomization<FRTMSDF_SettingsStructCustomization, FRTMSDF_BitmapGenerationSettings>();
}

#undef LOCTEXT_NAMESPACE
