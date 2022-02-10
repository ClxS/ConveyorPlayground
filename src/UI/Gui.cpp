#include "Gui.h"
#include <vector>
#include <queue>
#include <imgui.h>

#include "RenderContext.h"
#include "Profiler.h"
#include "SceneContext.h"
#include "Conveyor.h"

void cpp_conv::ui::drawUI(SceneContext& kSceneContext, RenderContext& kRenderContext)
{
    using namespace cpp_conv;
    constexpr const char* c_entityNames[] =
    {
        "Conveyor",
        "Inserter",
        "Junction",
        "Producer",
        "Stairs",
        "Storage",
        "Underground",
    };

    static_assert((int32_t)EntityKind::MAX == sizeof(c_entityNames) / sizeof(const char*), "Table mismatch");

    PROFILE(ImGuiNewFrame, cpp_conv::ui::newFrame());

    ImGui::NewFrame();

    ImGuiWindowFlags flags = 0;

    /*uint64_t uiItemCount = 0;
    for (auto& sequence : kSceneContext.m_sequences)
    {
        uiItemCount += sequence->CountItemsOnBelt();
    }

    for (cpp_conv::Conveyor* pConveyor : kSceneContext.m_rMap.GetConveyors())
    {
        if (!pConveyor->IsPartOfASequence())
        {
            uiItemCount += pConveyor->CountItemsOnBelt();
        }
    }*/ 

    ImGui::SetNextWindowPos({ 0, 0 });
    if (ImGui::Begin("Hello, world!", 0, flags))
    {
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text(std::format("Entites in world: {} Conveyors, {} Others", kSceneContext.m_rMap.GetConveyors().size(), kSceneContext.m_rMap.GetOtherEntities().size()).c_str());
        //ImGui::Text(std::format("Items in World: {}", uiItemCount).c_str());
        ImGui::Text(std::format("Drawn items: {}", kRenderContext.m_uiDrawnItems).c_str());

        ImGui::Text(std::format("Position: {}, {}", kSceneContext.m_player.GetX(), kSceneContext.m_player.GetY()).c_str());
        ImGui::Text(std::format("Current Floor: {}", kSceneContext.m_player.GetZ()).c_str());

        ImGui::Text("Item Type (NUM 1-9)");

        for (int32_t i = 0; i < (int32_t)EntityKind::MAX; ++i)
        {
            ImGui::Text(
                std::format(
                    "{} {}. {}",
                    kSceneContext.m_uiContext.m_selected == i ? ">" : " ",
                    i + 1,
                    c_entityNames[i]).c_str());
        }

        ImGui::Text("Rotation (R to rotate)");
        switch (kSceneContext.m_uiContext.m_rotation)
        {
            case Direction::Up:
                ImGui::Text("   v");
                break;
            case Direction::Down:
                ImGui::Text("   ^");
                break;
            case Direction::Left:
                ImGui::Text("   <");
                break;
            case Direction::Right:
                ImGui::Text("   >");
                break;
            default:
                break;
        }

        if (kSceneContext.m_uiContext.m_selected == (int)EntityKind::Stairs)
        {
            ImGui::Text("Stairs go up? (T)");
            ImGui::Text(kSceneContext.m_uiContext.m_bModifier ? "   true" : "   false");
        }


        ImGui::End();
    }

    ImGui::Render();

    PROFILE(ImGuiPresent, cpp_conv::ui::present());
}
