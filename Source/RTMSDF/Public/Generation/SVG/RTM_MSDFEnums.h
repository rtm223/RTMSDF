// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "RTM_MSDFEnums.generated.h"

UENUM(DisplayName = "MSDF Coloring Mode [RTMSDF]")
enum class ERTMSDF_MSDFColoringMode : uint8
{
	// None just produces a PSDF in BGRA, so effectively useless
	None UMETA(Hidden),

	// Simplest method, uses angle threshold (3 rads) to test for corners.
	Simple,

	// Specialized for fonts with ink traps as features
	InkTrap,

	// Theoretically best, but slowest
	Distance,

	MAX UMETA(Hidden),
};

UENUM(DisplayName = "MSDF Error Correction Mode [RTMSDF]")
enum class ERTMSDF_MSDFErrorCorrectionMode : uint8
{
	// Error Correction Disabled
	None,

	// Only correct artifacts at edges. No distance checks
	EdgeOnlyFast UMETA(DisplayName="Edge Only - Fast"),

	// Only correct artifacts at edges, Distance checks at edges only
	EdgeOnlyBalanced UMETA(DisplayName="Edge Only - Balanced"),

	// Only correct artifacts at edges. Full distance checks
	EdgeOnlyFull UMETA(DisplayName="Edge Only - Full"),

	// Correct all artifacts, but prioritise preserving edges and corners. No distance checks
	EdgePriorityFast UMETA(DisplayName="Edge Priority - Fast"),

	// Correct all artifacts, but prioritise preserving edges and corners. Full distance checks
	EdgePriorityFull UMETA(DisplayName="Edge Priority - Full"),

	// Correct all artifacts. No distance checks
	IndiscriminateFast UMETA(DisplayName="Indiscriminate - Fast"),

	// Correct all artifacts. Full distance checks
	IndiscriminateFull UMETA(DisplayName="Indiscriminate - Full"),

	MAX UMETA(Hidden),
};