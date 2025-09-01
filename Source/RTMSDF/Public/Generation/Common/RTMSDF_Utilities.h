// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once
#include "CoreMinimal.h"

namespace RTM::SDF::Utils
{
	static FVector2f ClosestPointOnSegment2D(const FVector2f& point, const FVector2f& startPoint, const FVector2f& endPoint)
	{
		// Duplicated from FMath::ClosestPointOnSegment2D so we have a floating point version in UE5

		const FVector2f segment = endPoint - startPoint;
		const FVector2f vecToPoint = point - startPoint;

		// See if closest point is before startPoint
		const float dot1 = vecToPoint | segment;
		if(dot1 <= 0)
			return startPoint;

		// See if closest point is beyond endPoint
		const float dot2 = segment | segment;
		if(dot2 <= dot1)
			return endPoint;

		// Closest point is within segment
		return startPoint + segment * (dot1 / dot2);
	}

	static uint16 RoundUpToPow2(uint16 a)
	{
		a--;
		a |= a >> 1;
		a |= a >> 2;
		a |= a >> 4;
		a |= a >> 8;
		a++;
		return a;
	}
}
