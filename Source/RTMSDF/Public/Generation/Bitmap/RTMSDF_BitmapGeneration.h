// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once
#include "CoreMinimal.h"

namespace RTM::SDF
{
	struct FSDFBufferDef;
	struct FSDFBufferMapping;
	struct FQuadTree;

	RTMSDF_API void SetChannelUniformValue(uint8* buffer, const FSDFBufferDef& bufferDef, uint8 channelOffset, uint8 value);
	RTMSDF_API void CopyChannelValues(uint8* sourceBuffer, const FSDFBufferDef& sourceBufferDef, uint8 sourceChannelOffset, uint8* targetBuffer, const FSDFBufferDef& targetBufferDef, uint8 targetChannelOffset);
	RTMSDF_API bool CreateDistanceField(uint8* const sourceBuffer, const FSDFBufferDef& sourceBufferDef, uint8* outSDFBuffer, const FSDFBufferDef& sdfBufferDef, const FSDFBufferMapping& mapping);
	RTMSDF_API bool FindIntersections(uint8* const sourceBuffer, const FSDFBufferDef& sourceBufferDef, float* outIntersectionBuffer, const FSDFBufferDef& intersectionBufferDef, int channelOffset, uint32& outNumIntersections);
	RTMSDF_API bool PopulateEdgeTree(const float* intersectionBuffer, const FSDFBufferDef& intersectionBufferDef, FQuadTree& tree);
	RTMSDF_API void FindDistances(const FQuadTree& tree, const uint8* sourceBuffer, const FSDFBufferDef& sourceBufferDef, uint8* outSDFBuffer, const FSDFBufferDef& sdfBufferDef, const FSDFBufferMapping& mapping);
}
