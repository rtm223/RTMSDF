// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "RTMSDF_SVGFactory.generated.h"

UCLASS()
class URTMSDF_SVGFactory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
	URTMSDF_SVGFactory();

	// UFactory
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn) override;
	virtual bool IsAutomatedImport() const override;

	// FReimportHandler
	virtual int32 GetPriority() const override;
	virtual bool CanReimport(UObject* obj, TArray<FString>& outFilenames) override;
	virtual void SetReimportPaths(UObject* obj, const TArray<FString>& newReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* obj) override;

private:
	static constexpr double DEFAULT_ANGLE_THRESHOLD = 3.0;
};
