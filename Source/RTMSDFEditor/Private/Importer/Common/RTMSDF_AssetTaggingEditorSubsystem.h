// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "RTMSDF_AssetTaggingEditorSubsystem.generated.h"

UCLASS()
class RTMSDFEDITOR_API URTMSDF_AssetTaggingEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
