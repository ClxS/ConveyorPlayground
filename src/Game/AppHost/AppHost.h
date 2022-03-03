#pragma once

#include <tuple>

namespace cpp_conv::apphost
{
    std::tuple<int, int> getAppDimensions();

    std::tuple<int, int> getCursorPosition();
}
