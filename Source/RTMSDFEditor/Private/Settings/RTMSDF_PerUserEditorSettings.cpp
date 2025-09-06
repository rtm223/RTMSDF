// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Settings/RTMSDF_PerUserEditorSettings.h"

void URTMSDF_PerUserEditorSettings::PostInitProperties()
{
	Super::PostInitProperties();
	LoadMaterials();
}

void URTMSDF_PerUserEditorSettings::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
	Super::PostEditChangeProperty(propertyChangedEvent);

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

void URTMSDF_PerUserEditorSettings::LoadMaterials()
{
	SDFThumbnailSingleChannel_Inst = SDFThumbnailSingleChannel.LoadSynchronous();
	SDFThumbnailMSDF_Inst = SDFThumbnailMSDF.LoadSynchronous();
	SDFThumbnailMultichannel_Inst = SDFThumbnailMultichannel.LoadSynchronous();
	SDFThumbnailPreserveRGB_Inst = SDFThumbnailPreserveRGB.LoadSynchronous();
}
