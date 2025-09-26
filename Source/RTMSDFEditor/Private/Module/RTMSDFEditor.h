// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Stats/Stats.h"

DECLARE_LOG_CATEGORY_EXTERN(RTMSDFEditor, Log, Log)

class FRTMSDFEditorModule : public IModuleInterface
{
protected:
	virtual void StartupModule() override;
};
