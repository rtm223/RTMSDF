// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "InterchangePipelineBase.h"
#include "RTMSDF_InterchangePipeline_Texture.generated.h"


UCLASS(Blueprintable, BlueprintType)
class RTMSDFEDITOR_API URTMSDF_InterchangePipeline_Texture : public UInterchangePipelineBase
{
	GENERATED_BODY()

protected:
	virtual void ExecutePostFactoryPipeline(const UInterchangeBaseNodeContainer* baseNodeContainer, const FString& nodeKey, UObject* createdAsset, bool isReimport) override;
};
