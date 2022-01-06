#include "Renderer.h"
#include "RenderContext.h"
#include "SceneContext.h"
#include "SwapChain.h"
#include "Entity.h"

#include <map>
#include <mutex>
#include "TileRenderHandler.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "TextTileAsset.h"

using TypeId = size_t;
static std::map<TypeId, std::function<void(cpp_conv::RenderContext&, const cpp_conv::resources::RenderableAsset*, cpp_conv::Transform2D)>*> g_typeHandlers;

namespace
{
	std::mutex& getStateMutex()
	{
		static std::mutex s_stateMutex;
		return s_stateMutex;
	}

    std::function<void(cpp_conv::RenderContext&, const cpp_conv::resources::RenderableAsset*, cpp_conv::Transform2D)>* getTypeHandler(const std::type_info& type)
    {
	    // No need to lock here, this is only called in the context of an existing lock
	    auto iter = g_typeHandlers.find(type.hash_code());
	    if (iter == g_typeHandlers.end())
	    {
		    return nullptr;
	    }

	    return iter->second;
    }
}

void cpp_conv::renderer::init(cpp_conv::renderer::SwapChain& rSwapChain)
{
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof cfi;
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 18;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_HEAVY;

    wcscpy_s<32>(cfi.FaceName, L"Lucida Console");
    ScreenBufferInitArgs kArgs = { cfi };

    rSwapChain.Initialize(kArgs);

    cpp_conv::renderer::registerTileRenderHandler();
}

void drawPlayer(const cpp_conv::SceneContext& kSceneContext, cpp_conv::RenderContext& kRenderContext)
{
	auto pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Player);
    if (!pTile)
    {
        return;
    }

    cpp_conv::renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            kSceneContext.m_player.m_x * cpp_conv::renderer::c_gridScale,
            kSceneContext.m_player.m_y * cpp_conv::renderer::c_gridScale,
            cpp_conv::Transform2D::Rotation::DegZero
        });
}

void cpp_conv::renderer::render(const SceneContext& kSceneContext, RenderContext& kContext)
{
    for (int y = 0; y < kContext.m_grid.size(); y++)
    {
        for (int x = 0; x < kContext.m_grid[y].size(); x++)
        {
            Entity* cell = kContext.m_grid[y][x];
            if (cell != nullptr)
            {
                cell->Draw(kContext);
            }
        }
    }

    drawPlayer(kSceneContext, kContext);
}

void cpp_conv::renderer::renderAsset(const std::type_info& type, RenderContext& kContext, resources::RenderableAsset* pRenderable, Transform2D transform)
{
    std::function<void(cpp_conv::RenderContext&, const resources::RenderableAsset*, cpp_conv::Transform2D)>* pHandler = nullptr;
    {
		std::lock_guard<std::mutex> lock(getStateMutex());
		pHandler = getTypeHandler(type);
    }

    if (!pHandler)
    {
        return;
    }

    (*pHandler)(kContext, pRenderable, std::move(transform));
}

void cpp_conv::renderer::registerTypeHandler(const std::type_info& type, std::function<void(cpp_conv::RenderContext&, const resources::RenderableAsset*, const cpp_conv::Transform2D&)> fHandler)
{
	std::lock_guard<std::mutex> lock(getStateMutex());
	g_typeHandlers[type.hash_code()] = new std::function<void(cpp_conv::RenderContext&, const resources::RenderableAsset*, cpp_conv::Transform2D)>(fHandler);
}