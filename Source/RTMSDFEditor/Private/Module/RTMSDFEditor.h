// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

DECLARE_LOG_CATEGORY_EXTERN(RTMSDFEditor, Log, Log)

class FRTMSDFEditorModule : public IModuleInterface
{
protected:
	virtual void StartupModule() override;
};
