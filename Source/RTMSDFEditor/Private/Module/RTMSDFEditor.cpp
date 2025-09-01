// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDFEditor.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Settings/RTMSDF_ProjectSettings.h"
#include "DetailsCustomization/RTMSDF_SettingsStructCustomization.h"
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
		ISettingsSectionPtr settingsSection = settingsModule->RegisterSettings("Project", "Plugins", "RTM SDF",
			LOCTEXT("SettingsName", "RTM SDF"),
			LOCTEXT("SettingsDescription", "Configure Defaults for newly imported .svg to SDF Textures"),
			GetMutableDefault<URTMSDF_ProjectSettings>());
	}

	auto& tnManager = UThumbnailManager::Get();
	tnManager.UnregisterCustomRenderer(UTexture2D::StaticClass());
	tnManager.RegisterCustomRenderer(UTexture2D::StaticClass(), URTMSDF_ThumbnailRenderer::StaticClass());

	RegisterStructDetailsCustomization<FRTMSDF_SettingsStructCustomization, FRTMSDF_SVGGenerationSettings>();
	RegisterStructDetailsCustomization<FRTMSDF_SettingsStructCustomization, FRTMSDF_BitmapGenerationSettings>();
}

#undef LOCTEXT_NAMESPACE
