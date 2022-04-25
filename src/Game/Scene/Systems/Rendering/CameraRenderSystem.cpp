#include "CameraRenderSystem.h"
#include "CameraComponent.h"
#include "Constants.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasAppHost/Application.h"
#include "bgfx/bgfx.h"
#include "bx/math.h"

namespace
{
    void updateViewProjectMatrix(const cpp_conv::components::LookAtCamera& camera)
    {
        Eigen::Matrix3f cameraRotation;
        cameraRotation =
            Eigen::AngleAxisf(0.0f, Eigen::Vector3f::UnitX())
          * Eigen::AngleAxisf(camera.m_Yaw.AsRadians(), Eigen::Vector3f::UnitY())
          * Eigen::AngleAxisf(-camera.m_Pitch.AsRadians(), Eigen::Vector3f::UnitZ());

        Eigen::Vector3f cameraOffset = { camera.m_Distance, 0.0f, 0.0f };
        cameraOffset = cameraRotation * cameraOffset;

        auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
        Eigen::Matrix4f view = atlas::maths_helpers::createLookAtMatrix(camera.m_LookAtPoint - cameraOffset, camera.m_LookAtPoint, camera.m_Up);
        Eigen::Matrix4f projection = createProjectionMatrix(
            atlas::maths_helpers::Angle::FromDegrees(60),
            static_cast<float>(width) / static_cast<float>(height),
            0.1f,
            100.0f,
            bgfx::getCaps()->homogeneousDepth);

        bgfx::setViewTransform(cpp_conv::constants::render_views::c_geometry, view.data(), projection.data());
    }
}

void cpp_conv::CameraRenderSystem::Update(atlas::scene::EcsManager& ecs)
{
    using namespace atlas::scene;
    using namespace cpp_conv::components;

    for(auto [entity, camera] : ecs.IterateEntityComponents<LookAtCamera>())
    {
        if (!camera.m_bIsActive)
        {
            continue;
        }

        updateViewProjectMatrix(camera);
    }
}
