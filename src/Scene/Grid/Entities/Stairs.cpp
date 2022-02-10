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

    Vector3 targetPosition = cpp_conv::grid::getForwardPosition(*this);
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
            (float)m_position.GetX() * cpp_conv::renderer::c_gridScale,
            (float)m_position.GetY() * cpp_conv::renderer::c_gridScale,
            Rotation::DegZero
        },
        { 0xFFFF00FF });
}

bool cpp_conv::Stairs::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const InsertInfo insertInfo)
{
    if (m_bIsUp && pSourceEntity.m_position.GetZ() != m_position.GetZ())
    {
        return false;
    }

    if (!m_bIsUp && pSourceEntity.m_position.GetZ() != (m_position.GetZ() + 1))
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

    m_pItem = insertInfo.GetItem();
    return true;
}
