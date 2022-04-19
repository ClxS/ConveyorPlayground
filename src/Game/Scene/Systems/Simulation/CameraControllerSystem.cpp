#include "CameraControllerSystem.h"

#include "CameraComponent.h"
#include "SDL_events.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "SDL_mouse.h"
#include "SDL_keyboard.h"
#include "AtlasAppHost/Application.h"
#include "bgfx/bgfx.h"
#include "bx/math.h"

namespace
{
    constexpr float c_rotationScaling = 0.01f;
    constexpr float c_moveScaling = 0.01f;
    constexpr float c_keyboardMoveScaling = 0.1f;

    std::tuple<Eigen::Vector3f, Eigen::Vector3f> getForwardAndRight(
        const cpp_conv::components::LookAtCamera& camera,
        const Eigen::Vector3f up = { 0.0f, 1.0f, 0.0f })
    {
        Eigen::Matrix3f cameraRotation;
        cameraRotation =
            Eigen::AngleAxisf(0.0f, Eigen::Vector3f::UnitX())
          * Eigen::AngleAxisf(camera.m_Yaw.AsRadians(), Eigen::Vector3f::UnitY())
          * Eigen::AngleAxisf(-camera.m_Pitch.AsRadians(), Eigen::Vector3f::UnitZ());

        Eigen::Vector3f forward = { camera.m_Distance, 0.0f, 0.0f };
        forward = (cameraRotation * forward).normalized();
        forward[1] = 0.0f;
        Eigen::Vector3f right = up.cross(forward);
        return { forward, right };
    }

    bool updateControls(cpp_conv::components::LookAtCamera& camera)
    {
        // TODO These should be moved into AtlasInput as non-statics once it exists
        static int previousMouseX = 0;
        static int previousMouseY = 0;

        using namespace cpp_conv::math_helpers;
        int mouseX, mouseY;

        const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
        const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
        if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0)
        {
            const int deltaX = mouseX - previousMouseX;
            const int deltaY = mouseY- previousMouseY;
            camera.m_Yaw += Angle::FromRadians(static_cast<float>(deltaX) * c_rotationScaling);
            camera.m_Pitch += Angle::FromRadians(static_cast<float>(deltaY) * c_rotationScaling);
        }
        else if ((buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0)
        {
            const int deltaX = mouseX - previousMouseX;
            const int deltaY = mouseY - previousMouseY;

            auto [forward, right] = getForwardAndRight(camera);
            forward *= static_cast<float>(deltaY) * c_moveScaling;
            right *= static_cast<float>(-deltaX) * c_moveScaling;

            camera.m_LookAtPoint += forward;
            camera.m_LookAtPoint += right;
        }

        if(keyboardState[SDL_SCANCODE_W])
        {
            auto [forward, _] = getForwardAndRight(camera);
            forward *= static_cast<float>(1) * c_keyboardMoveScaling;
            camera.m_LookAtPoint += forward;
        }
        else if(keyboardState[SDL_SCANCODE_S])
        {
            auto [forward, _] = getForwardAndRight(camera);
            forward *= static_cast<float>(1) * -c_keyboardMoveScaling;
            camera.m_LookAtPoint += forward;
        }

        if(keyboardState[SDL_SCANCODE_A])
        {
            auto [_, right] = getForwardAndRight(camera);
            right *= static_cast<float>(1) * -c_keyboardMoveScaling;
            camera.m_LookAtPoint += right;
        }
        else if(keyboardState[SDL_SCANCODE_D])
        {
            auto [_, right] = getForwardAndRight(camera);
            right *= static_cast<float>(1) * c_keyboardMoveScaling;
            camera.m_LookAtPoint += right;
        }

        previousMouseX = mouseX;
        previousMouseY = mouseY;
        return true;
    }

    Eigen::Matrix4f lookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& target, const Eigen::Vector3f& inUp, bool leftHanded = true)
    {
        const Eigen::Vector3f view = (leftHanded ? (target - eye) : (eye - target)).normalized();
        const Eigen::Vector3f right = inUp.cross(view).normalized();
        const Eigen::Vector3f up = view.cross(right);

        Eigen::Matrix4f mat = Eigen::Matrix4f::Zero();
        mat.row(0) = Eigen::Vector4f{right.x(), right.y(), right.z(), -right.dot(eye)};
        mat.row(1) = Eigen::Vector4f{up.x(), up.y(), up.z(), -up.dot(eye)};
        mat.row(2) = Eigen::Vector4f{view.x(), view.y(), view.z(), -view.dot(eye)};
        mat(3, 3) = 1.0f;
        return mat;
    }

    Eigen::Matrix4f project(
        const cpp_conv::math_helpers::Angle fovY,
        const float aspectRatio,
        float near,
        float far,
        bool homogenousDepth,
        bool leftHanded = true)
    {
        const float height = 1.0f/tan(fovY.AsRadians() * 0.5f);
        const float width  = height * 1.0f / aspectRatio;

        const float diff = far - near;
        const float aa = homogenousDepth ? (      far + near)/diff : far/diff;
        const float bb = homogenousDepth ? (2.0f* far * near)/diff : near*aa;

        Eigen::Matrix4f mat = Eigen::Matrix4f::Zero();
        mat(0, 0) = width;
        mat(1, 1) = height;
        mat(2, 2) = leftHanded ? aa : -aa;
        mat(3, 2) = leftHanded ? 1.0f: -1.0f;
        mat(2, 3) = -bb;
        return mat;
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
        Eigen::Matrix4f view = lookAt(camera.m_LookAtPoint - cameraOffset, camera.m_LookAtPoint, camera.m_Up);
        Eigen::Matrix4f projection = project(
            cpp_conv::math_helpers::Angle::FromDegrees(60),
            static_cast<float>(width) / static_cast<float>(height),
            0.1f,
            100.0f,
            bgfx::getCaps()->homogeneousDepth);

        bgfx::setViewTransform(0, view.data(), projection.data());
    }
}

void cpp_conv::CameraControllerSystem::Update(atlas::scene::EcsManager& ecs)
{
    using namespace atlas::scene;
    using namespace cpp_conv::components;

    for(auto [entity, camera] : ecs.IterateEntityComponents<LookAtCamera>())
    {
        if (!camera.m_bIsActive)
        {
            continue;
        }

        if (updateControls(camera))
        {
            updateViewProjectMatrix(camera);
        }
    }
}
