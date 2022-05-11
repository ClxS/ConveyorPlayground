#pragma once

#include <cstdint>

#include "bgfx/bgfx.h"

namespace cpp_conv::constants
{
    namespace render_views
    {
        constexpr bgfx::ViewId c_shadowPass = 0;
        constexpr bgfx::ViewId c_geometry = 1;
        constexpr bgfx::ViewId c_postProcess = 2;
        constexpr bgfx::ViewId c_ui = 32;
        constexpr bgfx::ViewId c_debugUi = 33;
    }

    namespace render_masks
    {
        constexpr int c_generalGeometry         = 1 << 0;
        constexpr int c_surfaceClippedGeometry  = 1 << 1;
        constexpr int c_clipCasterGeometry      = 1 << 2;
        constexpr int c_shadowCaster            = 1 << 3;
    }
}
