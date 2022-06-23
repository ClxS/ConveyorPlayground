#pragma once

#include "BaseCamera.h"
#include "AtlasCore/MathsHelpers.h"

namespace atlas::game::scene::components::cameras
{
    struct LookAtCameraComponent : BaseCameraComponent
    {
        Eigen::Vector3f m_Up {0.0f, 1.0f, 0.0f};
        Eigen::Vector3f m_LookAtPoint{7.0f, 0.0f, 0.0f};
        atlas::maths_helpers::Angle m_Yaw;
        atlas::maths_helpers::Angle m_Pitch;
        float m_Distance{1.0f};
    };
}
