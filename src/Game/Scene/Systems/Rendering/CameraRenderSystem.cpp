#include "CameraRenderSystem.h"
#include "CameraComponent.h"
#include "Constants.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "AtlasAppHost/Application.h"
#include "AtlasRender/Debug/debugdraw.h"
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

        if (camera.m_bIsRenderActive)
        {
            bgfx::setViewTransform(cpp_conv::constants::render_views::c_geometry, view.data(), projection.data());
        }
    }

    void updateViewProjectMatrix(const cpp_conv::components::SphericalLookAtCamera& camera, cpp_conv::components::SphericalLookAtCamera_Private& cameraPrivate, bool bAddDebugRendering)
    {
        Eigen::Vector3f upVector;
        Eigen::Vector3f forwardVector;
        Eigen::Vector3f cameraForwardVector;
        Eigen::Vector3f cameraRightVector;
        Eigen::Vector3f lookAtRealPosition;
        {
            auto tmpLookSpace = getMatrixForSphericalCoordinate(camera.m_LookAtPitch, camera.m_LookAtYaw, 0.0f);
            upVector = (tmpLookSpace * Eigen::Vector4f{0.0f, 1.0f, 0.0f, 1.0f}).head<3>();
            forwardVector = (tmpLookSpace * Eigen::Vector4f{1.0f, 0.0f, 0.0f, 1.0f}).head<3>();

            auto tmpCameraSpace = getMatrixForSphericalCoordinate(atlas::maths_helpers::Angle::FromDegrees(90.0f), camera.m_CameraYaw, 0.0f);
            cameraForwardVector = (tmpLookSpace * tmpCameraSpace * Eigen::Vector4f{0.0f, -1.0f, 0.0f, 1.0f}).head<3>();
            cameraRightVector = (tmpLookSpace * tmpCameraSpace * Eigen::Vector4f{0.0f, 0.0f, -1.0f, 1.0f}).head<3>();

            lookAtRealPosition = atlas::maths_helpers::sphericalCoordinateToCartesian(camera.m_LookAtPitch, camera.m_LookAtYaw, camera.m_SphericalCentreDistance);
        }

        const Eigen::Vector4f mulIdentity{0.0f, 0.0f, 0.0f, 1.0f};

        const Eigen::Matrix4f lookAtSpaceConversion = getMatrixForSphericalCoordinate(camera.m_LookAtPitch, camera.m_LookAtYaw, camera.m_SphericalCentreDistance);
        const Eigen::Matrix4f cameraSpaceConversion = getMatrixForSphericalCoordinate(camera.m_CameraPitch, camera.m_CameraYaw, camera.m_Distance);

        Eigen::Vector3f lookAtPosition = (lookAtSpaceConversion * mulIdentity).head<3>();
        Eigen::Vector3f cameraPosition = (lookAtSpaceConversion * cameraSpaceConversion * mulIdentity).head<3>();

        auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
        Eigen::Matrix4f view = atlas::maths_helpers::createLookAtMatrix(cameraPosition, lookAtPosition, upVector);
        Eigen::Matrix4f projection = createProjectionMatrix(
            atlas::maths_helpers::Angle::FromDegrees(80),
            static_cast<float>(width) / static_cast<float>(height),
            0.1f,
            100.0f,
            bgfx::getCaps()->homogeneousDepth);

        cameraPrivate.m_Camera = cameraPosition;
        cameraPrivate.m_LookAt = lookAtPosition;
        cameraPrivate.m_View = view;
        cameraPrivate.m_Projection = projection;

        if (camera.m_bIsRenderActive)
        {
            bgfx::setViewTransform(cpp_conv::constants::render_views::c_geometry, view.data(), projection.data());
        }

        using namespace atlas::render::debug;
        if (bAddDebugRendering)
        {
            debug_draw::setWireframe(true);
            {
                const bx::Sphere sphere = { { lookAtPosition.x(), lookAtPosition.y(), lookAtPosition.z() }, 0.25f };
                debug_draw::createScope();
                debug_draw::setColor(0xfff0c0ff);
                debug_draw::createScope();
                debug_draw::setLod(0);
                debug_draw::draw(sphere);
            }

            if constexpr (false)
            {
                debug_draw::setWireframe(true);
                {
                    const bx::Sphere sphere = { { cameraPosition.x(), cameraPosition.y(), cameraPosition.z() }, 0.15f };
                    debug_draw::createScope();
                    debug_draw::setColor(0xff000000);
                    debug_draw::createScope();
                    debug_draw::setLod(0);
                    debug_draw::draw(sphere);
                }
            }

            debug_draw::setWireframe(true);
            {
                const bx::Sphere sphere = { { lookAtRealPosition.x(), lookAtRealPosition.y(), lookAtRealPosition.z() }, 0.15f };
                debug_draw::createScope();
                debug_draw::setColor(0xff0000ff);
                debug_draw::createScope();
                debug_draw::setLod(0);
                debug_draw::draw(sphere);
            }

            {
                debug_draw::createScope();
                debug_draw::setColor(0xff000000);
                debug_draw::moveTo({ cameraPosition.x(), cameraPosition.y(), cameraPosition.z() });
                debug_draw::lineTo({ lookAtPosition.x(), lookAtPosition.y(), lookAtPosition.z() });
            }
            {
                debug_draw::createScope();
                const auto cameraForwardEnd = lookAtPosition + cameraForwardVector;
                debug_draw::setColor(0xff444444);
                debug_draw::moveTo({ lookAtPosition.x(), lookAtPosition.y(), lookAtPosition.z() });
                debug_draw::lineTo({ cameraForwardEnd.x(), cameraForwardEnd.y(), cameraForwardEnd.z() });

                const auto cameraRightEnd = lookAtPosition + cameraRightVector;
                debug_draw::setColor(0xffff4444);
                debug_draw::moveTo({ lookAtPosition.x(), lookAtPosition.y(), lookAtPosition.z() });
                debug_draw::lineTo({ cameraRightEnd.x(), cameraRightEnd.y(), cameraRightEnd.z() });
            }

            {
                debug_draw::createScope();
                const auto upEnd = lookAtPosition + upVector;
                debug_draw::setColor(0xff4a33ff);
                debug_draw::moveTo({ lookAtPosition.x(), lookAtPosition.y(), lookAtPosition.z() });
                debug_draw::lineTo({ upEnd.x(), upEnd.y(), upEnd.z() });
            }

            {
                debug_draw::createScope();
                const auto forwardEnd = lookAtPosition + forwardVector;
                debug_draw::setColor(0xff69ff29);
                debug_draw::moveTo({ lookAtPosition.x(), lookAtPosition.y(), lookAtPosition.z() });
                debug_draw::lineTo({ forwardEnd.x(), forwardEnd.y(), forwardEnd.z() });
            }

            debug_draw::setWireframe(false);
        }
    }
}

void cpp_conv::CameraRenderSystem::Initialise(atlas::scene::EcsManager& ecsManager)
{
    atlas::render::debug::initialise();
}

void cpp_conv::CameraRenderSystem::Update(atlas::scene::EcsManager& ecs)
{
    using namespace atlas::scene;
    using namespace cpp_conv::components;
    using namespace atlas::render::debug;

    debug_draw::drawGrid({ 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 100);

    for(auto [entity, camera, cameraPrivate] : ecs.IterateEntityComponents<SphericalLookAtCamera, SphericalLookAtCamera_Private>())
    {
        updateViewProjectMatrix(camera, cameraPrivate, m_bDebugRenderingEnabled);
        //camera.m_LookAtYaw += atlas::maths_helpers::Angle::FromRadians(0.02f);
        //camera.m_LookAtPitch += atlas::maths_helpers::Angle::FromRadians(0.005f);

        //camera.m_CameraPitch += atlas::maths_helpers::Angle::FromRadians(0.01f);
        if (camera.m_CameraPitch > atlas::maths_helpers::Angle::FromDegrees(80.0f))
        {
            //camera.m_CameraPitch = atlas::maths_helpers::Angle::FromDegrees(10.0f);
        }
    }

    for(auto [entity, camera] : ecs.IterateEntityComponents<LookAtCamera>())
    {
        if (!camera.m_bIsRenderActive)
        {
            continue;
        }

        updateViewProjectMatrix(camera);
    }
}

void cpp_conv::CameraRenderSystem::SetDebugRenderingEnabled(const bool bEnabled)
{
    m_bDebugRenderingEnabled = bEnabled;
}
