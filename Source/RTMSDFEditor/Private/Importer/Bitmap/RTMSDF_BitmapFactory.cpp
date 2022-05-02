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

	// let the texture factory do its thing
	UObject* obj = Super::FactoryCreateBinary(inClass, inParent, inName, flags, context, type, buffer, bufferEnd, warn);

	const uint64 cycleStart = FPlatformTime::Cycles();

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

	int numChannels, redChannel, greenChannel, blueChannel, alphaChannel;
	if(!GetTextureFormat(fmt, numChannels, redChannel, blueChannel, greenChannel, alphaChannel))
		return texture;	// return the raw asset

	const bool wantPreserveRGB = numChannels > 1 && importerSettings.RGBAMode == ERTMSDF_RGBAMode::PreserveRGB;
	const float scale = wantPreserveRGB ? 1.0f : importerSettings.TextureSize / (float)FMath::Min(sourceWidth, sourceHeight);

	float range = importerSettings.AbsoluteDistance;
	if(importerSettings.DistanceMode == ERTMSDFDistanceMode::Normalized)
		range = importerSettings.NormalizedDistance * FMath::Min(sourceWidth, sourceHeight);
	else if(importerSettings.DistanceMode == ERTMSDFDistanceMode::Pixels)
		range = importerSettings.PixelDistance / scale;

	float* sourceEdges = (float*)FMemory::Malloc((sourceWidth - 1) * (sourceHeight - 1) * 2 * sizeof(float));
	if(wantPreserveRGB)
	{
		if(FindEdges(sourceWidth, sourceHeight, mip, elementWidth, alphaChannel, sourceEdges))
		{
			CreateDistanceField(sourceWidth, sourceHeight, mip, elementWidth, alphaChannel, range, importerSettings.InvertDistance, sourceEdges, mip);
		}
		else
		{
			UE_LOG(RTMSDFEditor, Warning, TEXT("[%s] No alpha information found for Distance Field generation"), *inName.ToString());
		}

		texture->Source.UnlockMip(0, 0, 0);
	}
	else
	{
		const int sdfWidth = sourceWidth * scale;
		const int sdfHeight = sourceHeight * scale;
		uint8* sdfPixels = static_cast<uint8*>(FMemory::Malloc(sdfHeight * sdfWidth * elementWidth));

		for(int i = 0; i < numChannels; i++)
		{
			const bool useChannel = (i == redChannel && importerSettings.UsesAnyChannel(ERTMSDF_Channels::Red))
				|| (i == blueChannel && importerSettings.UsesAnyChannel(ERTMSDF_Channels::Blue))
				|| (i == greenChannel && importerSettings.UsesAnyChannel(ERTMSDF_Channels::Green))
				|| (i == alphaChannel && importerSettings.UsesAnyChannel(ERTMSDF_Channels::Alpha));

			// OK to reuse sourceEdges here as FindEdges will explicitly fill every entry
			if(useChannel && FindEdges(sourceWidth, sourceHeight, mip, elementWidth, i, sourceEdges))
				CreateDistanceField(sourceWidth, sourceHeight, sdfWidth, sdfHeight, mip, elementWidth, i, range, importerSettings.InvertDistance, sourceEdges, sdfPixels);
			else
				ForceChannelValue(sdfWidth, sdfHeight, sdfPixels, elementWidth, i, i == alphaChannel ? 255 : 0);
		}

		texture->Source.UnlockMip(0, 0, 0);
		texture->Source.Init(sdfWidth, sdfHeight, 1, 1, fmt, sdfPixels);
		FMemory::Free(sdfPixels);
		sdfPixels = nullptr;
	}
	FMemory::Free(sourceEdges);
	sourceEdges = nullptr;

	// TODO - PSD files always come in as RGBA even if they are Grayscale
	if(!existingTexture)
		textureSettings.CompressionSettings = numChannels == 1 ? TC_Grayscale : TC_EditorIcon;

	if(auto* assetData = texture->GetAssetUserData<URTMSDF_BitmapImportAssetData>())
	{
		assetData->ImportSettings.NumChannels = numChannels;
	}
	else
	{
		auto importData = NewObject<URTMSDF_BitmapImportAssetData>(texture, NAME_None, flags);
		importData->ImportSettings = importerSettings;
		// force num channels to 1 in the settings if user has selected single channel image 
		importData->ImportSettings.NumChannels = textureSettings.CompressionSettings == TC_Grayscale || textureSettings.CompressionSettings == TC_Alpha ? 1 : numChannels;
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

	UE_LOG(RTMSDFEditor, Log, TEXT("Import Complete - tool %.2f miliseconds"), FPlatformTime::ToMilliseconds(cyclesEnd-cycleStart));
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

bool URTMSDF_BitmapFactory::FindEdges(int width, int height, uint8* const pixels, int pixelWidth, int channelOffset, float* outEdgeBuffer)
{
	const int edgeMapWidth = width - 1;
	const int edgeMapHeight = height - 1;

	std::atomic_int64_t numEdges = false;
	ParallelFor(edgeMapHeight, [&](const int eY)
	{
		for(int eX = 0; eX < edgeMapWidth; eX++)
		{
			const int mipIdx = eY * width + eX;
			const uint8 currPix = pixels[mipIdx * pixelWidth + channelOffset];
			const uint8 xPix = pixels[(mipIdx + 1) * pixelWidth + channelOffset];
			const uint8 yPix = pixels[(mipIdx + width) * pixelWidth + channelOffset];

			const float numerator = (127 - currPix);
			const float denominatorX = static_cast<float>(xPix - currPix);
			const float denominatorY = static_cast<float>(yPix - currPix);

			const int edgeXIdx = (eY * edgeMapWidth + eX) * 2;
			const int edgeYIdx = edgeXIdx + 1;

			outEdgeBuffer[edgeXIdx] = denominatorX != 0.0f ? numerator / denominatorX : -FLT_MAX;
			outEdgeBuffer[edgeYIdx] = denominatorY != 0.0f ? numerator / denominatorY : -FLT_MAX;

			outEdgeBuffer[edgeXIdx] = outEdgeBuffer[edgeXIdx] > 1.0f ? -FLT_MAX : outEdgeBuffer[edgeXIdx];
			outEdgeBuffer[edgeYIdx] = outEdgeBuffer[edgeYIdx] > 1.0f ? -FLT_MAX : outEdgeBuffer[edgeYIdx];
			if(outEdgeBuffer[edgeXIdx] >= 0.0f || outEdgeBuffer[edgeYIdx] >= 0.0f)
				++numEdges;
		}
	});
	return numEdges > 1;
}

void URTMSDF_BitmapFactory::CreateDistanceField(int width, int height, uint8* const pixels, int pixelWidth, int channelOffset, float fieldDistance, bool invertDistance, float* const edges, uint8* outPixelBuffer)
{
	CreateDistanceField(width, height, width, height, pixels, pixelWidth, channelOffset, fieldDistance, invertDistance, edges, outPixelBuffer);
}

void URTMSDF_BitmapFactory::CreateDistanceField(int sourceWidth, int sourceHeight, int sdfWidth, int sdfHeight, uint8* const pixels, int pixelWidth, int channelOffset, float fieldDistance, bool invertDistance, float* const edges, uint8* outPixelBuffer)
{
	const int edgeMapWidth = sourceWidth - 1;
	const int edgeMapHeight = sourceHeight - 1;
	const float halfFieldDistance = fieldDistance * 0.5f;

	// static const auto edgeTest = [](const FVector2D& edgePos, const FVector2D& iPos, float& currDistSq)
	// {
	// 	FVector2D disp = edgePos - iPos;
	// 	const float distSq = disp.SizeSquared();
	// 	if(distSq < currDistSq)
	// 		currDistSq = distSq;
	// };

	static const auto edgeTest2 = [](const FVector2D& edgeStart, const FVector2D& edgeEnd, const FVector2D& iPos, float& currDistSq)
	{
		const FVector2D closest = FMath::ClosestPointOnSegment2D(iPos, edgeStart, edgeEnd);
		const float distSq = FVector2D::DistSquared(iPos, closest);

		if(distSq < currDistSq)
			currDistSq = distSq;
	};

	ParallelFor(sdfWidth * sdfHeight, [&](const int i)
	{
		const FVector2D sdfPos(i % sdfWidth, i / sdfWidth);
		const FVector2D sourcePos = TransformPos(sdfWidth, sdfHeight, sourceWidth, sourceHeight, sdfPos);

		const uint8 mipVal8 = ComputePixelValue(sourcePos, sourceWidth, sourceHeight, pixels, pixelWidth, channelOffset);
		const bool outside = mipVal8 <= 127;

		float currDistSq = fieldDistance * fieldDistance;
		float maxDist = halfFieldDistance;

		int edgeMinY = FMath::Max(0.0f, sourcePos.Y - maxDist);
		int edgeMaxY = FMath::Min(static_cast<float>(edgeMapHeight), sourcePos.Y + maxDist);

		for(int y = edgeMinY; y < edgeMaxY; y++)
		{
			const int edgeMinX = FMath::Max(0.0f, sourcePos.X - maxDist);
			const int edgeMaxX = FMath::Min(static_cast<float>(edgeMapWidth), sourcePos.X + maxDist);

			for(int x = edgeMinX; x < edgeMaxX; x++)
			{
				const int currIdx = (y * edgeMapWidth + x) * 2;
				const int nextColIdxUnsafe = (y * edgeMapWidth + (x + 1)) * 2;		// are these really unsafe? 
				const int nextRowIdxUnsafe = ((y + 1) * edgeMapWidth + x) * 2;		// TODO - should be possible to make this whole thing safe

				const float topIntersection = edges[currIdx];
				const float leftIntersection = edges[currIdx + 1];
				const float rightIntersection = (x < edgeMapWidth - 1) ? edges[nextColIdxUnsafe + 1] : -1.0f;
				const float bottomIntersection = (y < edgeMapHeight - 1) ? edges[nextRowIdxUnsafe] : -1.0f;

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
				// exactly 1 point that is on a corner (which we ignore)
				if(numPoints == 1)
				{
					const bool isNotCorner = (leftIntersection > 0.0f && leftIntersection < 1.0f)
						|| (rightIntersection > 0.0f && rightIntersection < 1.0f)
						|| (topIntersection > 0.0f && topIntersection < 1.0f)
						|| (bottomIntersection > 0.0f && bottomIntersection < 1.0f);

					ensureAlwaysMsgf(!isNotCorner, TEXT("At position [%d,%d] have %d edges, expected 0,2,4"), x, y, intersections.Num());
				}
				ensureAlwaysMsgf(numPoints != 3, TEXT("At position [%d,%d] have %d edges, expected 0,2,4"), x, y, intersections.Num());
			}

			maxDist = FMath::Min(FMath::Sqrt(currDistSq), maxDist);
			y = FMath::Max(y, static_cast<int>(sourcePos.Y - maxDist));
			edgeMaxY = FMath::Min(static_cast<float>(edgeMapHeight), sourcePos.Y + maxDist);
		}

		float dist = FMath::Sqrt(currDistSq);
		dist = (outside ^ invertDistance) ? dist : -dist;
		float distN = dist / fieldDistance + 0.5f;
		uint8 sdfMip = FMath::Clamp(FMath::FloorToInt(distN * 255.0f), 0, 255);
		outPixelBuffer[i * pixelWidth + channelOffset] = sdfMip;
	});
}

void URTMSDF_BitmapFactory::ForceChannelValue(int width, int height, uint8* pixels, int pixelWidth, int channelOffset, uint8 value)
{
	ParallelFor(width * height, [&](const int i)
	{
		pixels[i * pixelWidth + channelOffset] = value;
	});
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

bool URTMSDF_BitmapFactory::GetTextureFormat(ETextureSourceFormat format, int& numChannels, int& outRed, int& outBlue, int& outGreen, int& outAlpha)
{
	switch(format)
	{
		case TSF_G8:
			numChannels = 1;
			outAlpha = 1;
			outRed = outBlue = outGreen = -1;
			return true;

		case TSF_BGRA8:
		case TSF_BGRE8:		// unsure what the e is for?
			numChannels = 4;
			outBlue = 0;
			outGreen = 1;
			outRed = 2;
			outAlpha = 3;
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
