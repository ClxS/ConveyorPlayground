#pragma once
#include <cstdint>
#include <functional>
#include <string>

#include <Eigen/Core>

#include "AtlasCore/MathsHelpers.h"
#include "AtlasResource/AssetPtr.h"
#include "bgfx/bgfx.h"

namespace atlas::render
{
    class ShaderProgram;
    class ModelAsset;

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

    struct DrawInstance
    {
        Eigen::Vector3f m_Position;
        maths_helpers::Angle m_Rotation;
    };

    void init(const RendererInitArgs& args);

    RenderTaskHandle addToFrameGraph(std::string_view name, std::function<void()> initiailize, std::function<void()> callback, std::vector<RenderTaskHandle> dependentTasks = {});
    RenderTaskHandle addToFrameGraph(std::string_view name, std::function<void()> callback, std::vector<RenderTaskHandle> dependentTasks = {});
    RenderTaskHandle addToFrameGraph_oneOff(std::string_view name, std::function<void()> callback, std::vector<RenderTaskHandle> dependentTasks = {});

    void setShadowMap(uint8_t shadowIndex, bgfx::TextureHandle texture);

    void draw(
        bgfx::ViewId viewId,
        const resource::AssetPtr<ModelAsset>& model,
        const resource::AssetPtr<ShaderProgram>& program,
        const Eigen::Matrix4f& transform,
        uint8_t flags = BGFX_DISCARD_ALL);

    void drawInstanced(
        bgfx::ViewId viewId,
        const resource::AssetPtr<ModelAsset>& model,
        const resource::AssetPtr<ShaderProgram>& program,
        const std::vector<Eigen::Matrix4f>& transforms,
        uint8_t flags = BGFX_DISCARD_ALL);

    void sync();
}
