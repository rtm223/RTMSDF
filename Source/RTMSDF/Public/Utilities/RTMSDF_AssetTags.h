// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

namespace RTM::SDF::AssetTags
{
    inline static const FName SDFFormatTag = TEXT("SDF Format");
    inline static const FName UVRangeTag = TEXT("SDF UV Range");
    inline static const FName ScaledToFitTag = TEXT("SDF Scaled to Fit Distance");
    inline static const FName InvertedTag = TEXT("SDF Inverted");
    inline static const FName SourceWidthTag = TEXT("SDF Source Width");
    inline static const FName SourceHeightTag = TEXT("SDF Source Height");

    inline static const TCHAR* TrueValue = TEXT("TRUE");
    inline static const TCHAR* FalseValue = TEXT("FALSE");
    inline bool BoolValue(const FString& tagValue){return tagValue == TrueValue;}
    inline FString BoolString(bool value){return value ? TrueValue : FalseValue;}
}
