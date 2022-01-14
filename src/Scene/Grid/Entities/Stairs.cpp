#include "Stairs.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "Renderer.h"
#include "RenderContext.h"

#include <algorithm> 
#include <array>
#include <random>
#include <chrono>
#include "TileAsset.h"
#include "ResourceManager.h"

cpp_conv::Stairs::Stairs(Vector3 position, Vector3 size, Direction direction, bool bIsUp)
    : Entity(position, size, EntityKind::Stairs)
    , m_pItem(cpp_conv::ItemIds::None)
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

    Vector3 targetPosition = cpp_conv::grid::GetForwardPosition(*this);
    if (m_bIsUp)
    {
        ++targetPosition.m_depth;
    }

    Entity* pTarget = kContext.m_rMap.GetEntity(targetPosition);
    if (!pTarget || !pTarget->SupportsInsertion())
    {
        return;
    }

    if (pTarget->TryInsert(kContext, *this, m_pItem, 0))
    {
        m_pItem = cpp_conv::ItemIds::None;
    }
}

void cpp_conv::Stairs::Draw(RenderContext& kRenderContext) const
{
    auto pTile =
        m_bIsUp
            ? cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::StairsUp)
            : cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::StairsDown);

    if (!pTile)
    {
        return;
    }

    cpp_conv::renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            (float)m_position.m_x * cpp_conv::renderer::c_gridScale,
            (float)m_position.m_y * cpp_conv::renderer::c_gridScale,
            cpp_conv::Transform2D::Rotation::DegZero
        },
        { 0xFFFF00FF });
}

bool cpp_conv::Stairs::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel)
{
    if (m_bIsUp && pSourceEntity.m_position.m_depth != m_position.m_depth)
    {
        return false;
    }

    if (!m_bIsUp && pSourceEntity.m_position.m_depth != (m_position.m_depth + 1))
    {
        if (pSourceEntity.m_eEntityKind == EntityKind::Stairs && reinterpret_cast<const Stairs*>(&pSourceEntity)->m_bIsUp)
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

    m_pItem = pItem;
    return true;
}
