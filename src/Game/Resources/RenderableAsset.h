#pragma once
#include "ResourceAsset.h"

namespace cpp_conv::resources
{
    class RenderableAsset : public ResourceAsset
    {
    public:
        ~RenderableAsset() override = default;
    };
}
