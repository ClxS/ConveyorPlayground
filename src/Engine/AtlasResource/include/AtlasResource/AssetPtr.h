#pragma once

#include <memory>

namespace atlas::resource
{
    template <typename T>
    using AssetPtr = std::shared_ptr<T>;
}
