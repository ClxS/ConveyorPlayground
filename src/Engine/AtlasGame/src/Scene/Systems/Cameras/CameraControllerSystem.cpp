#include "AtlasGamePCH.h"
#include "Scene/Systems/Cameras/CameraControllerSystem.h"

#include "imgui.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "SDL_mouse.h"
#include "SDL_keyboard.h"
#include "AtlasAppHost/Application.h"
#include "bx/math.h"
#include "Scene/Components/Cameras/LookAtCameraComponent.h"
#include "Scene/Components/Cameras/SphericalLookAtCameraComponent.h"

namespace
{
    constexpr float c_rotationScaling = 0.01f;
    constexpr float c_moveScaling = 0.01f;
    constexpr float c_keyboardMoveScaling = 0.1f;

    std::tuple<Eigen::Vector3f, Eigen::Vector3f> getForwardAndRight(
        const atlas::game::scene::components::cameras::LookAtCameraComponent& camera,
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

    void getSphericalCoordinates(const Eigen::Vector3f position, atlas::maths_helpers::Angle& pitch, atlas::maths_helpers::Angle& yaw)
    {
        const float r = std::hypot(position.x(), position.y(), position.z());
        pitch = atlas::maths_helpers::Angle::FromRadians(std::acos(position.y() / r), atlas::maths_helpers::Angle::WrapMode::None);
        if (position.x() != 0.0f)
        {
            yaw = atlas::maths_helpers::Angle::FromRadians(std::atan2(position.z(), position.x()), atlas::maths_helpers::Angle::WrapMode::None);
        }
        else
        {
            yaw = atlas::maths_helpers::Angle::FromRadians(0.0f, atlas::maths_helpers::Angle::WrapMode::None);
        }
    }

    void moveCamera(atlas::game::scene::components::cameras::SphericalLookAtCameraComponent& camera, float forwardMovement, float rightMovement)
    {
        Eigen::Vector3f cameraForwardVector;
        Eigen::Vector3f cameraRightVector;
        {
            const Eigen::Matrix4f tmpLookSpace = atlas::maths_helpers::getMatrixForSphericalCoordinate(camera.m_LookAtPitch, camera.m_LookAtYaw, 0.0f);
            const Eigen::Matrix4f tmpCameraSpace = getMatrixForSphericalCoordinate(atlas::maths_helpers::Angle::FromDegrees(90.0f), camera.m_CameraYaw, 0.0f);
            cameraForwardVector = (tmpLookSpace * tmpCameraSpace * Eigen::Vector4f{0.0f, -1.0f, 0.0f, 1.0f}).head<3>().normalized();
            cameraRightVector = (tmpLookSpace * tmpCameraSpace * Eigen::Vector4f{0.0f, 0.0f, -1.0f, 1.0f}).head<3>().normalized();
        }

        const Eigen::Matrix4f lookAtSpaceConversion = atlas::maths_helpers::getMatrixForSphericalCoordinate(camera.m_LookAtPitch, camera.m_LookAtYaw, camera.m_SphericalCentreDistance);

        const Eigen::Vector3f lookAtPosition = (lookAtSpaceConversion * Eigen::Vector4f{0.0f, 0.0f, 0.0f, 1.0f}).head<3>();
        Eigen::Vector3f finalPosition = lookAtPosition;

        // Move the LookAt position then clamp it back to the spheres surface
        finalPosition += cameraForwardVector.normalized() * forwardMovement;
        finalPosition += cameraRightVector.normalized() * rightMovement;
        finalPosition = finalPosition.normalized() * camera.m_SphericalCentreDistance;

        atlas::maths_helpers::Angle finalYaw;
        atlas::maths_helpers::Angle finalPitch;
        getSphericalCoordinates(finalPosition, finalPitch, finalYaw);

        camera.m_LookAtYaw = finalYaw;
        camera.m_LookAtPitch = finalPitch;
    }


    bool updateControls(atlas::game::scene::components::cameras::LookAtCameraComponent& camera)
    {
        // TODO These should be moved into AtlasInput as non-statics once it exists
        static int previousMouseX = 0;
        static int previousMouseY = 0;

        int mouseX, mouseY;

        float speedFactor = camera.m_Distance / 1.0f;

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
            forward *= static_cast<float>(deltaY) * c_moveScaling * speedFactor;
            right *= static_cast<float>(-deltaX) * c_moveScaling * speedFactor;

            camera.m_LookAtPoint += forward;
            camera.m_LookAtPoint += right;
        }

        if(keyboardState[SDL_SCANCODE_W])
        {
            auto [forward, _] = getForwardAndRight(camera);
            forward *= static_cast<float>(1) * c_keyboardMoveScaling * speedFactor;
            camera.m_LookAtPoint += forward;
        }
        else if(keyboardState[SDL_SCANCODE_S])
        {
            auto [forward, _] = getForwardAndRight(camera);
            forward *= static_cast<float>(1) * -c_keyboardMoveScaling * speedFactor;
            camera.m_LookAtPoint += forward;
        }

        if(keyboardState[SDL_SCANCODE_A])
        {
            auto [_, right] = getForwardAndRight(camera);
            right *= static_cast<float>(1) * -c_keyboardMoveScaling * speedFactor;
            camera.m_LookAtPoint += right;
        }
        else if(keyboardState[SDL_SCANCODE_D])
        {
            auto [_, right] = getForwardAndRight(camera);
            right *= static_cast<float>(1) * c_keyboardMoveScaling * speedFactor;
            camera.m_LookAtPoint += right;
        }

        previousMouseX = mouseX;
        previousMouseY = mouseY;
        return true;
    }

    bool updateControls(atlas::game::scene::components::cameras::SphericalLookAtCameraComponent& camera)
    {
        // TODO These should be moved into AtlasInput as non-statics once it exists
        static int previousMouseX = 0;
        static int previousMouseY = 0;

        int mouseX, mouseY;

        const float speedFactor = camera.m_Distance / 1.8f;

        const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
        const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
        if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0)
        {
            const int deltaX = mouseX - previousMouseX;
            const int deltaY = mouseY- previousMouseY;
            camera.m_CameraYaw -= atlas::maths_helpers::Angle::FromRadians(
                static_cast<float>(deltaX) * c_rotationScaling,
                atlas::maths_helpers::Angle::WrapMode::None);
            camera.m_CameraPitch -= atlas::maths_helpers::Angle::FromRadians(
                static_cast<float>(deltaY) * c_rotationScaling,
                atlas::maths_helpers::Angle::WrapMode::None);
        }

        if(keyboardState[SDL_SCANCODE_W])
        {
            moveCamera(camera, c_keyboardMoveScaling * speedFactor, 0.0f);
        }
        else if(keyboardState[SDL_SCANCODE_S])
        {
            moveCamera(camera, c_keyboardMoveScaling * -speedFactor, 0.0f);
        }

        if(keyboardState[SDL_SCANCODE_A])
        {
            moveCamera(camera, 0.0f, c_keyboardMoveScaling * speedFactor);
        }
        else if(keyboardState[SDL_SCANCODE_D])
        {
            moveCamera(camera, 0.0f, c_keyboardMoveScaling * -speedFactor);
        }

        previousMouseX = mouseX;
        previousMouseY = mouseY;
        return true;
    }
}

void atlas::game::scene::systems::cameras::CameraControllerSystem::Update(atlas::scene::EcsManager& ecs)
{
    using namespace atlas::scene;
    if (ImGui::IsAnyItemHovered())
    {
        return;
    }

    for(auto [entity, camera] : ecs.IterateEntityComponents<components::cameras::LookAtCameraComponent>())
    {
        if (!camera.m_bIsControlActive)
        {
            continue;
        }

        updateControls(camera);
    }

    for(auto [entity, camera] : ecs.IterateEntityComponents<components::cameras::SphericalLookAtCameraComponent>())
    {
        if (!camera.m_bIsControlActive)
        {
            continue;
        }

        updateControls(camera);
    }
}
