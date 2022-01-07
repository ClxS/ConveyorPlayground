#pragma once
#include "Transform2D.h"
#include "Colour.h"
#include "RenderableAsset.h"
#include <functional>

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;
}

namespace cpp_conv::renderer
{
    constexpr int c_gridScale = 3;

    class SwapChain;

    void init(SwapChain& rSwapChain);
    void render(const SceneContext& kSceneContext, RenderContext& kContext);

    void renderAsset(const std::type_info& type, RenderContext& kContext, resources::RenderableAsset* pRenderable, Transform2D transform, Colour kColourOverride = {});

    void registerTypeHandler(const std::type_info& type, std::function<void (RenderContext&, const resources::RenderableAsset*, Transform2D, cpp_conv::Colour)> fHandler);

    template<typename TType>
    void renderAsset(RenderContext& kContext, TType* pRenderable, Transform2D transform, Colour kColourOverride = {})
    {
        renderAsset(typeid(TType), kContext, pRenderable, std::move(transform), kColourOverride);
    }

    template<typename TType>
    void registerTypeHandler(std::function<void(RenderContext&, const resources::RenderableAsset*, Transform2D, cpp_conv::Colour)> fHandler)
    {
        registerTypeHandler(typeid(TType), fHandler);
    }
};