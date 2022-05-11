#include "GameSceneDebugUI.h"

#include "Constants.h"
#include "imgui.h"
#include "AtlasAppHost/Application.h"
#include "AtlasAppHost/PlatformApplication.h"
#include "backends/imgui_impl_sdl.h"
#include "ImguiBgfx/ImguiBgfxImpl.h"

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
}

void cpp_conv::GameSceneDebugUI::Initialise(atlas::scene::EcsManager& ecsManager)
{
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
        ImGui::Text("Hello World");
    }
    ImGui::End();

    present();
}
