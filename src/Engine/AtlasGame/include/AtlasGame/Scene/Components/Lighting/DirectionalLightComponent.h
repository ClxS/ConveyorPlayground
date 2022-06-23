#pragma once

#include <Eigen/Core>

namespace atlas::game::scene::components::cameras
{
    struct DirectionalLightComponent
    {
        Eigen::Vector3f m_LightDirection;
        Eigen::Vector4f m_LightColour;
    };
}
