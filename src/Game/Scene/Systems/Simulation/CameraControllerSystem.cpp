#include "CameraControllerSystem.h"

#include "CameraComponent.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "SDL_mouse.h"
#include "SDL_keyboard.h"
#include "AtlasAppHost/Application.h"
#include "bx/math.h"

namespace
{
    constexpr float c_rotationScaling = 0.01f;
    constexpr float c_moveScaling = 0.01f;
    constexpr float c_keyboardMoveScaling = 0.1f;

    std::tuple<Eigen::Vector3f, Eigen::Vector3f> getForwardAndRight(
        const cpp_conv::components::LookAtCamera& camera,
        const Eigen::Vector3f& up = { 0.0f, 1.0f, 0.0f })
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

        int mouseX, mouseY;

        const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
        const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
        if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0)
        {
            const int deltaX = mouseX - previousMouseX;
            const int deltaY = mouseY- previousMouseY;
            camera.m_Yaw += atlas::maths_helpers::Angle::FromRadians(static_cast<float>(deltaX) * c_rotationScaling);
            camera.m_Pitch += atlas::maths_helpers::Angle::FromRadians(static_cast<float>(deltaY) * c_rotationScaling);
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

        updateControls(camera);
    }
}
