#pragma once

#if !defined(_SDL)
#error Invalid Include
#endif

#include "RenderableAsset.h"

namespace cpp_conv::resources
{
    class SDLTileAsset : public RenderableAsset
    {

    };

    using TileAsset = SDLTileAsset;
}
