#pragma once

namespace cpp_conv::constants
{
    namespace render_views
    {
        constexpr int c_geometry = 1;
        constexpr int c_ui = 2;
        constexpr int c_surface = 3;
        constexpr int c_postProcess = 0;
    }

    namespace render_masks
    {
        constexpr int c_generalGeometry = 1 << 0;
        constexpr int c_surfaceClippedGeometry = 1 << 1;
        constexpr int c_clipCasterGeometry = 1 << 2;
    }
}
