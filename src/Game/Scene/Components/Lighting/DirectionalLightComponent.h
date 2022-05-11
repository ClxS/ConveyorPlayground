#pragma once
#include "Eigen/Core"

namespace cpp_conv::components
{
    struct DirectionalLightComponent
    {
        Eigen::Vector3f m_LightDirection;
        Eigen::Vector4f m_LightColour;
    };
}
