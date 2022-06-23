#include "GameSceneDebugUI.h"

#include <format>

#include "Constants.h"
#include "imgui.h"
#include "AtlasAppHost/Application.h"
#include "AtlasAppHost/PlatformApplication.h"
#include "AtlasGame/Scene/Components/Cameras/LookAtCameraComponent.h"
#include "AtlasGame/Scene/Components/Cameras/SphericalLookAtCameraComponent.h"
#include "AtlasGame/Scene/Systems/Cameras/CameraViewProjectionUpdateSystem.h"
#include "AtlasGame/Utility/ImguiBgfx/ImguiBgfxImpl.h"
#include "AtlasScene/ECS/Components/EcsManager.h"
#include "backends/imgui_impl_sdl.h"

namespace
{
    void newFrame()
    {
        atlas::app_host::platform::PlatformApplication& platform = atlas::app_host::Application::Get().GetPlatform();
        ImGui_ImplSDL2_NewFrame(platform.GetSDLContext().m_Window);
        ImGui_Implbgfx_NewFrame();
        ImGui::NewFrame();
    }

    void present()
    {
        ImGui::Render();

        ImDrawData* pDrawData = ImGui::GetDrawData();
        if (!pDrawData)
        {
            return;
        }

        ImGui_Implbgfx_RenderDrawLists(pDrawData);
    }

    void shutdown()
    {
        ImGui_Implbgfx_Shutdown();
        ImGui::DestroyContext();
    }

    enum class ActiveCameraType
    {
        Rendering,
        Control
    };

    std::string getCameraIdentifier(const atlas::scene::EntityId entityId, const atlas::game::scene::components::cameras::SphericalLookAtCameraComponent&)
    {
        return std::format("SphericalLookAt ({})", entityId.m_Value);
    }

    std::string getCameraIdentifier(const atlas::scene::EntityId entityId, const atlas::game::scene::components::cameras::LookAtCameraComponent&)
    {
        return std::format("LookAt ({})", entityId.m_Value);
    }

    std::string findCurrentCamera(atlas::scene::EcsManager& ecs, ActiveCameraType type)
    {
        using namespace atlas::scene;
        using namespace cpp_conv;

        for(auto [entity, camera] : ecs.IterateEntityComponents<atlas::game::scene::components::cameras::SphericalLookAtCameraComponent>())
        {
            if ((!camera.m_bIsRenderActive && type == ActiveCameraType::Rendering) || (!camera.m_bIsControlActive && type == ActiveCameraType::Control))
            {
                continue;
            }

            return getCameraIdentifier(entity, camera);
        }

        for(auto [entity, camera] : ecs.IterateEntityComponents<atlas::game::scene::components::cameras::LookAtCameraComponent>())
        {
            if ((!camera.m_bIsRenderActive && type == ActiveCameraType::Rendering) || (!camera.m_bIsControlActive && type == ActiveCameraType::Control))
            {
                continue;
            }

            return getCameraIdentifier(entity, camera);
        }

        return "";
    }

    bool addComboItem(std::string& selectedItem, const std::string& id)
    {
        const bool isSelected = (selectedItem == id);
        if (ImGui::Selectable(id.c_str(), isSelected))
        {
            selectedItem = id;
            return true;
        }

        if (isSelected)
        {
            ImGui::SetItemDefaultFocus();
        }

        return false;
    }

    void setActiveCamera(atlas::scene::EcsManager& ecs, atlas::game::scene::components::cameras::BaseCameraComponent& activeCamera, const ActiveCameraType type)
    {
        using namespace cpp_conv;
        for(auto [entity, camera] : ecs.IterateEntityComponents<atlas::game::scene::components::cameras::SphericalLookAtCameraComponent>())
        {
            if (type == ActiveCameraType::Rendering)
            {
                camera.m_bIsRenderActive = false;
            }
            else
            {
                camera.m_bIsControlActive = false;
            }
        }

        for(auto [entity, camera] : ecs.IterateEntityComponents<atlas::game::scene::components::cameras::LookAtCameraComponent>())
        {
            if (type == ActiveCameraType::Rendering)
            {
                camera.m_bIsRenderActive = false;
            }
            else
            {
                camera.m_bIsControlActive = false;
            }
        }

        if (type == ActiveCameraType::Rendering)
        {
            activeCamera.m_bIsRenderActive = true;
        }
        else
        {
            activeCamera.m_bIsControlActive = true;
        }
    }

    void addCameraDebugUi(atlas::scene::EcsManager& ecs, atlas::game::scene::systems::cameras::CameraViewProjectionUpdateSystem* pCameraRenderer)
    {
        if (!pCameraRenderer)
        {
            return;
        }

        using namespace atlas::scene;
        using namespace cpp_conv;

        static bool s_enableDebug{true};
        ImGui::Text("Camera Debug");
        ImGui::Checkbox("Enable Debug Rendering", &s_enableDebug);

        static std::string s_selectedControlCamera = findCurrentCamera(ecs, ActiveCameraType::Control);
        if (ImGui::BeginCombo("Control Camera", s_selectedControlCamera.c_str()))
        {
            for(auto [entity, camera] : ecs.IterateEntityComponents<atlas::game::scene::components::cameras::SphericalLookAtCameraComponent>())
            {
                if (addComboItem(s_selectedControlCamera, getCameraIdentifier(entity, camera)))
                {
                    setActiveCamera(ecs, camera, ActiveCameraType::Control);
                }
            }
            for(auto [entity, camera] : ecs.IterateEntityComponents<atlas::game::scene::components::cameras::LookAtCameraComponent>())
            {
                if (addComboItem(s_selectedControlCamera, getCameraIdentifier(entity, camera)))
                {
                    setActiveCamera(ecs, camera, ActiveCameraType::Control);
                }
            }

            ImGui::EndCombo();
        }

        static std::string s_selectedRenderCamera = findCurrentCamera(ecs, ActiveCameraType::Rendering);
        if (ImGui::BeginCombo("Render Camera", s_selectedRenderCamera.c_str()))
        {
            for(auto [entity, camera] : ecs.IterateEntityComponents<atlas::game::scene::components::cameras::SphericalLookAtCameraComponent>())
            {
                if (addComboItem(s_selectedRenderCamera, getCameraIdentifier(entity, camera)))
                {
                    setActiveCamera(ecs, camera, ActiveCameraType::Rendering);
                }
            }
            for(auto [entity, camera] : ecs.IterateEntityComponents<atlas::game::scene::components::cameras::LookAtCameraComponent>())
            {
                if (addComboItem(s_selectedRenderCamera, getCameraIdentifier(entity, camera)))
                {
                    setActiveCamera(ecs, camera, ActiveCameraType::Rendering);
                }
            }

            ImGui::EndCombo();
        }

        if (ImGui::BeginTable("Spherical Cameras", 2))
        {
            for(auto [entity, camera] : ecs.IterateEntityComponents<atlas::game::scene::components::cameras::SphericalLookAtCameraComponent>())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Look At Centre");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%f, %f, %f", camera.m_SphericalCentre.x(), camera.m_SphericalCentre.y(), camera.m_SphericalCentre.z());

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Look At");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%f°, %f°", camera.m_LookAtYaw.AsDegrees(), camera.m_LookAtPitch.AsDegrees());

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Camera Position");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%f°, %f°", camera.m_CameraYaw.AsDegrees(), camera.m_CameraPitch.AsDegrees());

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Camera Distance");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%f", camera.m_Distance);
            }

            ImGui::EndTable();
        }

        pCameraRenderer->SetDebugRenderingEnabled(s_enableDebug);
    }
}

void cpp_conv::GameSceneDebugUI::Initialise(atlas::scene::EcsManager& ecsManager, atlas::game::scene::systems::cameras::CameraViewProjectionUpdateSystem* pCameraRenderer)
{
    m_pCameraRenderer = pCameraRenderer;

    IMGUI_CHECKVERSION();
    ImGui::StyleColorsDark();
    atlas::app_host::platform::PlatformApplication& platform = atlas::app_host::Application::Get().GetPlatform();
    ImGui_ImplSDL2_InitForSDLRenderer(platform.GetSDLContext().m_Window);
    ImGui_Implbgfx_Init(constants::render_views::c_debugUi);
}

void cpp_conv::GameSceneDebugUI::Update(atlas::scene::EcsManager& ecs)
{
    newFrame();

    constexpr ImGuiWindowFlags flags = 0;
    ImGui::SetNextWindowPos({0, 0});
    if (ImGui::Begin("GameScene Debug UI", nullptr, flags))
    {
        addCameraDebugUi(ecs, m_pCameraRenderer);
    }
    ImGui::End();

    present();
}
