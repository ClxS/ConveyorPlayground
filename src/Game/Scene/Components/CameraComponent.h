#pragma once
#include "AtlasCore/MathsHelpers.h"
#include "Eigen/Core"
#include "Eigen/Geometry"

namespace cpp_conv::components
{
    struct CameraComponent
    {
        Eigen::Matrix4f m_ViewMatrix{};
        Eigen::Matrix4f m_ProjectionMatrix{};
        Eigen::Vector2f m_NearFar{0.01f, 100.0f};
        bool m_bIsActive{false};
    };

    struct LookAtCamera : public CameraComponent
    {
        Eigen::Vector3f m_Up {0.0f, 1.0f, 0.0f};
        Eigen::Vector3f m_LookAtPoint{7.0f, 0.0f, 0.0f};
        atlas::maths_helpers::Angle m_Yaw;
        atlas::maths_helpers::Angle m_Pitch;
        float m_Distance;
    };

    struct FreeCamera : public CameraComponent
    {
        Eigen::Vector3f m_CameraPosition;
        Eigen::Quaternionf m_Orientation;
    };
}
