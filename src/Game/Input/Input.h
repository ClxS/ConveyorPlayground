#pragma once

#include <queue>
#include "CommandType.h"
#include "RenderContext.h"

namespace cpp_conv
{
    struct RenderContext;
}

namespace cpp_conv
{
    struct SceneContext;
}

namespace cpp_conv::input
{
    void receiveInput(
        /*SceneContext& kContext, RenderContext& kRenderContext, std::queue<commands::CommandType>& commands*/);
}
