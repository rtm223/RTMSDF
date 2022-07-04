// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EditorReimportHandler.h"
#include "Factories/TextureFactory.h"
#include "RTMSDF_BitmapFactory.generated.h"

UCLASS()
class RTMSDFEDITOR_API URTMSDF_BitmapFactory : public UTextureFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
	URTMSDF_BitmapFactory();

	// UTextureFactory
	virtual bool FactoryCanImport(const FString& filename) override;
	virtual UObject* FactoryCreateBinary(UClass* inClass, UObject* inParent, FName inName, EObjectFlags flags, UObject* context, const TCHAR* type, const uint8*& buffer, const uint8* bufferEnd, FFeedbackContext* warn) override;
	virtual bool IsAutomatedImport() const override;

	// FReimportHandler
	virtual int32 GetPriority() const override;
	virtual bool CanReimport(UObject* obj, TArray<FString>& outFilenames) override;
	virtual void SetReimportPaths(UObject* obj, const TArray<FString>& newReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* obj) override;

private:
	static bool FindIntersections(int width, int height, uint8* const pixels, int pixelWidth, int channelOffset, float* outIntersectionBuffer, uint32& outNumIntersections);
	static void CreateDistanceField(int width, int height, uint8* const pixels, int pixelWidth, int channelOffset, float fieldDistance, bool invertDistance, float* const intersections, uint8* outPixelBuffer);
	static void CreateDistanceField(int sourceWidth, int sourceHeight, int sdfWidth, int sdfHeight, uint8* const pixels, int pixelWidth, int channelOffset, float fieldDistance, bool invertDistance, const float* intersectionMap, uint8* outPixelBuffer);
	static void ForceChannelValue(int width, int height, uint8* pixels, int pixelWidth, int channelOffset, uint8 value);
	static FVector2D TransformPos(float fromWidth, float fromHeight, float toWidth, float toHeight, const FVector2D& fromVec);
	static uint8 ComputePixelValue(FVector2D pos, int width, int height, uint8* const buffer, int pixelWidth, int channelOffset);
	static bool GetTextureFormat(ETextureSourceFormat format, int& numChannels, int& outRed, int& outBlue, int& outGreen, int& outAlpha);
};
