#pragma once

#include <memory>

namespace cpp_conv::resources
{
    template <typename T>
    using AssetPtr = std::shared_ptr<T>;
}
