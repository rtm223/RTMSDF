// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "ChlumskyMSDFGen.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "ChlumskyMSDFGen"
DEFINE_LOG_CATEGORY(ChlumskyMSDFGen);

IMPLEMENT_MODULE(FChlumskyMSDFGenModule, ChlumskyMSDFGen)

// These checks are to ensure we can detect the existence of MSDF and skia source in the UE5 engine
// In editor this is not causing any issues, but may well become issues if we try to move the MSDF support to runtime
//
// Currently versions of both msdf and skia exist in source builds, but not in launcher builds.
// In source builds we are fine in editor as the separate dlls keep all our symbols from clashing, but I'm expecting
// That in a packaged build, where everything gets rolled into a monolithic binary, then we may see issues (???)
//
// If you find yourself with static_asserts here it is likely because you tried to move this module from editor to runtime
// in a source build and this is your reminder to work out what the deal is.
//
// Runtime MSDF generation from SVGs is a low priority at time of writing
//
// Rich

#if WITH_MSDF_SOURCE && !WITH_EDITOR
static_assert(false, "TODO - Need to implement handling for MSDF source in the engine");
#endif

#if WITH_SKIA_SOURCE && !WITH_EDITOR
static_assert(false, "TODO - Need to implement handling for skia source in the engine");
#endif

#undef LOCTEXT_NAMESPACE
