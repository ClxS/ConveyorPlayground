#pragma once

#include "RenderContext.h"

#if defined(_CONSOLE)
#include "TextTileRenderHandler.h"
#elif defined(_SDL)
#endif

void tileRenderer(
    cpp_conv::RenderContext& kContext,
    const cpp_conv::resources::RenderableAsset* pAsset,
    const cpp_conv::Transform2D& kTransform,
    cpp_conv::Colour kColourOverride = { 0xFF000000 },
    bool bTrack = false);
