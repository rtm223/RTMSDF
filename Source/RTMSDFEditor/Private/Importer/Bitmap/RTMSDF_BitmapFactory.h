// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EditorReimportHandler.h"
#include "Factories/TextureFactory.h"
#include "RTMSDF_BitmapFactory.generated.h"

enum class ERTMSDF_Channels : uint8;

UCLASS()
class URTMSDF_BitmapFactory : public UTextureFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
#if ENGINE_MAJOR_VERSION >= 5
	using FVector2f = UE::Math::TVector2<float>;
#else
	using FVector2f = FVector2D;
#endif

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

};
