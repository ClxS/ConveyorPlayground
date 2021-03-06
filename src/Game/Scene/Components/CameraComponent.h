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
        bool m_bIsRenderActive{false};
        bool m_bIsControlActive{false};
    };

    struct LookAtCamera : public CameraComponent
    {
        Eigen::Vector3f m_Up {0.0f, 1.0f, 0.0f};
        Eigen::Vector3f m_LookAtPoint{7.0f, 0.0f, 0.0f};
        atlas::maths_helpers::Angle m_Yaw;
        atlas::maths_helpers::Angle m_Pitch;
        float m_Distance;
    };

    struct SphericalLookAtCamera : public CameraComponent
    {
        Eigen::Vector3f m_SphericalCentre;
        float m_SphericalCentreDistance;
        atlas::maths_helpers::Angle m_LookAtYaw;
        atlas::maths_helpers::Angle m_LookAtPitch;
        atlas::maths_helpers::Angle m_CameraYaw;
        atlas::maths_helpers::Angle m_CameraPitch;
        float m_Distance;
    };

    struct SphericalLookAtCamera_Private
    {
        Eigen::Vector3f m_LookAt;
        Eigen::Vector3f m_Camera;
        Eigen::Matrix4f m_View;
        Eigen::Matrix4f m_Projection;
    };

    struct FreeCamera : public CameraComponent
    {
        Eigen::Vector3f m_CameraPosition;
        Eigen::Quaternionf m_Orientation;
    };
}
