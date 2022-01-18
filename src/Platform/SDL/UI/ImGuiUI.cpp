#include <cstdint>

#include "Gui.h"
#include "RenderContext.h"
#include "AppHost.h"
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
    ImGui_ImplSDL2_InitForSDLRenderer(cpp_conv::apphost::App.window);
    ImGui_ImplSDLRenderer_Init(cpp_conv::apphost::App.renderer);
}

void cpp_conv::ui::newFrame()
{
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(cpp_conv::apphost::App.window);
}

void cpp_conv::ui::present()
{
    ImDrawData* pDrawData = ImGui::GetDrawData();
    if (!pDrawData)
    {
        return;
    }

    ImGui_ImplSDLRenderer_RenderDrawData(pDrawData);
    SDL_RenderPresent(cpp_conv::apphost::App.renderer);
}

void cpp_conv::ui::shutdown()
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
