// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Generation/SVG/RTMSDF_SVGGenerationAssetData.h"

void URTMSDF_SVGGenerationAssetData::PostLoad()
{
	Super::PostLoad();

	GenerationSettings.FixUpVersioning();
}