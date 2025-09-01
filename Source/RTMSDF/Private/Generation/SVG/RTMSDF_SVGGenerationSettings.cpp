// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Generation/SVG/RTMSDF_SVGGenerationSettings.h"
#include "Module/RTMSDF.h"

void FRTMSDF_SVGGenerationSettings::FixUpVersioning()
{
	if(VersionNumber >= CurrentVersionNumber)
		return;

	UE_LOG(RTMSDF, Log, TEXT("%s : Fixing up asset version number %d -> %d"), ANSI_TO_TCHAR(__FUNCTION__), VersionNumber, CurrentVersionNumber);

	if(VersionNumber < 1 && !bIsInProjectSettings)
		bScaleToFitDistance = false;

	VersionNumber = CurrentVersionNumber;
	return;
}
