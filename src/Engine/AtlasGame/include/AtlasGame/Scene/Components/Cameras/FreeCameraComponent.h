#pragma once

#include <Eigen/Geometry>
#include "BaseCamera.h"

namespace atlas::game::scene::components::cameras
{
    struct FreeCameraComponent : BaseCameraComponent
    {
        Eigen::Vector3f m_CameraPosition;
        Eigen::Quaternionf m_Orientation;
    };
}
