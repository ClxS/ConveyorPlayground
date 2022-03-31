#include "Stairs.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "RenderContext.h"
#include "Renderer.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <random>
#include "ResourceManager.h"
#include "TileAsset.h"

cpp_conv::Stairs::Stairs(Vector3 position, Vector3 size, Direction direction, bool bIsUp)
    : Entity(position, size, EntityKind::Stairs)
      , m_pItem(ItemIds::None)
      , m_uiTick(0)
      , m_direction(direction)
      , m_bIsUp(bIsUp)
{
}

void cpp_conv::Stairs::Tick(const SceneContext& kContext)
{
    if (!m_pItem.IsValid())
    {
        return;
    }

    Vector3 targetPosition = grid::getForwardPosition(*this);
    if (m_bIsUp)
    {
        ++targetPosition.GetZ();
    }

    Entity* pTarget = kContext.m_rMap.GetEntity(targetPosition);
    if (!pTarget || !pTarget->SupportsInsertion())
    {
        return;
    }

    if (pTarget->TryInsert(kContext, *this, InsertInfo(m_pItem)))
    {
        m_pItem = ItemIds::None;
    }
}

void cpp_conv::Stairs::Draw(RenderContext& kRenderContext) const
{
    const auto pTile =
        m_bIsUp
            ? cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
                resources::registry::assets::c_Storage)
            : cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(
                resources::registry::assets::c_Storage);

    if (!pTile)
    {
        return;
    }

    renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            static_cast<float>(m_position.GetX()) * renderer::c_gridScale,
            static_cast<float>(m_position.GetY()) * renderer::c_gridScale,
        },
        {0xFFFF00FF});
}

bool cpp_conv::Stairs::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const InsertInfo insertInfo)
{
    if (m_bIsUp && pSourceEntity.m_position.GetZ() != m_position.GetZ())
    {
        return false;
    }

    if (!m_bIsUp && pSourceEntity.m_position.GetZ() != (m_position.GetZ() + 1))
    {
        if (pSourceEntity.m_eEntityKind == EntityKind::Stairs && static_cast<const Stairs*>(&pSourceEntity)->m_bIsUp)
        {
            // Special case, we allow up stairs to make this transition
        }
        else
        {
            return false;
        }
    }

    if (!m_pItem.IsEmpty())
    {
        return false;
    }

    m_pItem = insertInfo.GetItem();
    return true;
}
