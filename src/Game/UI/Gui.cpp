#include "Gui.h"
#include <imgui.h>
#include <vector>

#include "Profiler.h"
#include "RenderContext.h"
#include "SceneContext.h"

void cpp_conv::ui::drawUI(SceneContext& kSceneContext, const RenderContext& kRenderContext)
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

    static_assert(static_cast<int32_t>(EntityKind::MAX) == sizeof(c_entityNames) / sizeof(const char*), "Table mismatch"
    );

    PROFILE(ImGuiNewFrame, cpp_conv::ui::newFrame());

    ImGui::NewFrame();

    constexpr ImGuiWindowFlags flags = 0;

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

    ImGui::SetNextWindowPos({0, 0});
    if (ImGui::Begin("Hello, world!", nullptr, flags))
    {
        ImGui::Text("%.3f ms/frame (%.1f FPS)", static_cast<double>(1000.0f / ImGui::GetIO().Framerate),
                    static_cast<double>(ImGui::GetIO().Framerate));
        ImGui::Text(std::format("Entities in world: {} Conveyors, {} Others",
                                kSceneContext.m_rMap.GetConveyors().size(),
                                kSceneContext.m_rMap.GetOtherEntities().size()).c_str());
        //ImGui::Text(std::format("Items in World: {}", uiItemCount).c_str());
        ImGui::Text(std::format("Drawn items: {}", kRenderContext.m_uiDrawnItems).c_str());

        ImGui::Text(std::format("Position: {}, {}", kRenderContext.m_CameraPosition.GetX(),
                                kRenderContext.m_CameraPosition.GetY()).c_str());
        ImGui::Text(std::format("Current Floor: {}", kRenderContext.m_CameraPosition.GetZ()).c_str());

        ImGui::Text("Item Type (NUM 1-9)");

        for (int32_t i = 0; i < static_cast<int32_t>(EntityKind::MAX); ++i)
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
        }

        if (kSceneContext.m_uiContext.m_selected == static_cast<int>(EntityKind::Stairs))
        {
            ImGui::Text("Stairs go up? (T)");
            ImGui::Text(kSceneContext.m_uiContext.m_bModifier ? "   true" : "   false");
        }
    }
    ImGui::End();


    ImGui::Render();

    PROFILE(ImGuiPresent, cpp_conv::ui::present());
}
