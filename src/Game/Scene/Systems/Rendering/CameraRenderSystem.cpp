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
    /*Eigen::Matrix3f getSphericalConversionMatrix(const Eigen::Vector3f referenceUpVector, const Eigen::Vector3f currentUpVector)
    {
        if (currentUpVector == referenceUpVector)
        {
            return Eigen::Matrix3f::Identity();
        }

        const Eigen::Vector3f cross = referenceUpVector.cross(currentUpVector).normalized();
        Eigen::Matrix3f m;
        m.col(0) = referenceUpVector.normalized();
        m.col(1) = cross.cross(referenceUpVector).normalized();
        m.col(2) = cross;
        return m;
    }*/

    Eigen::Matrix4f getSphericalConversionMatrix(const atlas::maths_helpers::Angle pitch, const atlas::maths_helpers::Angle yaw, const float distance)
    {
        const Eigen::Affine3f translation{Eigen::Translation3f( 0.0f, distance, 0.0f )};
        const Eigen::Affine3f rotYaw{Eigen::AngleAxisf{yaw.AsRadians(), Eigen::Vector3f::UnitZ()}};
        const Eigen::Affine3f rotPitch{Eigen::AngleAxisf{pitch.AsRadians(), Eigen::Vector3f::UnitY()}};
        Eigen::Matrix4f m = (rotPitch * rotYaw * translation).matrix();
        return m;
    }

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

    Eigen::Vector4f expandForMul(const Eigen::Vector3f& input)
    {
        return Eigen::Vector4f{input.x(), input.y(), input.z(), 1.0f};
    }

    void updateViewProjectMatrix(const cpp_conv::components::SphericalLookAtCamera& camera)
    {
        Eigen::Vector3f upVector;
        Eigen::Vector3f forwardVector;
        {
            const auto tmp = getSphericalConversionMatrix(camera.m_LookAtYaw, camera.m_LookAtPitch, 0.0f);
            upVector = (tmp * Eigen::Vector4f{0.0f, 1.0f, 0.0f, 1.0f}).head<3>();
            forwardVector = (tmp * Eigen::Vector4f{1.0f, 0.0f, 0.0f, 1.0f}).head<3>();
        }

        const Eigen::Vector4f mulIdentity{0.0f, 0.0f, 0.0f, 1.0f};

        const Eigen::Matrix4f lookAtSpaceConversion = getSphericalConversionMatrix(camera.m_LookAtYaw, camera.m_LookAtPitch, camera.m_SphericalCentreDistance);
        const auto cameraSpaceConversion = getSphericalConversionMatrix(camera.m_CameraYaw, camera.m_CameraPitch, camera.m_Distance);

        Eigen::Vector3f lookAtPosition = (lookAtSpaceConversion * mulIdentity).head<3>();
        Eigen::Vector3f cameraPosition = (lookAtSpaceConversion * cameraSpaceConversion * mulIdentity).head<3>();

        auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();
        Eigen::Matrix4f view = atlas::maths_helpers::createLookAtMatrix(lookAtPosition - cameraPosition, lookAtPosition, upVector);
        Eigen::Matrix4f projection = createProjectionMatrix(
            atlas::maths_helpers::Angle::FromDegrees(60),
            static_cast<float>(width) / static_cast<float>(height),
            0.1f,
            100.0f,
            bgfx::getCaps()->homogeneousDepth);

        bgfx::setViewTransform(cpp_conv::constants::render_views::c_geometry, view.data(), projection.data());

        using namespace atlas::render::debug;
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

            debug_draw::setWireframe(true);
            {
                const bx::Sphere sphere = { { cameraPosition.x(), cameraPosition.y(), cameraPosition.z() }, 0.15f };
                debug_draw::createScope();
                debug_draw::setColor(0xff000000);
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

    debug_draw::begin(constants::render_views::c_geometry);
    debug_draw::drawAxis(0.0f, 0.0f, 0.0f);

    for(auto [entity, camera] : ecs.IterateEntityComponents<SphericalLookAtCamera>())
    {
        if (!camera.m_bIsActive)
        {
            continue;
        }

        updateViewProjectMatrix(camera);
        camera.m_LookAtYaw += atlas::maths_helpers::Angle::FromRadians(0.02f);
        camera.m_LookAtPitch += atlas::maths_helpers::Angle::FromRadians(0.005f);

        //camera.m_CameraPitch += atlas::maths_helpers::Angle::FromRadians(0.01f);
        if (camera.m_CameraPitch > atlas::maths_helpers::Angle::FromDegrees(80.0f))
        {
            //camera.m_CameraPitch = atlas::maths_helpers::Angle::FromDegrees(10.0f);
        }
    }

    for(auto [entity, camera] : ecs.IterateEntityComponents<LookAtCamera>())
    {
        if (!camera.m_bIsActive)
        {
            continue;
        }

        updateViewProjectMatrix(camera);
    }

    debug_draw::end();
}
