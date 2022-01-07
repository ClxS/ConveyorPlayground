#include "Producer.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "Renderer.h"
#include "RenderContext.h"

#include <array>
#include <random>
#include "ConsoleWriteUtility.h"
#include "ResourceManager.h"
#include "ResourceRegistry.h"
#include "TileAsset.h"
#include "ItemId.h"

cpp_conv::Producer::Producer(int x, int y, Direction direction, ItemId pItem, uint64_t productionRate)
    : Entity(x, y, EntityKind::Producer)
    , m_pItem(pItem)
    , m_direction(direction)
    , m_uiTick(0)
    , m_productionRate(productionRate)
    , m_bProductionReady(false)
{
}

bool cpp_conv::Producer::IsReadyToProduce() const
{
    return m_bProductionReady;
}

cpp_conv::ItemId cpp_conv::Producer::ProduceItem()
{
    if (!m_bProductionReady)
    {
        return cpp_conv::Item::None;
    }

    m_bProductionReady = false;
    return m_pItem;
}

void cpp_conv::Producer::Tick(const SceneContext& kContext)
{
    if (!IsReadyToProduce())
    {
        m_uiTick++;
        if ((m_uiTick % m_productionRate) == 0)
        {
            m_bProductionReady = true;
        }

        if (!IsReadyToProduce())
        {
            return;
        }
    }

    ItemId pItem = ProduceItem();
    if (pItem.IsEmpty())
    {
        return;
    }

    cpp_conv::Entity* pEntity = cpp_conv::grid::SafeGetEntity(kContext.m_grid, cpp_conv::grid::GetForwardPosition(*this, GetDirection()));
    if (!pEntity || !pEntity->SupportsInsertion())
    {
        return;
    }

    for (int iExitChannel = 0; iExitChannel < cpp_conv::c_conveyorChannels; ++iExitChannel)
    {
        if (pEntity->TryInsert(kContext, *this, m_pItem, (iExitChannel + m_uiTick) % cpp_conv::c_conveyorChannels))
        {
            m_bProductionReady = false;
            break;
        }
    }
}

void cpp_conv::Producer::Draw(RenderContext& kRenderContext) const
{
    auto pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Tunnel);
    if (!pTile)
    {
        return;
    }

    cpp_conv::renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            m_position.m_x * cpp_conv::renderer::c_gridScale,
            m_position.m_y * cpp_conv::renderer::c_gridScale,
            cpp_conv::rotationFromDirection(m_direction)
        },
        { 0xFFFFFF00 });
}
