#include "Renderer.h"
#include "Entity.h"
#include "RenderContext.h"
#include "SceneContext.h"
#include "SwapChain.h"

#include <map>
#include <mutex>
#include "AppHost.h"
#include "Profiler.h"
#include "ResourceManager.h"
#include "TileAsset.h"
#include "TileRenderHandler.h"
#include "WorldMap.h"
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

void drawPlayer(const cpp_conv::SceneContext& kSceneContext, cpp_conv::RenderContext& kRenderContext)
{
    const auto pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(
        cpp_conv::resources::registry::assets::items::c_CopperOre);
    if (!pTile)
    {
        return;
    }

    cpp_conv::renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            (kRenderContext.m_CameraPosition.GetX()),
            (kRenderContext.m_CameraPosition.GetY()),
            Rotation::DegZero
        },
        {0xFFFFFFFF});
}

void cpp_conv::renderer::render(const SceneContext& kSceneContext, RenderContext& kContext)
{
    drawBackground(kSceneContext, kContext);

    auto [width, height] = atlas::app_host::Application::Get().GetAppDimensions();

    const Vector2F startCellPosition = kContext.m_CameraPosition.GetXY() / kContext.m_fZoom * -1.0f / 64;
    const Vector2F endCellPosition = (kContext.m_CameraPosition.GetXY() / kContext.m_fZoom * -1.0f + Vector2F(
        static_cast<float>(width), static_cast<float>(height)) / kContext.m_fZoom) / 64;
    const WorldMap::CellCoordinate startCoord = WorldMap::ToCellSpace({
        static_cast<int>(startCellPosition.GetX()), static_cast<int>(startCellPosition.GetY()), 0
    });
    const WorldMap::CellCoordinate endCoord = WorldMap::ToCellSpace({
        static_cast<int>(std::ceil(endCellPosition.GetX())), static_cast<int>(std::ceil(endCellPosition.GetY())), 0
    });

    uint32_t uiPassCount = 0;
    WorldMap::CellCoordinate currentCoord = startCoord;
    for (currentCoord.m_CellY = startCoord.m_CellY; currentCoord.m_CellY <= endCoord.m_CellY; currentCoord.m_CellY++)
    {
        for (currentCoord.m_CellX = startCoord.m_CellX; currentCoord.m_CellX <= endCoord.m_CellX; currentCoord.m_CellX
             ++)
        {
            WorldMap::Cell* pCell = kContext.m_rMap.GetCell(currentCoord);
            if (!pCell || !pCell->m_bHasFloors)
            {
                continue;
            }

            for (kContext.m_iCurrentLayer = 0; kContext.m_iCurrentLayer <= kContext.m_CameraPosition.GetZ(); kContext.
                 m_iCurrentLayer++)
            {
                if (!pCell->HasFloor(kContext.m_iCurrentLayer))
                {
                    break;
                }

                kContext.m_LayerColour = {0xFFFFFFFF};
                kContext.m_LayerColour.m_argb.m_a = static_cast<uint8_t>(std::pow(
                    0.5F, kContext.m_CameraPosition.GetZ() - kContext.m_iCurrentLayer) * 0xFF);

                bool bHadPassAction = false;
                uint32_t uiPass = 0;
                do
                {
                    kContext.m_uiCurrentDrawPass = uiPass;
                    bHadPassAction = false;
                    for (uint32_t y = 0; y < WorldMap::c_uiCellSize; y++)
                    {
                        for (uint32_t x = 0; x < WorldMap::c_uiCellSize; x++)
                        {
                            currentCoord.m_CellSlotX = x;
                            currentCoord.m_CellSlotY = y;
                            currentCoord.m_Depth = kContext.m_iCurrentLayer;

                            const Entity* pEntity = pCell->GetEntity(currentCoord);
                            if (!pEntity)
                            {
                                continue;
                            }

                            if (pEntity->GetDrawPassCount() < (uiPass + 1) ||
                                (pEntity->m_position.GetZ() + pEntity->m_size.GetZ() - 1) < kContext.m_iCurrentLayer ||
                                pEntity->m_position.GetZ() > kContext.m_iCurrentLayer)
                            {
                                continue;
                            }

                            bHadPassAction = true;
                            pEntity->Draw(kContext);
                        }
                    }

                    uiPass++;
                }
                while (bHadPassAction);
            }
        }
    }

    //drawPlayer(kSceneContext, kContext);
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

    renderAsset(
        kContext,
        pTile.get(),
        {
            (kContext.m_CameraPosition.GetX()),
            (kContext.m_CameraPosition.GetY()),
            Rotation::DegZero,
            true
        },
        {0xFFFFFFFF});
}
