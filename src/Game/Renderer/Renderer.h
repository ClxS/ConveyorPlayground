#pragma once
#include <functional>
#include "Colour.h"
#include "Transform2D.h"
//
// namespace cpp_conv
// {
//     struct SceneContext;
//     struct RenderContext;
// }
//
// namespace cpp_conv::renderer
// {
//     constexpr int c_gridScale = 4;
//
//     class SwapChain;
//
//     void init(RenderContext& kContext, SwapChain& rSwapChain);
//     void render(const SceneContext& kSceneContext, RenderContext& kContext);
//
//     void renderAsset(const std::type_info& type, RenderContext& kContext, resources::RenderableAsset* pRenderable,
//                      Transform2D transform, Colour kColourOverride = {}, bool bTrack = false);
//
//     void registerTypeHandler(const std::type_info& type, std::function<void (
//                                  RenderContext&, const resources::RenderableAsset*, Transform2D,
//                                  Colour, bool)> fHandler);
//
//     template <typename TType>
//     void renderAsset(RenderContext& kContext, TType* pRenderable, Transform2D transform, Colour kColourOverride = {},
//                      bool bTrack = false)
//     {
//         renderAsset(typeid(TType), kContext, pRenderable, std::move(transform), kColourOverride);
//     }
//
//     template <typename TType>
//     void registerTypeHandler(
//         std::function<void(RenderContext&, const resources::RenderableAsset*, Transform2D, Colour, bool)> fHandler)
//     {
//         registerTypeHandler(typeid(TType), fHandler);
//     }
//
//     void drawBackground(const SceneContext& kSceneContext, RenderContext& kContext);
// };
