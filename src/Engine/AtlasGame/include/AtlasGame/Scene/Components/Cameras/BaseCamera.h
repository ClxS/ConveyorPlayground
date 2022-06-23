#pragma once

#include <Eigen/Core>

namespace atlas::game::scene::components::cameras
{
    struct BaseCameraComponent
    {
        Eigen::Matrix4f m_ViewMatrix{};
        Eigen::Matrix4f m_ProjectionMatrix{};
        Eigen::Vector2f m_NearFar{0.01f, 100.0f};
        bool m_bIsRenderActive{false};
        bool m_bIsControlActive{false};
    };
}
