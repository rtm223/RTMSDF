// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Settings/RTMSDF_ProjectSettings.h"
#include "UObject/ObjectSaveContext.h"

URTMSDF_ProjectSettings::URTMSDF_ProjectSettings()
{
	DefaultBitmapImportSettings_SingleChannel.NumSourceChannels = 1;
	DefaultBitmapImportSettings_SingleChannel.Format = ERTMSDF_SDFFormat::SingleChannel;
	DefaultBitmapImportSettings_MultiChannel.NumSourceChannels = 4;
	DefaultBitmapImportSettings_MultiChannel.Format = ERTMSDF_SDFFormat::SeparateChannels;

#if WITH_EDITORONLY_DATA
	DefaultBitmapImportSettings_SingleChannel.bIsInProjectSettings = true;
	DefaultBitmapImportSettings_MultiChannel.bIsInProjectSettings = true;
	DefaultSVGImportSettings.bIsInProjectSettings = true;
#endif
}

void URTMSDF_ProjectSettings::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITORONLY_DATA
	LoadMaterials();
#endif

	if(DefaultBitmapImportSettings_MultiChannel.VersionNumber == 0)
	{
		DefaultBitmapImportSettings_MultiChannel = DefaultBitmapImportSettings_SingleChannel;
		DefaultBitmapImportSettings_MultiChannel.NumSourceChannels = 4;
		DefaultBitmapImportSettings_MultiChannel.Format = ERTMSDF_SDFFormat::SeparateChannels;
	}

	DefaultBitmapImportSettings_SingleChannel.FixUpVersioning();
	DefaultBitmapImportSettings_MultiChannel.FixUpVersioning();
	DefaultSVGImportSettings.FixUpVersioning();
}

#if WITH_EDITOR
void URTMSDF_ProjectSettings::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
	UObject::PostEditChangeProperty(propertyChangedEvent);

	const auto propName = propertyChangedEvent.GetPropertyName();
	const bool isMaterialProperty
		= propName == GET_MEMBER_NAME_CHECKED(ThisClass, SDFThumbnailSingleChannel)
		|| propName == GET_MEMBER_NAME_CHECKED(ThisClass, SDFThumbnailMSDF)
		|| propName == GET_MEMBER_NAME_CHECKED(ThisClass, SDFThumbnailMultichannel)
		|| propName == GET_MEMBER_NAME_CHECKED(ThisClass, SDFThumbnailPreserveRGB);

	if(isMaterialProperty)
	{
		LoadMaterials();
		// TODO - try to refresh thumbnails?

	}
}

void URTMSDF_ProjectSettings::LoadMaterials()
{
	SDFThumbnailSingleChannel_Inst = SDFThumbnailSingleChannel.LoadSynchronous();
	SDFThumbnailMSDF_Inst = SDFThumbnailMSDF.LoadSynchronous();
	SDFThumbnailMultichannel_Inst = SDFThumbnailMultichannel.LoadSynchronous();
	SDFThumbnailPreserveRGB_Inst = SDFThumbnailPreserveRGB.LoadSynchronous();
}

#endif
