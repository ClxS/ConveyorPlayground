#include <cstdint>

#include "Gui.h"
#include "RenderContext.h"

#include <imgui.h>
#include <SDL.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_sdlrenderer.h>

#include "AtlasAppHost/Application.h"

void cpp_conv::ui::initializeGuiSystem()
{
    atlas::app_host::platform::PlatformApplication& platform = atlas::app_host::Application::Get().GetPlatform();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(platform.GetSDLContext().m_Window);
    ImGui_ImplSDLRenderer_Init(platform.GetSDLContext().m_Renderer);
}

void cpp_conv::ui::newFrame()
{
    atlas::app_host::platform::PlatformApplication& platform = atlas::app_host::Application::Get().GetPlatform();
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(platform.GetSDLContext().m_Window);
}

void cpp_conv::ui::present()
{
    ImDrawData* pDrawData = ImGui::GetDrawData();
    if (!pDrawData)
    {
        return;
    }

    atlas::app_host::platform::PlatformApplication& platform = atlas::app_host::Application::Get().GetPlatform();
    ImGui_ImplSDLRenderer_RenderDrawData(pDrawData);
    SDL_RenderPresent(platform.GetSDLContext().m_Renderer);
}

void cpp_conv::ui::shutdown()
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
