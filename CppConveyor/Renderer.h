#pragma once

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

	void setPixel(RenderContext& kContext, wchar_t value, int x, int y, int colour, bool allowBackFill = false);
}