// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once
#include "CoreMinimal.h"

namespace RTM::SDF
{
	struct FCell
	{
		int16 MinX = 0;
		int16 MinY = 0;
		int16 Size = 0;		// TODO - consider if size should be unsigned?
		bool bChildCells = false;
		uint8 NumChildren = 0;

		uint16 Children[4] = {INVALID, INVALID, INVALID, INVALID};

		static constexpr uint16 INVALID = ~0;

		constexpr FCell(int16 minX, int16 minY, int16 size)
			: MinX(minX)
			, MinY(minY)
			, Size(size)
		{}

	private:
		FCell() = default;
	};

	struct FEdgeData
	{
		uint16 MinX = 0;
		uint16 MinY = 0;
		uint16 MaxX = 0;
		uint16 MaxY = 0;
		FVector2f P1 = FVector2f(ForceInitToZero);
		FVector2f P2 = FVector2f(ForceInitToZero);

		FEdgeData(const FVector2f& p1, const FVector2f& p2)
			: P1(p1)
			, P2(p2)
		{
			const FVector2f min = FVector2f::Min(p1, p2);
			const FVector2f max = FVector2f::Max(p1, p2);
			MinX = FMath::FloorToInt(min.X);
			MinY = FMath::FloorToInt(min.Y);
			MaxX = FMath::CeilToInt(max.X);
			MaxY = FMath::CeilToInt(max.Y);
		}

	private:
		FEdgeData() = default;
	};

	struct FSearchContext
	{
		FVector2f P = FVector2f(ForceInitToZero);
		float ClosestDistanceSq = FLT_MAX;
		int16 MinX = 0;
		int16 MinY = 0;
		int16 MaxX = 0;
		int16 MaxY = 0;

		FSearchContext(FVector2f searchLocation, float maxDistance)
			: P(searchLocation)
		{
			ClosestDistanceSq = maxDistance * maxDistance;
			UpdateSearchBounds(maxDistance);
		}

		void UpdateSearchBounds(float maxDistance)
		{
			check(maxDistance >= 0);
			int16 searchExtent = FMath::CeilToInt(maxDistance);
			MinX = FMath::FloorToInt(P.X) - searchExtent;
			MinY = FMath::FloorToInt(P.Y) - searchExtent;
			MaxX = FMath::CeilToInt(P.X) + searchExtent;
			MaxY = FMath::CeilToInt(P.Y) + searchExtent;
		}

	private:
		FSearchContext() = default;
	};

	// TODO - convert to class and use a TSharedPtr?
	struct RTMSDF_API FQuadTree
	{
		TArray<FCell> Cells;
		TArray<FEdgeData> Edges;

		FQuadTree(int16 sizeX, int16 sizeY, size_t reserveEdges = 0, size_t reserveCells = 0);

		void AddItem(const FVector2f& edgePointA, const FVector2f& edgePointB);
		bool FindDistance(FSearchContext& context) const;
		FString ToString() const;

		// TODO - remove once sure of branch prediction inefficacy
		mutable std::atomic_uint32_t BranchHits = 0;
		mutable std::atomic_uint32_t BranchMisses = 0;

	private:
		FQuadTree() = default;

		void AddItem(const uint16 cellID, const FEdgeData& edgeData, uint16 edgeIDX);
		bool FindDistance(const FCell& cell, FSearchContext& context) const;
		FString ToString(const FCell& cell, const FString& indentString) const;
	};
}
