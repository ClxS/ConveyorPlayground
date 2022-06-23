#pragma once
#include "BaseCamera.h"
#include "AtlasCore/MathsHelpers.h"

namespace atlas::game::scene::components::cameras
{
    struct SphericalLookAtCameraComponent : BaseCameraComponent
    {
        Eigen::Vector3f m_SphericalCentre;
        float m_SphericalCentreDistance{1.0f};
        atlas::maths_helpers::Angle m_LookAtYaw;
        atlas::maths_helpers::Angle m_LookAtPitch;
        atlas::maths_helpers::Angle m_CameraYaw;
        atlas::maths_helpers::Angle m_CameraPitch;
        float m_Distance{1.0f};
    };

    struct SphericalLookAtCameraComponent_Private
    {
        Eigen::Vector3f m_LookAt;
        Eigen::Vector3f m_Camera;
        Eigen::Matrix4f m_View;
        Eigen::Matrix4f m_Projection;
    };
}
