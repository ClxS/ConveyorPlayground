#include "Factory.h"
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
#include "DataId.h"
#include "AssetPtr.h"
#include "FactoryRegistry.h"
#include "FactoryDefinition.h"

cpp_conv::Factory::Factory(int x, int y, Direction direction, FactoryId factoryId)
    : Entity(x, y, EntityKind::Producer)
    , m_pFactoryId(factoryId)
    , m_pItem(ItemIds::None)
    , m_direction(direction)
    , m_uiTick(0)
    , m_productionRate(0)
    , m_bProductionReady(false)
{
    const cpp_conv::resources::AssetPtr<cpp_conv::FactoryDefinition> pFactory = cpp_conv::resources::getFactoryDefinition(factoryId);
    if (!pFactory)
    {
        return;
    }

    m_pItem = pFactory->GetProducedItem();
    m_productionRate = pFactory->GetProductionRate();
}

bool cpp_conv::Factory::IsReadyToProduce() const
{
    return m_bProductionReady;
}

cpp_conv::ItemId cpp_conv::Factory::ProduceItem()
{
    if (!m_bProductionReady)
    {
        return cpp_conv::ItemIds::None;
    }

    m_bProductionReady = false;
    return m_pItem;
}

void cpp_conv::Factory::Tick(const SceneContext& kContext)
{
    if (this->m_productionRate == 0)
    {
        return;
    }

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

    cpp_conv::Entity* pEntity = cpp_conv::grid::SafeGetEntity(kContext.m_grid, cpp_conv::grid::GetForwardPosition(*this));
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

void cpp_conv::Factory::Draw(RenderContext& kRenderContext) const
{
    auto pFactory = cpp_conv::resources::getFactoryDefinition(m_pFactoryId);
    if (!pFactory)
    {
        return;
    }

    auto pTile = pFactory->GetTile();
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
