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
#include "TileAsset.h"
#include "Profiler.h"
#include "WorldMap.h"
#include "AppHost.h"

using TypeId = size_t;
static std::map<TypeId, std::function<void(cpp_conv::RenderContext&, const cpp_conv::resources::RenderableAsset*, cpp_conv::Transform2D, cpp_conv::Colour, bool)>*> g_typeHandlers;

namespace
{
    std::mutex& getStateMutex()
    {
        static std::mutex s_stateMutex;
        return s_stateMutex;
    }

    std::function<void(cpp_conv::RenderContext&, const cpp_conv::resources::RenderableAsset*, cpp_conv::Transform2D, cpp_conv::Colour, bool)>* getTypeHandler(const std::type_info& type)
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

void cpp_conv::renderer::init(cpp_conv::RenderContext& kContext, cpp_conv::renderer::SwapChain& rSwapChain)
{
    ScreenBufferInitArgs kArgs = { };
    rSwapChain.Initialize(kContext, kArgs);
    kContext.m_surface = &rSwapChain.GetWriteSurface();
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
            (float)kSceneContext.m_player.GetX() * cpp_conv::renderer::c_gridScale,
            (float)kSceneContext.m_player.GetY() * cpp_conv::renderer::c_gridScale,
            Rotation::DegZero
        },
        { 0xFFFFFFFF });
}

void cpp_conv::renderer::render(const SceneContext& kSceneContext, RenderContext& kContext)
{
    drawBackground(kSceneContext, kContext);

    int width, height;
    std::tie(width, height) = cpp_conv::apphost::getAppDimensions();

    Vector2F startCellPosition = kContext.m_CameraPosition / kContext.m_fZoom * -1.0f / 64;
    Vector2F endCellPosition = (kContext.m_CameraPosition / kContext.m_fZoom * -1.0f + Vector2F((float)width, (float)height) / kContext.m_fZoom) / 64;
    WorldMap::CellCoordinate startCoord = WorldMap::ToCellSpace({ (int)startCellPosition.GetX(), (int)startCellPosition.GetY(), 0 });
    WorldMap::CellCoordinate endCoord = WorldMap::ToCellSpace({ (int)std::ceil(endCellPosition.GetX()), (int)std::ceil(endCellPosition.GetY()), 0 });

     uint32_t uiPassCount = 0;
     WorldMap::CellCoordinate currentCoord = startCoord;
     for (currentCoord.m_CellY = startCoord.m_CellY; currentCoord.m_CellY <= endCoord.m_CellY; currentCoord.m_CellY++)
     {
         for (currentCoord.m_CellX = startCoord.m_CellX; currentCoord.m_CellX <= endCoord.m_CellX; currentCoord.m_CellX++)
         {
             WorldMap::Cell* pCell = kContext.m_rMap.GetCell(currentCoord);
             if (!pCell || !pCell->m_bHasFloors)
             {
                 continue;
             }

             for (kContext.m_iCurrentLayer = 0; kContext.m_iCurrentLayer <= kSceneContext.m_player.GetZ(); kContext.m_iCurrentLayer++)
             {
                 if (!pCell->HasFloor(kContext.m_iCurrentLayer))
                 {
                     break;
                 }

                 kContext.m_LayerColour = { 0xFFFFFFFF };
                 kContext.m_LayerColour.m_argb.m_a = (uint8_t)(std::pow(0.5F, kSceneContext.m_player.GetZ() - kContext.m_iCurrentLayer) * 0xFF);

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
                             currentCoord.m_depth = kContext.m_iCurrentLayer;

                             const Entity* pEntity = pCell->GetEntity(currentCoord);
                             if (!pEntity)
                             {
                                 continue;
                             }

                             if (pEntity->GetDrawPassCount() < (uiPass + 1) ||
                                 (pEntity->m_position.GetZ() + pEntity->m_size.GetZ() - 1) < kContext.m_iCurrentLayer || pEntity->m_position.GetZ() > kContext.m_iCurrentLayer)
                             {
                                 continue;
                             }

                             bHadPassAction = true;
                             pEntity->Draw(kContext);
                         }
                     }

                     uiPass++;
                 } while (bHadPassAction);
             }
         }
     }

    //drawPlayer(kSceneContext, kContext);
}

void cpp_conv::renderer::renderAsset(const std::type_info& type, RenderContext& kContext, resources::RenderableAsset* pRenderable, Transform2D transform, Colour kColourOverride, bool bTrack)
{
    PROFILE_FUNC();
    std::function<void(cpp_conv::RenderContext&, const resources::RenderableAsset*, cpp_conv::Transform2D, cpp_conv::Colour, bool)>* pHandler = nullptr;
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

void cpp_conv::renderer::registerTypeHandler(const std::type_info& type, std::function<void(cpp_conv::RenderContext&, const resources::RenderableAsset*, cpp_conv::Transform2D, cpp_conv::Colour, bool)> fHandler)
{
    std::lock_guard<std::mutex> lock(getStateMutex());
    g_typeHandlers[type.hash_code()] = new std::function<void(cpp_conv::RenderContext&, const resources::RenderableAsset*, cpp_conv::Transform2D, cpp_conv::Colour, bool)>(fHandler);
}

void cpp_conv::renderer::drawBackground(const SceneContext& kSceneContext, RenderContext& kContext)
{
    PROFILE_FUNC();
    auto pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(
        cpp_conv::resources::registry::visual::BackgroundRepeating);
    if (!pTile)
    {
        return;
    }

    cpp_conv::renderer::renderAsset(
        kContext,
        pTile.get(),
        {
            (float)kSceneContext.m_player.GetX() * cpp_conv::renderer::c_gridScale,
            (float)kSceneContext.m_player.GetY() * cpp_conv::renderer::c_gridScale,
            Rotation::DegZero,
            true
        },
        { 0xFFFFFFFF });
}
