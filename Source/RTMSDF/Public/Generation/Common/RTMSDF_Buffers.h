// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

enum ETextureSourceFormat : int;

namespace RTM::SDF
{
	struct FSDFBufferDef
	{
		uint16 Width = 0;
		uint16 Height = 0;
		uint8 NumChannels = 0;
		ETextureSourceFormat Format = static_cast<ETextureSourceFormat>(0); // TSF_Invalid;

		size_t GetBufferLen() const { return Width * Height * NumChannels; }	// todo - * type ?
	};

	struct FSDFBufferMapping
	{
		uint8 SourceChannel = 0;
		uint8 TargetChannel = 0;
		bool bInvertDistance = false;
		bool bTileX = false;
		bool bTileY = false;
		float DistanceRangeNormalized = 0.0f;
		float Scale = 1.0f;

		FSDFBufferMapping(uint8 sourceChannel, uint8 targetChannel, float distanceRangeNormalized, bool tileX = false, bool tileY = false, float scale = 1.0f, bool invertDistance = false)
			: SourceChannel(sourceChannel)
			, TargetChannel(targetChannel)
			, bInvertDistance(invertDistance)
			, bTileX(tileX)
			, bTileY(tileY)
			, DistanceRangeNormalized(distanceRangeNormalized)
			, Scale(scale)
		{}
	};
}
