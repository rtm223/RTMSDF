// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Generation/Bitmap/RTMSDF_QuadTree.h"
#include "Generation/Common/RTMSDF_Utilities.h"

namespace RTM::SDF
{
	static constexpr bool TestIntersection(const FCell& cell, const FEdgeData& edge)
	{
		const int16 cMaxX = cell.MinX + cell.Size;
		const int16 cMaxY = cell.MinY + cell.Size;

		return !(edge.MinX >= cMaxX
			|| edge.MinY >= cMaxY
			|| edge.MaxX <= cell.MinX
			|| edge.MaxY <= cell.MinY);
	}

	static constexpr bool TestIntersection(const FCell& cell, const FSearchContext& context)
	{
		const int16 cMaxX = cell.MinX + cell.Size;
		const int16 cMaxY = cell.MinY + cell.Size;

		return !(context.MinX >= cMaxX
			|| context.MinY >= cMaxY
			|| context.MaxX <= cell.MinX
			|| context.MaxY <= cell.MinY);
	}

	static constexpr bool TestIntersection(const FSearchContext& context, const FEdgeData& edge)
	{
		return !(context.MinX >= edge.MaxX
			|| context.MinY >= edge.MaxY
			|| context.MaxX <= edge.MinX
			|| context.MaxY <= edge.MinY);
	}

	static constexpr uint32 GetOverlapArea(const FCell& cell, const FSearchContext& context)
	{
		const int16 cMaxX = cell.MinX + cell.Size;
		const int16 cMaxY = cell.MinY + cell.Size;

		const int32 xMin = FMath::Max(cell.MinX, context.MinX);
		const int32 yMin = FMath::Max(cell.MinY, context.MinY);
		const int32 xMax = FMath::Min(cMaxX, context.MaxX);
		const int32 yMax = FMath::Min(cMaxY, context.MaxY);

		const int32 overlapArea = FMath::Max(xMax - xMin, 0) * FMath::Max(yMax - yMin, 0);

		check(overlapArea > 0 == TestIntersection(cell, context));
		return overlapArea;
	}

	FQuadTree::FQuadTree(int16 sizeX, int16 sizeY, size_t reserveEdges, size_t reserveCells)
	{
		if(reserveEdges > 0)
			Edges.Reserve(reserveEdges);

		if(reserveCells > 0)
			Cells.Reserve(reserveCells);

		const int16 size = Utils::RoundUpToPow2(FMath::Max(sizeX, sizeY));
		const int16 offsetX = (sizeX - size) / 2;
		const int16 offsetY = (sizeY - size) / 2;
		Cells.Add({offsetX, offsetY, size});
	}

	void FQuadTree::AddItem(const FVector2f& edgePointA, const FVector2f& edgePointB)
	{
		check(Cells.Num() > 0);

		const uint16 edgeIDX = Edges.Add({edgePointA, edgePointB});
		const FEdgeData& newEdge = Edges[edgeIDX];

		constexpr int16 rootCellIDX = 0;
		const FCell& rootCell = Cells[rootCellIDX];

		if(TestIntersection(rootCell, newEdge))
			AddItem(rootCellIDX, newEdge, edgeIDX);
	}

	bool FQuadTree::FindDistance(FSearchContext& context) const
	{
		if(Edges.Num() == 0)
			return false;

		check(Cells.Num() > 0);
		const FCell& rootCell = Cells[0];

		return TestIntersection(rootCell, context) &&
			FindDistance(rootCell, context);
	}

	FString FQuadTree::ToString() const
	{
		return ToString(Cells[0], TEXT(""));
	}

	void FQuadTree::AddItem(const uint16 cellID, const FEdgeData& edgeData, uint16 edgeIDX)
	{
		// NOTE: it is required that calls into this function have already verified that the edge intersects this cell

		const auto cell = Cells[cellID];
		if(cell.bChildCells)
		{
			// check which of the 4 subcells overlap the edge and add it.
			// Note it is possible for multiple cells to overlap the edge
			for(int i = 0; i < 4; ++i)
			{
				const uint16 childCellIdx = cell.Children[i];
				check(childCellIdx != FCell::INVALID);
				FCell& childCell = Cells[childCellIdx];

				// NOTE: hit rate around 20-30% here - might be possible to improve with branch prediction
				// However, UE UNLIKELY doesn't operate on windows and cpp [[unlikely]] has no measurable effect (poss not implemented either?)
				if(TestIntersection(childCell, edgeData))
					AddItem(childCellIdx, edgeData, edgeIDX);
			}
		}
		else if(Cells[cellID].NumChildren == 4)
		{
			// Need to reposition all of the 5 children (4 existing and 1 new);
			const uint16 childEdgeIDXs[5] = {Cells[cellID].Children[0], Cells[cellID].Children[1], Cells[cellID].Children[2], Cells[cellID].Children[3], edgeIDX};

			// Subdivide cells
			const int16 childSize = Cells[cellID].Size >> 1;	// TODO - need to ensure this never gets smaller than the minimum size (1). Will also need offsets fixed
			for(int c = 0; c < 4; ++c)
			{
				const int16 childX = Cells[cellID].MinX + ((c & 1) ? childSize : 0);
				const int16 childY = Cells[cellID].MinY + ((c > 1) ? childSize : 0);
				Cells[cellID].Children[c] = Cells.Add({childX, childY, childSize});
			}
			Cells[cellID].bChildCells = true;
			Cells[cellID].NumChildren = 4;

			for(int i = 0; i < 5; ++i)
			{
				const uint16 childEdgeIDX = childEdgeIDXs[i];
				const FEdgeData childEdge = Edges[childEdgeIDX];
				for(int c = 0; c < 4; ++c)
				{
					// note - our edge may intersect multiple cells, if it crosses cell boundaries. This is fine
					uint16 childCellIDX = Cells[cellID].Children[c];
					const FCell& childCell = Cells[childCellIDX];
					if(TestIntersection(childCell, childEdge))
						AddItem(childCellIDX, childEdge, childEdgeIDX);
				}
			}
		}
		else
		{
			auto& mutableCell = Cells[cellID];
			mutableCell.Children[mutableCell.NumChildren] = edgeIDX;
			mutableCell.NumChildren++;
		}
	}

	bool FQuadTree::FindDistance(const FCell& cell, FSearchContext& context) const
	{
		bool anyFound = false;
		if(cell.bChildCells)
		{
			// Quick and dirty check to see which quadrant to search in first, with the hope of finding an edge in there that makes the other quadrants skippable in broadphase
			// Seems to shave around 15% total search time (depending on asset). Maybe worth investigating improved patterns, but expect diminishing returns
			const bool toRight = context.P.X > cell.MinX + cell.Size;
			const bool below = context.P.Y > cell.MinY + cell.Size;
			const int firstCell = (toRight ? 1 : 0) + (below ? 2 : 0);

			for(int i = 0; i < 4; ++i)
			{
				int cellIndex = (firstCell + i) % 4;
				const uint16 childCellId = cell.Children[cellIndex];
				check(childCellId != FCell::INVALID);

				const FCell& childCell = Cells[childCellId];
				// NOTE: around 75% hitrate here (prior to testing num children). Branch prediciton doing nothing for us though
				if(childCell.NumChildren > 0 && TestIntersection(childCell, context))
					anyFound |= FindDistance(childCell, context);
			}
		}
		else	// Children are leaf nodes, so just to a distance check against them
		{
			bool updateBounds = false;
			for(int i = 0; i < cell.NumChildren; ++i)
			{
				const uint16 foundEdgeId = cell.Children[i];
				const FEdgeData& foundEdge = Edges[foundEdgeId];

				// NOTE: broad phase check against edges is not worth it. probably due to the cell based checks doing good enough culling
				// Also would require moving the update search bounds back inside the loop, and that optimisation one is actually shaving time off
				// if(!TestIntersection(context, foundEdge))
				// 	continue;

				anyFound = true;

				const FVector2f& closestPointOnEdge = Utils::ClosestPointOnSegment2D(context.P, foundEdge.P1, foundEdge.P2);
				const float foundDistanceSq = FVector2f::DistSquared(context.P, closestPointOnEdge);

				if(foundDistanceSq < context.ClosestDistanceSq)
				{
					context.ClosestDistanceSq = foundDistanceSq;
					updateBounds = true;
				}
			}
			if(updateBounds)
			 	context.UpdateSearchBounds(FMath::Sqrt(context.ClosestDistanceSq));
		}

		return anyFound;
	}

	FString FQuadTree::ToString(const FCell& cell, const FString& indentString) const
	{
		FString cellString = indentString + FString::Printf(TEXT("CELL [%d,%d][%d,%d]"), cell.MinX, cell.MinY, cell.MinX + cell.Size, cell.MinY + cell.Size);	// TODO - move to cell
		const FString childIndent = indentString + TEXT("\t");
		if(cell.bChildCells)
		{
			for(int i = 0; i < 4; ++i)
			{
				cellString += FString(TEXT("\n")) + ToString(Cells[cell.Children[i]], childIndent);
			}
		}
		else
		{
			for(int i = 0; i < 4; ++i)
			{
				if(cell.Children[i] == FCell::INVALID)
					break;
				FEdgeData edge = Edges[cell.Children[i]];
				cellString += FString::Printf(TEXT("\n%sleaf [%d,%d][%d,%d]"), *childIndent, edge.MinX, edge.MinY, edge.MaxX, edge.MaxY);
			}
		}
		return cellString;
	}
}
