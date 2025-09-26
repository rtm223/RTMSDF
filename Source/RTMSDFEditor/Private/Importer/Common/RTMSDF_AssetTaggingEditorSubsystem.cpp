// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDF_AssetTaggingEditorSubsystem.h"
#include "Editor.h"
#include "Engine/Texture2D.h"
#include "Generation/Common/RTMSDF_GenerationAssetData_Base.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "UObject/AssetRegistryTagsContext.h"
#include "Utilities/RTMSDF_AssetTags.h"

namespace RTM::SDF::AssetTaggingEditorSubsystemStatics
{
	template<typename TEnum>
	FString GetEnumNameString(TEnum enumValue)
	{
		// TODO - this is getting a fair bit of use, including in asserts and stuff, might be worth making it a util somewhere (poss in runtime, so can be used by BP lib as well)
		const int enumIntValue = static_cast<int>(enumValue);
		const auto* uenumPtr = StaticEnum<TEnum>();
		return uenumPtr->GetNameStringByValue(enumIntValue);
	}

	void GetTags(const URTMSDF_GenerationAssetData_Base* assetData, TArray<UObject::FAssetRegistryTag, TInlineAllocator<16>>& outTags)
	{
		using namespace RTM::SDF::AssetTags;
		const auto& settings = assetData->GetGenerationSettings();
		outTags.Add({SDFFormatTag, GetEnumNameString(settings.GetFormat()), UObject::FAssetRegistryTag::TT_Numerical});
		outTags.Add({UVRangeTag, FString::Printf(TEXT("%f"), assetData->UVRange), UObject::FAssetRegistryTag::TT_Numerical});
		outTags.Add({InvertedTag, BoolString(settings.bInvertDistance), UObject::FAssetRegistryTag::TT_Numerical});
		outTags.Add({ScaledToFitTag, BoolString(settings.bScaleToFitDistance), UObject::FAssetRegistryTag::TT_Numerical});
		outTags.Add({SourceWidthTag, FString::FromInt(assetData->SourceDimensions.X), UObject::FAssetRegistryTag::TT_Numerical});
		outTags.Add({SourceHeightTag, FString::FromInt(assetData->SourceDimensions.Y), UObject::FAssetRegistryTag::TT_Numerical});
	}
}

void URTMSDF_AssetTaggingEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	using namespace RTM::SDF::AssetTaggingEditorSubsystemStatics;
	using namespace RTM::SDF::AssetTags;
	Super::Initialize(Collection);

	if(auto* iss = GEditor->GetEditorSubsystem<UImportSubsystem>())
	{
		iss->OnAssetPostImport.AddWeakLambda(this, [this](UFactory* factory, UObject* createdObject)
		{
			auto* texture = Cast<UTexture2D>(createdObject);
			if(!texture)
				return;

			auto* assetSS = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
			if(!assetSS)
				return;

			if(const auto* sdfData = texture->GetAssetUserData<URTMSDF_GenerationAssetData_Base>())
			{
				TArray<FAssetRegistryTag, TInlineAllocator<16>> tags;
				GetTags(sdfData, tags);
				for(auto& tag : tags)
					assetSS->SetMetadataTag(createdObject, tag.Name, tag.Value);
			}
			else
			{
				assetSS->RemoveMetadataTag(createdObject, SDFFormatTag);
				assetSS->RemoveMetadataTag(createdObject, UVRangeTag);
				assetSS->RemoveMetadataTag(createdObject, SourceWidthTag);
				assetSS->RemoveMetadataTag(createdObject, SourceHeightTag);
			}
		});
	}

	UObject::FAssetRegistryTag::OnGetExtraObjectTagsWithContext.AddWeakLambda(this, [this](FAssetRegistryTagsContext Context)
	{
		auto* texture = Cast<UTexture2D>(Context.GetObject());
		if(!texture)
			return;

		for(const auto* userData : *texture->GetAssetUserDataArray())
		{
			if(auto* sdfData = Cast<URTMSDF_GenerationAssetData_Base>(userData))
			{
				TArray<FAssetRegistryTag, TInlineAllocator<16>> tags;
				GetTags(sdfData, tags);
				for(auto& tag : tags)
					Context.AddTag(tag);

				break;	// We only support one set of settings TODO - consider warning on this
			}
		}
	});
}

void URTMSDF_AssetTaggingEditorSubsystem::Deinitialize()
{
	Super::Deinitialize();
	if(auto* iss = GEditor->GetEditorSubsystem<UImportSubsystem>())
		iss->OnAssetPostImport.RemoveAll(this);
}
