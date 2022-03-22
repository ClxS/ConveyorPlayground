#pragma once
#include <optional>

#include "Eigen/src/Core/Matrix.h"

namespace cpp_conv::components
{
    struct FactoryComponent
    {
        std::optional<Eigen::Vector3i> m_OutputPipe;
    };
}
