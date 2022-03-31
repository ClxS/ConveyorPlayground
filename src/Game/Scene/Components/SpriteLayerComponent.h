#pragma once

#include "AssetPtr.h"
#include "TileAsset.h"

namespace cpp_conv::components
{
    template <int32_t Layer>
    struct SpriteLayerComponent
    {
        resources::AssetPtr<resources::TileAsset> m_pTile;
        float m_RotationRadians = 0.0f;
    };
}
