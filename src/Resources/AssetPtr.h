#pragma once

namespace cpp_conv::resources
{
    template<typename T>
    using AssetPtr = std::shared_ptr<T>;
}