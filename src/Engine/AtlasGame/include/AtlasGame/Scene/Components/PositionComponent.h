#pragma once

#include <Eigen/Core>

namespace atlas::game::scene::components
{
    struct PositionComponent
    {
        Eigen::Vector3i m_Position{};
    };
}
