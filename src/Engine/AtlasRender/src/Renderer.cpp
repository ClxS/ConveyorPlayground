#include "AtlasRenderPCH.h"
#include "Renderer.h"

#include <algorithm>

#include "bgfx/platform.h"

#include <functional>

namespace
{
    struct Task
    {
        std::function<void()> m_Callback;
        bool m_IsOneOff;
    };

    static std::vector<Task> m_RenderMethods;
}

void atlas::render::init(const RendererInitArgs& args)
{
    bgfx::PlatformData pd{};
    pd.nwh = args.m_WindowHandle;
#if BX_PLATFORM_LINUX
#error implement ndt
#endif

    bgfx::Init bgfxInitArgs;
    bgfxInitArgs.type = bgfx::RendererType::Count; // auto choose renderer
    bgfxInitArgs.resolution.width = args.m_Width;
    bgfxInitArgs.resolution.height = args.m_Height;
    bgfxInitArgs.resolution.reset = BGFX_RESET_VSYNC;
    bgfxInitArgs.platformData = pd;
    bgfx::init(bgfxInitArgs);

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, args.m_Width, args.m_Height);
}

atlas::render::RenderTaskHandle atlas::render::addToFrameGraph(
    std::string_view name,
    std::function<void()> initiailize,
    std::function<void()> callback,
    std::vector<RenderTaskHandle> dependentTasks)
{
    m_RenderMethods.emplace_back(initiailize, true);
    m_RenderMethods.emplace_back(callback, false);
    return {-1};
}

atlas::render::RenderTaskHandle atlas::render::addToFrameGraph(
    std::string_view name,
    std::function<void()> callback,
    std::vector<RenderTaskHandle> dependentTasks)
{
    m_RenderMethods.emplace_back(callback, false);
    return {-1};
}

atlas::render::RenderTaskHandle atlas::render::addToFrameGraph_oneOff(std::string_view name, std::function<void()> callback,
    std::vector<RenderTaskHandle> dependentTasks)
{
    m_RenderMethods.emplace_back(callback, true);
    return {-1};
}

void atlas::render::sync()
{
    for(auto& task : m_RenderMethods)
    {
        if (task.m_Callback)
        {
            task.m_Callback();
        }
    }

    bgfx::frame();
    std::erase_if(m_RenderMethods, [](const Task& t) { return t.m_IsOneOff; });
}
