// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Settings/RTMSDF_ProjectSettings.h"
#include "Materials/MaterialInterface.h"

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