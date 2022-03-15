// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDFEditor.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Config/RTMSDFConfig.h"
#include "DetailsCustomization/RTMSDF_SettingsStructCustomization.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Thumbnails/RTMSDF_ThumbnailRenderer.h"

#define LOCTEXT_NAMESPACE "RTMSDFEditorModule"
DEFINE_LOG_CATEGORY(RTMSDFEditor);

IMPLEMENT_MODULE(FRTMSDFEditorModule, RTMSDFEditor)

namespace RTMSDF
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

	RTMSDF::RegisterStructDetailsCustomization<FRTMSDF_SettingsStructCustomization, FRTMSDF_SVGImportSettings>();
	RTMSDF::RegisterStructDetailsCustomization<FRTMSDF_SettingsStructCustomization, FRTMSDF_BitmapImportSettings>();
}

#undef LOCTEXT_NAMESPACE
