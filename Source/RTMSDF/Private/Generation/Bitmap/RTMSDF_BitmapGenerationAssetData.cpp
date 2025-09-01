// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Generation/Bitmap/RTMSDF_BitmapGenerationAssetData.h"

void URTMSDF_BitmapGenerationAssetData::PostLoad()
{
	Super::PostLoad();

	GenerationSettings.FixUpVersioning();
}