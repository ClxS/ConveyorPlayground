#pragma once

#include <tuple>

namespace cpp_conv::apphost
{
    std::tuple<int, int> getAppDimensions();
    void setAppDimensions(int width, int height);
}