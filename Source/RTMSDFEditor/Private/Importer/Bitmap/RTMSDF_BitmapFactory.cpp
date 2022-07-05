// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDF_BitmapFactory.h"
#include "RTMSDF_BitmapImportAssetData.h"
#include "RTMSDF_BitmapImportSettings.h"
#include "Async/ParallelFor.h"
#include "Config/RTMSDFConfig.h"
#include "Curves/CurveLinearColorAtlas.h"
#include "EditorFramework/AssetImportData.h"
#include "Importer/RTMSDFTextureSettingsCache.h"
#include "Module/RTMSDFEditor.h"

#define PREPROCESS_EDGES true

class UTextureRenderTarget;

URTMSDF_BitmapFactory::URTMSDF_BitmapFactory()
{
	// Import priority is super high - we want to jump in and test for SDF filenames or user assets
	// before the engine TextureImporter / Reimporter can get hold of the asset
	ImportPriority = INT32_MAX;
}

bool URTMSDF_BitmapFactory::FactoryCanImport(const FString& filename)
{
	if(const auto* settings = GetDefault<URTMSDFConfig>())
	{
		const auto& suffix = settings->BitmapFilenameSuffix;
		if(suffix.Len() == 0 || FPaths::GetBaseFilename(filename).EndsWith(suffix))
			return true;
	}
	return false;
}

UObject* URTMSDF_BitmapFactory::FactoryCreateBinary(UClass* inClass, UObject* inParent, FName inName, EObjectFlags flags, UObject* context, const TCHAR* type, const uint8*& buffer, const uint8* bufferEnd, FFeedbackContext* warn)
{
	// TODO - unsure exactly how this really should be handled as it's going to get called internally
	// Possibly we can call it after the base class create happens below?
	//GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, inClass, inParent, inName, type);

	bool isInteresting = false;
	auto* existingTexture = FindObject<UTexture2D>(inParent, *inName.ToString());

	FRTMSDFTextureSettingsCache textureSettings(existingTexture);
	FRTMSDF_BitmapImportSettings importerSettings;
	if(const auto* previousSettings = existingTexture ? existingTexture->GetAssetUserData<URTMSDF_BitmapImportAssetData>() : nullptr)
	{
		importerSettings = previousSettings->ImportSettings;
		isInteresting = true;
	}
	else if(const auto* defaultConfig = GetDefault<URTMSDFConfig>())
	{
		const auto& suffix = defaultConfig->BitmapFilenameSuffix;
		if(suffix.Len() > 0 && inName.ToString().EndsWith(suffix))
		{
			isInteresting = true;
			importerSettings = defaultConfig->DefaultBitmapImportSettings;
			textureSettings.LODGroup = defaultConfig->BitmapTextureGroup;
		}
	}
	// TODO - this is overwriting the hardcoded "never SRGB" in the textureSettingsCache. Maybe wants a rethink?
	if(importerSettings.RGBAMode == ERTMSDF_RGBAMode::PreserveRGB)
		textureSettings.SRGB = existingTexture ? existingTexture->SRGB : true;

	// let the texture factory do its thing
	UObject* obj = Super::FactoryCreateBinary(inClass, inParent, inName, flags, context, type, buffer, bufferEnd, warn);

	const uint64 cyclesStart = FPlatformTime::Cycles();

	auto texture = Cast<UTexture2D>(obj);
	if(!isInteresting || !texture)
		return obj;

	texture->bHasBeenPaintedInEditor = false;

	// here we will the place where we do our stuff - grab the source data, rebuild it and overwrite, and attach a data to it.

	uint8* mip = texture->Source.LockMip(0, 0, 0);
	const ETextureSourceFormat fmt = texture->Source.GetFormat(0);
	const int elementWidth = texture->Source.GetBytesPerPixel(0);
	const int sourceWidth = texture->Source.GetSizeX();
	const int sourceHeight = texture->Source.GetSizeY();

	TArray<ERTMSDF_Channels, TFixedAllocator<4>> channelColors;
	if(!GetTextureFormat(fmt, channelColors))
		return texture;	// return the raw asset

	const int numSourceChannels = channelColors.Num();
	const int numDesiredChannels = textureSettings.CompressionSettings == TC_Grayscale || textureSettings.CompressionSettings == TC_Alpha ? 1 : numSourceChannels;
	const bool wantPreserveRGB = numDesiredChannels > 1 && importerSettings.RGBAMode == ERTMSDF_RGBAMode::PreserveRGB;
	const float scale = wantPreserveRGB ? 1.0f : importerSettings.TextureSize / static_cast<float>(FMath::Min(sourceWidth, sourceHeight));

	importerSettings.NumChannels = numDesiredChannels;

	float range = importerSettings.AbsoluteDistance;
	if(importerSettings.DistanceMode == ERTMSDFDistanceMode::Normalized)
		range = importerSettings.NormalizedDistance * FMath::Min(sourceWidth, sourceHeight);
	else if(importerSettings.DistanceMode == ERTMSDFDistanceMode::Pixels)
		range = importerSettings.PixelDistance / scale;

	float* sourceIntersections = static_cast<float*>(FMemory::Malloc((sourceWidth - 1) * (sourceHeight - 1) * 2 * sizeof(float)));
	uint32 numIntersections = 0;

	if(wantPreserveRGB)
	{
		for(int i = 0, num = channelColors.Num(); i < num; i++)
		{
			if(channelColors[i] == ERTMSDF_Channels::Alpha)
			{
				if(FindIntersections(sourceWidth, sourceHeight, mip, elementWidth, i, sourceIntersections, numIntersections))
				{
					CreateDistanceField(sourceWidth, sourceHeight, mip, elementWidth, i, range, importerSettings.InvertDistance, sourceIntersections, mip);
				}
				else
				{
					UE_LOG(RTMSDFEditor, Warning, TEXT("[%s] No alpha information found for Distance Field generation"), *inName.ToString());
				}
			}
		}

		texture->Source.UnlockMip(0, 0, 0);
	}
	else
	{
		const int sdfWidth = sourceWidth * scale;
		const int sdfHeight = sourceHeight * scale;
		uint8* sdfPixels = static_cast<uint8*>(FMemory::Malloc(sdfHeight * sdfWidth * elementWidth));

		for(int i = 0; i < numSourceChannels; i++)
		{
			const bool useChannel = importerSettings.UsesAnyChannel(channelColors[i]);

			// OK to reuse sourceIntersections here as FindIntersections will explicitly fill the entire buffer
			if(useChannel && FindIntersections(sourceWidth, sourceHeight, mip, elementWidth, i, sourceIntersections, numIntersections))
				CreateDistanceField(sourceWidth, sourceHeight, sdfWidth, sdfHeight, mip, elementWidth, i, range, importerSettings.InvertDistance, sourceIntersections, sdfPixels);
			else
				ForceChannelValue(sdfWidth, sdfHeight, sdfPixels, elementWidth, i, channelColors[i] == ERTMSDF_Channels::Alpha ? 255 : 0);
		}

		texture->Source.UnlockMip(0, 0, 0);
		texture->Source.Init(sdfWidth, sdfHeight, 1, 1, fmt, sdfPixels);
		FMemory::Free(sdfPixels);
		sdfPixels = nullptr;
	}
	FMemory::Free(sourceIntersections);
	sourceIntersections = nullptr;

	// TODO - PSD files always come in as RGBA even if they are Grayscale
	if(!existingTexture)
		textureSettings.CompressionSettings = numSourceChannels == 1 ? TC_Grayscale : TC_EditorIcon;

	if(auto* assetData = texture->GetAssetUserData<URTMSDF_BitmapImportAssetData>())
	{
		assetData->ImportSettings.NumChannels = numDesiredChannels;
	}
	else
	{
		auto importData = NewObject<URTMSDF_BitmapImportAssetData>(texture, NAME_None, flags);
		importData->ImportSettings = importerSettings;
		// force num channels to 1 in the settings if user has selected single channel image 
		importData->ImportSettings.NumChannels = numDesiredChannels;
		texture->AddAssetUserData(importData);
	}

	if(!existingTexture)
	{
		UE_LOG(RTMSDFEditor, Log, TEXT("Fresh import of %s - applying default SDF settings"), *texture->GetPathName())
	}

	textureSettings.Restore(texture);
	texture->AssetImportData->Update(CurrentFilename, FileHash.IsValid() ? &FileHash : nullptr);
	texture->PostEditChange();

	const uint64 cyclesEnd = FPlatformTime::Cycles();

	UE_LOG(RTMSDFEditor, Log, TEXT("Import Complete - %.2f miliseconds"), FPlatformTime::ToMilliseconds(cyclesEnd-cyclesStart));
	// TODO - as above, need to work out what to do with this
	//GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, texture);
	return texture;
}

bool URTMSDF_BitmapFactory::IsAutomatedImport() const
{
	return Super::IsAutomatedImport() || IsAutomatedReimport();
}

int32 URTMSDF_BitmapFactory::GetPriority() const
{
	return ImportPriority;
}

bool URTMSDF_BitmapFactory::CanReimport(UObject* obj, TArray<FString>& outFilenames)
{
	UTexture* tex = Cast<UTexture2D>(obj);
	if(tex && !tex->IsA<UCurveLinearColorAtlas>() && tex->GetAssetUserData<URTMSDF_BitmapImportAssetData>())
	{
		tex->AssetImportData->ExtractFilenames(outFilenames);
		return true;
	}
	return false;
}

void URTMSDF_BitmapFactory::SetReimportPaths(UObject* obj, const TArray<FString>& newReimportPaths)
{
	UTexture* tex = Cast<UTexture2D>(obj);
	if(tex && ensure(newReimportPaths.Num() == 1))
		tex->AssetImportData->UpdateFilenameOnly(newReimportPaths[0]);
}

EReimportResult::Type URTMSDF_BitmapFactory::Reimport(UObject* obj)
{
	if(auto* texture = Cast<UTexture2D>(obj))
	{
		const FString textureName = texture->GetName();
		const FString resolvedSourceFilePath = texture->AssetImportData->GetFirstFilename();

		if(!resolvedSourceFilePath.Len())
		{
			UE_LOG(RTMSDFEditor, Error, TEXT("Cannot reimport %s: texture resource does not have path stored."), *textureName);
			return EReimportResult::Failed;
		}
		if(IFileManager::Get().FileSize(*resolvedSourceFilePath) == INDEX_NONE)
		{
			UE_LOG(RTMSDFEditor, Warning, TEXT("Cannot reimport %s: source file [%s] cannot be found."), *textureName, *resolvedSourceFilePath);
			return EReimportResult::Failed;
		}

		UE_LOG(RTMSDFEditor, Log, TEXT("Performing atomic reimport of %s [%s]"), *textureName, *resolvedSourceFilePath);

		bool outCancelled = false;
		UTextureFactory::SuppressImportOverwriteDialog();
		if(ImportObject(texture->GetClass(), texture->GetOuter(), *textureName, RF_Public | RF_Standalone, resolvedSourceFilePath, nullptr, outCancelled))
		{
			if(auto outer = texture->GetOuter())
				outer->MarkPackageDirty();
			else
				texture->MarkPackageDirty();

			texture->AssetImportData->Update(resolvedSourceFilePath);
			return EReimportResult::Succeeded;
		}
		else if(outCancelled)
		{
			UE_LOG(RTMSDFEditor, Warning, TEXT("import of %s canceled"), *textureName);
			return EReimportResult::Cancelled;
		}

		UE_LOG(RTMSDFEditor, Warning, TEXT("import of %s failed"), *textureName);
		return EReimportResult::Failed;
	}

	return EReimportResult::Type::Failed;
}

bool URTMSDF_BitmapFactory::FindIntersections(int width, int height, uint8* const pixels, int pixelWidth, int channelOffset, float* outIntersectionBuffer, uint32& outNumIntersections)
{
	const int intersectionMapWidth = width - 1;
	const int intersectionMapHeight = height - 1;

	std::atomic_uint32_t numFound = false;
	ParallelFor(intersectionMapHeight, [&](const int eY)
	{
		for(int eX = 0; eX < intersectionMapWidth; eX++)
		{
			const int mipIdx = eY * width + eX;
			const uint8 currPix = pixels[mipIdx * pixelWidth + channelOffset];
			const uint8 xPix = pixels[(mipIdx + 1) * pixelWidth + channelOffset];
			const uint8 yPix = pixels[(mipIdx + width) * pixelWidth + channelOffset];

			const float numerator = (127 - currPix);
			const float denominatorX = static_cast<float>(xPix - currPix);
			const float denominatorY = static_cast<float>(yPix - currPix);

			const int intersectionTopIdx = (eY * intersectionMapWidth + eX) * 2;
			const int intersectionLeftIdx = intersectionTopIdx + 1;

			const float intersectionTop = denominatorX != 0.0f ? numerator / denominatorX : -FLT_MAX;
			const float intersectionLeft = denominatorY != 0.0f ? numerator / denominatorY : -FLT_MAX;

			outIntersectionBuffer[intersectionTopIdx] = intersectionTop > 1.0f ? -FLT_MAX : intersectionTop;
			outIntersectionBuffer[intersectionLeftIdx] = intersectionLeft > 1.0f ? -FLT_MAX : intersectionLeft;

			if(outIntersectionBuffer[intersectionTopIdx] >= 0.0f)
				++numFound;

			if(outIntersectionBuffer[intersectionLeftIdx] >= 0.0f)
				++numFound;
		}
	});

	outNumIntersections = numFound;
	UE_LOG(RTMSDFEditor, Log, TEXT("Num Intersections = %d"), outNumIntersections);
	return numFound > 1;
}

void URTMSDF_BitmapFactory::CreateDistanceField(int width, int height, uint8* const pixels, int pixelWidth, int channelOffset, float fieldDistance, bool invertDistance, float* const intersections, uint8* outPixelBuffer)
{
	CreateDistanceField(width, height, width, height, pixels, pixelWidth, channelOffset, fieldDistance, invertDistance, intersections, outPixelBuffer);
}

void URTMSDF_BitmapFactory::CreateDistanceField(int sourceWidth, int sourceHeight, int sdfWidth, int sdfHeight, uint8* const pixels, int pixelWidth, int channelOffset, float fieldDistance, bool invertDistance, const float* intersectionMap, uint8* outPixelBuffer)
{
	const int intersectionMapWidth = sourceWidth - 1;
	const int intersectionMapHeight = sourceHeight - 1;
	const float halfFieldDistance = fieldDistance * 0.5f;

	static const auto edgeTest2 = [](const FVector2D& edgeStart, const FVector2D& edgeEnd, const FVector2D& iPos, float& currDistSq)
	{
		const FVector2D closest = FMath::ClosestPointOnSegment2D(iPos, edgeStart, edgeEnd);
		const float distSq = FVector2D::DistSquared(iPos, closest);

		if(distSq < currDistSq)
			currDistSq = distSq;
	};

#if PREPROCESS_EDGES
	TArray<FVector2D> edgeBuffer;
	for(int y = 0; y < intersectionMapHeight; y++)
	{
		for(int x = 0; x < intersectionMapWidth; x++)
		{
			const int currIdx = (y * intersectionMapWidth + x) * 2;
			const int nextColIdxUnsafe = (y * intersectionMapWidth + (x + 1)) * 2;		// are these really unsafe? 
			const int nextRowIdxUnsafe = ((y + 1) * intersectionMapWidth + x) * 2;		// TODO - should be possible to make this whole thing safe

			const float topIntersection = intersectionMap[currIdx];
			const float leftIntersection = intersectionMap[currIdx + 1];
			const float rightIntersection = (x < intersectionMapWidth - 1) ? intersectionMap[nextColIdxUnsafe + 1] : -1.0f;
			const float bottomIntersection = (y < intersectionMapHeight - 1) ? intersectionMap[nextRowIdxUnsafe] : -1.0f;

			TArray<FVector2D, TInlineAllocator<4>> intersections;

			if(topIntersection >= 0.0f)
				intersections.Add(FVector2D(x + topIntersection, y));

			if(bottomIntersection >= 0.0f)
				intersections.Add(FVector2D(x + bottomIntersection, y + 1));

			if(leftIntersection > 0.0f && leftIntersection < 1.0f)
				intersections.Add(FVector2D(x, y + leftIntersection));

			if(rightIntersection > 0.0f && rightIntersection < 1.0f)
				intersections.Add(FVector2D(x + 1, y + rightIntersection));

			const int numPoints = intersections.Num();
			if(numPoints >= 2)
			{
				edgeBuffer.Add(intersections[0]);
				edgeBuffer.Add(intersections[1]);
			}
			if(numPoints == 4)
			{
				edgeBuffer.Add(intersections[2]);
				edgeBuffer.Add(intersections[3]);
			}
		}
	}
#endif

	ParallelFor(sdfWidth * sdfHeight, [&](const int i)
	{
		const FVector2D sdfPos(i % sdfWidth, i / sdfWidth);
		const FVector2D sourcePos = TransformPos(sdfWidth, sdfHeight, sourceWidth, sourceHeight, sdfPos);

		float currDistSq = halfFieldDistance * halfFieldDistance;

#if PREPROCESS_EDGES
		for(int e = 0, numE = edgeBuffer.Num(); e < numE; e += 2)
		{
			// TODO - consider if there is an efficient way of utilising the shrinking currDistSq to do more aggressive skipping
			// TODO - this should probably be || to include any edge where either of the two lines in within range. Very small false negatives at the very end of the distance field though. Seems costly
			if((sourcePos - edgeBuffer[e]).GetAbsMax() <= halfFieldDistance && (sourcePos - edgeBuffer[e + 1]).GetAbsMax() <= halfFieldDistance)
				edgeTest2(edgeBuffer[e], edgeBuffer[e + 1], sourcePos, currDistSq);
		}
#else
		float maxDist = halfFieldDistance;
		int edgeMinY = FMath::Max(0.0f, sourcePos.Y - maxDist);
		int edgeMaxY = FMath::Min(static_cast<float>(intersectionMapHeight), sourcePos.Y + maxDist);

		for(int y = edgeMinY; y < edgeMaxY; y++)
		{
			const int edgeMinX = FMath::Max(0.0f, sourcePos.X - maxDist);
			const int edgeMaxX = FMath::Min(static_cast<float>(intersectionMapWidth), sourcePos.X + maxDist);

			for(int x = edgeMinX; x < edgeMaxX; x++)
			{
				const int currIdx = (y * intersectionMapWidth + x) * 2;
				const int nextColIdxUnsafe = (y * intersectionMapWidth + (x + 1)) * 2;		// are these really unsafe? 
				const int nextRowIdxUnsafe = ((y + 1) * intersectionMapWidth + x) * 2;		// TODO - should be possible to make this whole thing safe

				const float topIntersection = intersectionMap[currIdx];
				const float leftIntersection = intersectionMap[currIdx + 1];
				const float rightIntersection = (x < intersectionMapWidth - 1) ? intersectionMap[nextColIdxUnsafe + 1] : -1.0f;
				const float bottomIntersection = (y < intersectionMapHeight - 1) ? intersectionMap[nextRowIdxUnsafe] : -1.0f;

				TArray<FVector2D, TInlineAllocator<4>> intersections;

				if(topIntersection >= 0.0f)
					intersections.Add(FVector2D(x + topIntersection, y));

				if(bottomIntersection >= 0.0f)
					intersections.Add(FVector2D(x + bottomIntersection, y + 1));

				if(leftIntersection > 0.0f && leftIntersection < 1.0f)
					intersections.Add(FVector2D(x, y + leftIntersection));

				if(rightIntersection > 0.0f && rightIntersection < 1.0f)
					intersections.Add(FVector2D(x + 1, y + rightIntersection));

				const int numPoints = intersections.Num();
				if(numPoints >= 2)
					edgeTest2(intersections[0], intersections[1], sourcePos, currDistSq);
				if(numPoints == 4)
					edgeTest2(intersections[2], intersections[3], sourcePos, currDistSq);

				// Error detection. We should always have exactly 2 or 4 points OR
				// exactly 1 point that is on a corner (which is ignored ignore)
				if(numPoints == 1)
				{
					const bool isNotCorner = (leftIntersection > 0.0f && leftIntersection < 1.0f)
						|| (rightIntersection > 0.0f && rightIntersection < 1.0f)
						|| (topIntersection > 0.0f && topIntersection < 1.0f)
						|| (bottomIntersection > 0.0f && bottomIntersection < 1.0f);

					ensureAlwaysMsgf(!isNotCorner, TEXT("At position [%d,%d] have %d intersections, expected 0, 2, or 4"), x, y, intersections.Num());
				}
				ensureAlwaysMsgf(numPoints != 3, TEXT("At position [%d,%d] have %d intersections, expected 0, 2, or 4"), x, y, intersections.Num());
			}

			// shrink the search radius after every horizontal scan and skip lines if possible
			maxDist = FMath::Min(FMath::Sqrt(currDistSq) + 1, maxDist);
			y = FMath::Max(y, static_cast<int>(sourcePos.Y - maxDist));
			edgeMaxY = FMath::Min(static_cast<float>(intersectionMapHeight), sourcePos.Y + maxDist);
		}
#endif

		const uint8 mipVal8 = ComputePixelValue(sourcePos, sourceWidth, sourceHeight, pixels, pixelWidth, channelOffset);
		const bool outside = mipVal8 < 127;

		const float dist = FMath::Sqrt(currDistSq);
		const float signedDist = (outside ^ invertDistance) ? dist : -dist;
		float distN = signedDist / fieldDistance + 0.5f;
		uint8 sdfMip = FMath::Clamp(FMath::FloorToInt(distN * 255.0f), 0, 255);
		outPixelBuffer[i * pixelWidth + channelOffset] = sdfMip;
	});
}

void URTMSDF_BitmapFactory::ForceChannelValue(int width, int height, uint8* pixels, int pixelWidth, int channelOffset, uint8 value)
{
	if(width < 1024)	// needs particularly wide rows for a ParallelFor to help
	{
		for(int i = 0, num = width * height; i < num; i++)
			pixels[i * pixelWidth + channelOffset] = value;
	}
	else
	{
		ParallelFor(height, [&](const int y)
		{
			for(int x = 0; x < width; x++)
			{
				const int i = y * width + x;
				pixels[i * pixelWidth + channelOffset] = value;
			}
		});
	}
}

FVector2D URTMSDF_BitmapFactory::TransformPos(float fromWidth, float fromHeight, float toWidth, float toHeight, const FVector2D& fromVec)
{
	FVector2D toCenter = (FVector2D(toWidth, toHeight) - 1.0f) / 2.0f;
	FVector2D fromCenter = (FVector2D(fromWidth, fromHeight) - 1.0f) / 2.0f;
	FVector2D fromPos = fromVec - fromCenter;
	FVector2D toPos = fromPos * FVector2D(toWidth / fromWidth, toHeight / fromHeight);
	return toCenter + toPos;
}

uint8 URTMSDF_BitmapFactory::ComputePixelValue(FVector2D pos, int width, int height, uint8* const buffer, int pixelWidth, int channelOffset)
{
	auto index = [width, pixelWidth, channelOffset](int x, int y) { return (y * width + x) * pixelWidth + channelOffset; };
	pos = FVector2D(FMath::Clamp(pos.X, 0.0f, width - 1.0f), FMath::Clamp(pos.Y, 0.0f, width - 1.0f));
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

bool URTMSDF_BitmapFactory::GetTextureFormat(ETextureSourceFormat format, TArray<ERTMSDF_Channels, TFixedAllocator<4>>& channelPositions)
{
	switch(format)
	{
		case TSF_G8:
			channelPositions.Add(ERTMSDF_Channels::Alpha);
			return true;

		case TSF_BGRA8:
		case TSF_BGRE8:		// unsure what the e is for?
			channelPositions.Append({ERTMSDF_Channels::Blue, ERTMSDF_Channels::Green, ERTMSDF_Channels::Red, ERTMSDF_Channels::Alpha});
			return true;

		case TSF_RGBA16:
		case TSF_RGBA16F:
		case TSF_G16:
			UE_LOG(RTMSDFEditor, Error, TEXT("Unsupported Source format - 16 bit formats not currently supported"));
		default:
			UE_LOG(RTMSDFEditor, Error, TEXT("Unable to import file with texture format (ETextureSourceFormat)  = %d"), static_cast<int>(format));
			return false;
	}
}

#undef PREPROCESS_EDGES
