#pragma once
#include "AtlasResource/AssetPtr.h"

namespace atlas
{
    namespace render
    {
        class ModelAsset;
    }
}

namespace cpp_conv::components
{
    struct ModelComponent
    {
        atlas::resource::AssetPtr<atlas::render::ModelAsset> m_Model;
    };
}
