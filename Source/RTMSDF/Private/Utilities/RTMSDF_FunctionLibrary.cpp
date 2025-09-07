// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Utilities/RTMSDF_FunctionLibrary.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Texture2D.h"
#include "Generation/Bitmap/RTMSDF_BitmapGenerationAssetData.h"
#include "Generation/Common/RTMSDF_GenerationAssetData_Base.h"
#include "Generation/SVG/RTMSDF_SVGGenerationAssetData.h"
#include "Utilities/RTMSDF_AssetTags.h"

namespace RTM::SDF::FunctionLibraryStatics
{
	template<typename TEnum>
	TEnum GetEnumValueFromName(FName name, TEnum defaultValue = {})
	{
		const auto* uenumPtr = StaticEnum<TEnum>();
		const int64 enumValue = uenumPtr->GetValueByName(name);
		return enumValue == INDEX_NONE ? defaultValue : static_cast<TEnum>(enumValue);
	}

	static bool TryGetAssetTag(TSoftObjectPtr<> softObject, FName tag, FString& outValue)
	{
		IAssetRegistry& registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

		FAssetData assetData;
		if(UE::AssetRegistry::EExists::Exists == registry.TryGetAssetByObjectPath(softObject.ToSoftObjectPath(), assetData))
			return assetData.GetTagValue(tag, outValue);

		return false;
	}

	static bool HasAssetTag(TSoftObjectPtr<> softObject, FName tag)
	{
		FString tagValue;
		return TryGetAssetTag(softObject, tag, tagValue);
	}

	static bool IsSingleChannelSDFTexture(ERTMSDF_SDFFormat format)
	{
		return format == ERTMSDF_SDFFormat::SingleChannel
			|| format == ERTMSDF_SDFFormat::SingleChannelPseudo;
	}

	static bool IsMSDFTexture(ERTMSDF_SDFFormat format)
	{
		return format == ERTMSDF_SDFFormat::Multichannel
			|| format == ERTMSDF_SDFFormat::MultichannelPlusAlpha;
	}
}

bool URTMSDF_FunctionLibrary::IsSDFTexture(const UTexture2D* texture)
{
	const auto* assetData = texture ? const_cast<UTexture2D*>(texture)->GetAssetUserData<URTMSDF_GenerationAssetData_Base>() : nullptr;
	return !!assetData;
}

ERTMSDF_SDFFormat URTMSDF_FunctionLibrary::GetSDFFormat(const UTexture2D* texture)
{
	const auto* assetData = texture ? const_cast<UTexture2D*>(texture)->GetAssetUserData<URTMSDF_GenerationAssetData_Base>() : nullptr;
	return assetData ? assetData->GetGenerationSettings().GetFormat() : ERTMSDF_SDFFormat::Invalid;
}

FIntPoint URTMSDF_FunctionLibrary::GetSourceDimensions(const UTexture2D* texture)
{
	const auto* assetData = texture ? const_cast<UTexture2D*>(texture)->GetAssetUserData<URTMSDF_GenerationAssetData_Base>() : nullptr;
	return assetData ? assetData->SourceDimensions : FIntPoint(-1, -1);
}

bool URTMSDF_FunctionLibrary::IsSingleChannelSDFTexture(const UTexture2D* texture)
{
	return RTM::SDF::FunctionLibraryStatics::IsSingleChannelSDFTexture(GetSDFFormat(texture));
}

bool URTMSDF_FunctionLibrary::IsMSDFTexture(const UTexture2D* texture)
{
	return RTM::SDF::FunctionLibraryStatics::IsSingleChannelSDFTexture(GetSDFFormat(texture));
}

float URTMSDF_FunctionLibrary::GetSDFUVRange(const UTexture2D* texture)
{
	const auto* assetData = texture ? const_cast<UTexture2D*>(texture)->GetAssetUserData<URTMSDF_GenerationAssetData_Base>() : nullptr;
	ensureAlways(!assetData || assetData->UVRange > 0);
	return assetData ? assetData->UVRange : -1.0f;
}

bool URTMSDF_FunctionLibrary::IsSDFSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture)
{
	return GetSDFFormatFromSoftTexture(softTexture) != ERTMSDF_SDFFormat::Invalid;
}

ERTMSDF_SDFFormat URTMSDF_FunctionLibrary::GetSDFFormatFromSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture)
{
	using namespace RTM::SDF;

	FString tagValue;
	if(FunctionLibraryStatics::TryGetAssetTag(softTexture, AssetTags::SDFFormatTag, tagValue))
		return FunctionLibraryStatics::GetEnumValueFromName(FName(tagValue), ERTMSDF_SDFFormat::Invalid);

	return ERTMSDF_SDFFormat::Invalid;
}

FIntPoint URTMSDF_FunctionLibrary::GetSourceDimensionsFromSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture)
{
	using namespace RTM::SDF;

	FString widthValue, heightValue;
	const bool dataValid = FunctionLibraryStatics::TryGetAssetTag(softTexture, AssetTags::SourceWidthTag, widthValue)
		&& FunctionLibraryStatics::TryGetAssetTag(softTexture, AssetTags::SourceHeightTag, heightValue);

	return dataValid ? FIntPoint(FCString::Atoi(*widthValue), FCString::Atoi(*heightValue)) : FIntPoint(-1, -1);
}

bool URTMSDF_FunctionLibrary::IsSingleChannelSDFSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture)
{
	return RTM::SDF::FunctionLibraryStatics::IsSingleChannelSDFTexture(GetSDFFormatFromSoftTexture(softTexture));
}

bool URTMSDF_FunctionLibrary::IsMSDFSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture)
{
	return RTM::SDF::FunctionLibraryStatics::IsMSDFTexture(GetSDFFormatFromSoftTexture(softTexture));
}

float URTMSDF_FunctionLibrary::GetSDFUVRangeFromSoftTexure(const TSoftObjectPtr<UTexture2D>& softTexture)
{
	using namespace RTM::SDF;

	FString tagValue;
	if(FunctionLibraryStatics::TryGetAssetTag(softTexture, AssetTags::UVRangeTag, tagValue))
		return FCString::Atof(*tagValue);

	return -1;
}
