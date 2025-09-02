// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Generation/Bitmap/RTMSDF_BitmapGeneration.h"
#include "Generation/Bitmap/RTMSDF_QuadTree.h"
#include "Generation/Common/RTMSDF_Buffers.h"
#include "Logging/LogMacros.h"
#include "Misc/ScopeExit.h"
#include "Module/RTMSDF.h"
#include "Async/ParallelFor.h"

#define TREESEARCH_THREAD_ROWS 0 // 1 to put each row of the SDF on a thread, vs each pixel

namespace RTM::SDF
{
	namespace Internal
	{
		static FVector2f TransformPos(float fromWidth, float fromHeight, float toWidth, float toHeight, const FVector2f& fromVec, float toScale)
		{
			const FVector2f toCenter = (FVector2f(toWidth, toHeight) - 1.0f) * 0.5f;
			const FVector2f fromCenter = (FVector2f(fromWidth, fromHeight) - 1.0f) * 0.5f;
			const FVector2f fromPos = fromVec - fromCenter;
			const FVector2f toPos = fromPos * FVector2f(toWidth / fromWidth, toHeight / fromHeight) * toScale;
			return toCenter + toPos;
		}

		static FVector2f TransformPos(const FSDFBufferDef& fromBufferDef, const FSDFBufferDef& toBufferDef, const FVector2f& fromVec, float toScale)
		{
			return TransformPos(fromBufferDef.Width, fromBufferDef.Height, toBufferDef.Width, toBufferDef.Height, fromVec, toScale);
		}

		static uint8 ComputePixelValue(FVector2f pos, int width, int height, const uint8* buffer, int pixelWidth, int channelOffset)
		{
			auto index = [width, pixelWidth, channelOffset](int x, int y) { return (y * width + x) * pixelWidth + channelOffset; };
			pos = FVector2f(FMath::Clamp(pos.X, 0.0f, width - 1.0f), FMath::Clamp(pos.Y, 0.0f, height - 1.0f));
			const int top = FMath::FloorToInt(pos.Y);
			const int left = FMath::FloorToInt(pos.X);
			const int bottom = FMath::Min(top + 1, height - 1);
			const int right = FMath::Min(left + 1, width - 1);
			const float bottomWeight = pos.Y - top;
			const float rightWeight = pos.X - left;

			const uint8 lt = buffer[index(left, top)];
			const uint8 rt = buffer[index(right, top)];
			const uint8 lb = buffer[index(left, bottom)];
			const uint8 rb = buffer[index(right, bottom)];

			const float topVal = lt * (1.0f - rightWeight) + rt * rightWeight;
			const float bottomVal = lb * (1.0f - rightWeight) + rb * rightWeight;
			return static_cast<uint8>(FMath::RoundToInt(topVal * (1.0f - bottomWeight) + bottomVal * bottomWeight));
		}

		static uint8 ComputePixelValue(FVector2f pos, const uint8* buffer, const FSDFBufferDef& bufferDef, int channelOffset)
		{
			return ComputePixelValue(pos, bufferDef.Width, bufferDef.Height, buffer, bufferDef.NumChannels, channelOffset);
		}
	}

	void CreateDistanceField(uint8* const sourceBuffer, const FSDFBufferDef& sourceBufferDef, const float* intersectionBuffer, const FSDFBufferDef& intersectionBufferDef, int numIntersections, uint8* outSDFBuffer, const FSDFBufferDef& sdfBufferDef, const FSDFBufferMapping& mapping)
	{
		FQuadTree tree(sourceBufferDef.Width, sourceBufferDef.Height, numIntersections, numIntersections * 2);
		PopulateEdgeTree(intersectionBuffer, intersectionBufferDef, tree);
		FindDistances(tree, sourceBuffer, sourceBufferDef, outSDFBuffer, sdfBufferDef, mapping);

		// TODO - remove this once we're sure that branch prediction isn't working
		int hits = tree.BranchHits;
		int misses = tree.BranchMisses;
		UE_LOG(RTMSDF, Verbose, TEXT("Branch Hits/Misses %d / %d (%.02f hitrate)"), hits, misses, (hits+misses > 0) ? (hits*100.0f) / (hits+misses) : 0.0f);
	}

	void SetChannelUniformValue(uint8* buffer, const FSDFBufferDef& bufferDef, uint8 channelOffset, uint8 value)
	{
		const int bufferLen = bufferDef.GetBufferLen();
		for(int i = 0; i < bufferLen; i += bufferDef.NumChannels)
			buffer[i + channelOffset] = value;
	}

	void CopyChannelValues(uint8* sourceBuffer, const FSDFBufferDef& sourceBufferDef, uint8 sourceChannelOffset, uint8* targetBuffer, const FSDFBufferDef& targetBufferDef, uint8 targetChannelOffset)
	{
		check(sourceBufferDef.Width == targetBufferDef.Width);
		check(sourceBufferDef.Height == targetBufferDef.Height);
		const int numPixels = sourceBufferDef.Width * sourceBufferDef.Height;
		for(int i = 0; i < numPixels; ++i)
			targetBuffer[i * targetBufferDef.NumChannels + targetChannelOffset] = sourceBuffer[i * sourceBufferDef.NumChannels + sourceChannelOffset];
	}

	bool CreateDistanceField(uint8* const sourceBuffer, const FSDFBufferDef& sourceBufferDef, uint8* outSDFBuffer, const FSDFBufferDef& sdfBufferDef, const FSDFBufferMapping& mapping)
	{
		FSDFBufferDef intersectionBufferDef(sourceBufferDef.Width, sourceBufferDef.Height, 1);

		float* const sourceIntersections = static_cast<float*>(FMemory::Malloc(intersectionBufferDef.GetBufferLen() * 2 * sizeof(float)));
		ON_SCOPE_EXIT { FMemory::Free(sourceIntersections); };

		uint32 numIntersections = 0;

		const bool foundIntersections = FindIntersections(sourceBuffer, sourceBufferDef, sourceIntersections, intersectionBufferDef, mapping.SourceChannel, numIntersections);
		if(foundIntersections)
			CreateDistanceField(sourceBuffer, sourceBufferDef, sourceIntersections, intersectionBufferDef, numIntersections, outSDFBuffer, sdfBufferDef, mapping);

		return foundIntersections;
	}

	bool FindIntersections(uint8* const sourceBuffer, const FSDFBufferDef& sourceBufferDef, float* outIntersectionBuffer, const FSDFBufferDef& intersectionBufferDef, int channelOffset, uint32& outNumIntersections)
	{
		const uint64 cyclesStart = FPlatformTime::Cycles();
		std::atomic_uint32_t numFound = false;
		ParallelFor(intersectionBufferDef.Height, [&](const int eY)
		{
			const bool bottomRow = eY == intersectionBufferDef.Height - 1;
			for(int eX = 0; eX < intersectionBufferDef.Width; eX++)
			{
				// NOTE: When we come back to wrapping this stuff, might be worth having the invalid row / column either
				//	- For wrapped, read on the other side
				//	- For non-wrapped, read the current pixel (will create a 0 denominator and thus no edge
				//	Should be cleaner

				const bool rightmostCol = eX == intersectionBufferDef.Width - 1;

				const int currPixIdx = eY * sourceBufferDef.Width + eX;
				const int currValueIdx = currPixIdx * sourceBufferDef.NumChannels + channelOffset;
				const int rightValueIdx = (currPixIdx + 1) * sourceBufferDef.NumChannels + channelOffset;
				const int downValueIdx = (currPixIdx + sourceBufferDef.Width) * sourceBufferDef.NumChannels + channelOffset;

				const int intersectionRightIdx = (eY * intersectionBufferDef.Width + eX) * 2;
				const int intersectionDownIdx = intersectionRightIdx + 1;

				const uint8 currValue = sourceBuffer[currValueIdx];
				const uint8 rightPixValue = sourceBuffer[rightValueIdx];
				const uint8 downPixValue = sourceBuffer[downValueIdx];

				// TODO - maybe keep demoninators and numerators as uint8 until we reach the division
				const float numerator = (127 - currValue);
				const float denominatorX = rightmostCol ? 0.0f : static_cast<float>(rightPixValue - currValue);
				const float denominatorY = bottomRow ? 0.0f : static_cast<float>(downPixValue - currValue);
				const float intersectionTop = denominatorX != 0.0f ? numerator / denominatorX : -FLT_MAX;
				const float intersectionLeft = denominatorY != 0.0f ? numerator / denominatorY : -FLT_MAX;

				outIntersectionBuffer[intersectionRightIdx] = intersectionTop > 1.0f ? -FLT_MAX : intersectionTop;
				outIntersectionBuffer[intersectionDownIdx] = intersectionLeft > 1.0f ? -FLT_MAX : intersectionLeft;

				if(outIntersectionBuffer[intersectionRightIdx] >= 0.0f)
					++numFound;

				if(outIntersectionBuffer[intersectionDownIdx] >= 0.0f)
					++numFound;
			}
		});

		const uint64 cyclesEnd = FPlatformTime::Cycles();

		outNumIntersections = numFound;
		UE_LOG(RTMSDF, Verbose, TEXT("Num Intersections[%d] = %d (%.2fms)"), channelOffset, outNumIntersections, FPlatformTime::ToMilliseconds(cyclesEnd-cyclesStart));
		return numFound > 1;
	}

	bool PopulateEdgeTree(const float* intersectionBuffer, const FSDFBufferDef& intersectionBufferDef, FQuadTree& tree)
	{
		const uint64 cyclesStart = FPlatformTime::Cycles();

		for(int y = 0; y < intersectionBufferDef.Height; y++)
		{
			for(int x = 0; x < intersectionBufferDef.Width; x++)
			{
				const int currIdx = (y * intersectionBufferDef.Width + x) * 2;
				const int nextColIdxUnsafe = (y * intersectionBufferDef.Width + (x + 1)) * 2;		// are these really unsafe?
				const int nextRowIdxUnsafe = ((y + 1) * intersectionBufferDef.Width + x) * 2;		// TODO - should be possible to make this whole thing safe

				const float topIntersection = intersectionBuffer[currIdx];
				const float leftIntersection = intersectionBuffer[currIdx + 1];
				const float rightIntersection = (x < intersectionBufferDef.Width - 1) ? intersectionBuffer[nextColIdxUnsafe + 1] : -1.0f;
				const float bottomIntersection = (y < intersectionBufferDef.Height - 1) ? intersectionBuffer[nextRowIdxUnsafe] : -1.0f;

				TArray<FVector2f, TFixedAllocator<4>> intersections;

				if(topIntersection >= 0.0f)
					intersections.Add(FVector2f(x + topIntersection, y));

				if(bottomIntersection >= 0.0f)
					intersections.Add(FVector2f(x + bottomIntersection, y + 1));

				if(leftIntersection > 0.0f && leftIntersection < 1.0f)
					intersections.Add(FVector2f(x, y + leftIntersection));

				if(rightIntersection > 0.0f && rightIntersection < 1.0f)
					intersections.Add(FVector2f(x + 1, y + rightIntersection));

				const int numPoints = intersections.Num();
				if(numPoints >= 2)
				{
					tree.AddItem(intersections[0], intersections[1]);
				}
				if(numPoints == 4)
				{
					tree.AddItem(intersections[2], intersections[3]);
				}
			}
		}

		const uint64 cyclesEnd = FPlatformTime::Cycles();
		const int numEdges = tree.Edges.Num();
		UE_LOG(RTMSDF, Verbose, TEXT("Num Edges/Cells %d/%d (%.2fms)"), numEdges, tree.Cells.Num(), FPlatformTime::ToMilliseconds(cyclesEnd-cyclesStart));
		UE_LOG(RTMSDF, VeryVerbose, TEXT("%s"), *tree.ToString());

		return numEdges > 0;
	}

	void FindDistances(const FQuadTree& tree, const uint8* sourceBuffer, const FSDFBufferDef& sourceBufferDef, uint8* outSDFBuffer, const FSDFBufferDef& sdfBufferDef, const FSDFBufferMapping& mapping)
	{
		const uint64 cyclesStart = FPlatformTime::Cycles();

		const float halfFieldDistance = mapping.DistanceRangeNormalized * FMath::Min(sourceBufferDef.Width, sourceBufferDef.Height) * mapping.Scale;
		const float searchRange = halfFieldDistance * 2.0f;

		TArray<FVector2f> searchZones = {{0.0f, 0.0f}};
		if(mapping.bTileX)
			searchZones.Append({{-1.0f, 0.0f}, {1.0f, 0.0f}});

		if(mapping.bTileY)
			searchZones.Append({{0.0f, -1.0f}, {0.0f, 1.0f}});

		if(mapping.bTileX && mapping.bTileY)
			searchZones.Append({{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, 1.0f}});

		FVector2f zoneOffset = FVector2f(sourceBufferDef.Width, sourceBufferDef.Height) * mapping.Scale;

		// NOTE - seems to be little to no difference to splitting threads by rows or pixels
#if TREESEARCH_THREAD_ROWS
		ParallelFor(sdfBufferDef.Height, [&](const int y)
		{
			for(int x = 0; x<sdfBufferDef.Width; ++x)
			{
				const int i = y * sdfBufferDef.Width + x;
				const FVector2f sdfPos(x, y);
#else
		ParallelFor(sdfBufferDef.Width * sdfBufferDef.Height, [&](const int i)
		{
			{
				const FVector2f sdfPos(i % sdfBufferDef.Width, i / sdfBufferDef.Width);
#endif
				const FVector2f sourcePos = Internal::TransformPos(sdfBufferDef, sourceBufferDef, sdfPos, mapping.Scale);

				FSearchContext search(sourcePos, halfFieldDistance);

				for(auto& zone : searchZones)
				{
					search = FSearchContext(sourcePos + zone * zoneOffset, FMath::Sqrt(search.ClosestDistanceSq));
					tree.FindDistance(search);
				}

				const uint8 mipVal8 = Internal::ComputePixelValue(sourcePos, sourceBuffer, sourceBufferDef, mapping.SourceChannel);
				const bool outside = mipVal8 < 127;
				const float dist = FMath::Sqrt(search.ClosestDistanceSq);
				const float signedDist = (outside ^ mapping.bInvertDistance) ? dist : -dist;
				const float distN = signedDist / searchRange + 0.5f;
				const uint8 sdfMip = distN >= halfFieldDistance ? 255 : FMath::Clamp(FMath::FloorToInt(distN * 255.0f), 0, 255);

				const int sdfPixelIdx = i * sdfBufferDef.NumChannels + mapping.TargetChannel;
				outSDFBuffer[sdfPixelIdx] = sdfMip;
			}
		});

		const uint64 cyclesEnd = FPlatformTime::Cycles();
		UE_LOG(RTMSDF, Verbose, TEXT("Populated Distance Field from QuadTree %d pixels (%.2fms)"), sdfBufferDef.Width * sdfBufferDef.Height, FPlatformTime::ToMilliseconds(cyclesEnd-cyclesStart));
	}
}
