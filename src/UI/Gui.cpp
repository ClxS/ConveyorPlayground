#include "Gui.h"
#include <vector>
#include <queue>
#include <imgui.h>

#include "RenderContext.h"
#include "Profiler.h"

void cpp_conv::ui::drawUI(SceneContext& kContext, RenderContext& kRenderContext)
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
    ImGui::Render();

    PROFILE(ImGuiPresent, cpp_conv::ui::present());
}
