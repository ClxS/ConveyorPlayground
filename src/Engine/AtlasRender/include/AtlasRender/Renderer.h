#pragma once
#include <cstdint>
#include <functional>
#include <string>

namespace atlas::render
{
    struct RendererInitArgs
    {
        void* m_WindowHandle;

        int16_t m_Width;
        int16_t m_Height;
    };

    struct RenderTaskHandle
    {
        int32_t m_Value;
    };

    void init(const RendererInitArgs& args);

    RenderTaskHandle addToFrameGraph(std::string name, std::function<void()> initiailize, std::function<void()> callback, std::vector<RenderTaskHandle> dependentTasks = {});
    RenderTaskHandle addToFrameGraph(std::string name, std::function<void()> callback, std::vector<RenderTaskHandle> dependentTasks = {});

    RenderTaskHandle addToFrameGraph_oneOff(std::string name, std::function<void()> callback, std::vector<RenderTaskHandle> dependentTasks = {});

    void sync();
}
