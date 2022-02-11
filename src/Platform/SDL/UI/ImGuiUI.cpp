#include <cstdint>

#include "AppHost.h"
#include "Gui.h"
#include "RenderContext.h"
#include "SDLAppHost.h"

#include <imgui.h>
#include <SDL.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_sdlrenderer.h>

void cpp_conv::ui::initializeGuiSystem()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(apphost::app.m_Window);
    ImGui_ImplSDLRenderer_Init(apphost::app.m_Renderer);
}

void cpp_conv::ui::newFrame()
{
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(apphost::app.m_Window);
}

void cpp_conv::ui::present()
{
    ImDrawData* pDrawData = ImGui::GetDrawData();
    if (!pDrawData)
    {
        return;
    }

    ImGui_ImplSDLRenderer_RenderDrawData(pDrawData);
    SDL_RenderPresent(apphost::app.m_Renderer);
}

void cpp_conv::ui::shutdown()
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
