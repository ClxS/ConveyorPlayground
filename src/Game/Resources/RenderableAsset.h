#pragma once
#include "ResourceAsset.h"

namespace cpp_conv::resources
{
    class RenderableAsset : public ResourceAsset
    {
    public:
        virtual ~RenderableAsset() = default;
    };
}