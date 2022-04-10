#pragma once

#include "TileAsset.h"
#include "AtlasResource/AssetPtr.h"

namespace cpp_conv::components
{
    template <int32_t Layer>
    struct SpriteLayerComponent
    {
        atlas::resource::AssetPtr<resources::TileAsset> m_pTile;
        float m_RotationRadians = 0.0f;
    };
}
