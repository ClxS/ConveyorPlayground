#pragma once
#include "Transform2D.h"
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

	void renderAsset(const std::type_info& type, RenderContext& kContext, resources::RenderableAsset* pRenderable, Transform2D transform);

	void registerTypeHandler(const std::type_info& type, std::function<void (RenderContext&, const resources::RenderableAsset*, const Transform2D&)> fHandler);

	template<typename TType>
	void renderAsset(RenderContext& kContext, TType* pRenderable, Transform2D transform)
	{
		renderAsset(typeid(TType), kContext, pRenderable, std::move(transform));
	}

	template<typename TType>
	void registerTypeHandler(std::function<void(RenderContext&, const resources::RenderableAsset*, const Transform2D&)> fHandler)
	{
		registerTypeHandler(typeid(TType), fHandler);
	}
};