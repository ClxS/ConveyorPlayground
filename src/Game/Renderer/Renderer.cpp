#include "Renderer.h"
#include "Entity.h"
#include "RenderContext.h"
#include "SwapChain.h"

#include <map>
#include <mutex>
#include "Profiler.h"
#include "ResourceManager.h"
#include "TileAsset.h"
#include "TileRenderHandler.h"
#include "AtlasAppHost/Application.h"

using TypeId = size_t;
static std::map<TypeId, std::function<void(cpp_conv::RenderContext&, const cpp_conv::resources::RenderableAsset*,
                                           cpp_conv::Transform2D, cpp_conv::Colour, bool)>*> g_typeHandlers;

namespace
{
    std::mutex& getStateMutex()
    {
        static std::mutex s_stateMutex;
        return s_stateMutex;
    }

    std::function<void(cpp_conv::RenderContext&, const cpp_conv::resources::RenderableAsset*, cpp_conv::Transform2D,
                       cpp_conv::Colour, bool)>* getTypeHandler(const std::type_info& type)
    {
        // No need to lock here, this is only called in the context of an existing lock
        const auto iter = g_typeHandlers.find(type.hash_code());
        if (iter == g_typeHandlers.end())
        {
            return nullptr;
        }

        return iter->second;
    }
}

void cpp_conv::renderer::init(RenderContext& kContext, SwapChain& rSwapChain)
{
    ScreenBufferInitArgs kArgs = {};
    rSwapChain.Initialize(kContext, kArgs);
    kContext.m_surface = &rSwapChain.GetWriteSurface();
}


void cpp_conv::renderer::render(const SceneContext& kSceneContext, RenderContext& kContext)
{
    drawBackground(kSceneContext, kContext);
}

void cpp_conv::renderer::renderAsset(const std::type_info& type, RenderContext& kContext,
                                     resources::RenderableAsset* pRenderable, Transform2D transform,
                                     Colour kColourOverride, bool bTrack)
{
    PROFILE_FUNC();
    const std::function<void(RenderContext&, const resources::RenderableAsset*, Transform2D, Colour, bool)>* pHandler =
        nullptr;
    {
        std::lock_guard<std::mutex> lock(getStateMutex());
        pHandler = getTypeHandler(type);
    }

    if (!pHandler)
    {
        return;
    }

    (*pHandler)(kContext, pRenderable, std::move(transform), kColourOverride, bTrack);
}

void cpp_conv::renderer::registerTypeHandler(const std::type_info& type, std::function<void(
                                                 RenderContext&, const resources::RenderableAsset*,
                                                 Transform2D, Colour, bool)> fHandler)
{
    std::lock_guard<std::mutex> lock(getStateMutex());
    g_typeHandlers[type.hash_code()] = new std::function<void(RenderContext&, const resources::RenderableAsset*,
                                                              Transform2D, Colour, bool)>(fHandler);
}

void cpp_conv::renderer::drawBackground(const SceneContext& kSceneContext, RenderContext& kContext)
{
    PROFILE_FUNC();
    const auto pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
        resources::registry::assets::c_background_repeating);
    if (!pTile)
    {
        return;
    }

}
