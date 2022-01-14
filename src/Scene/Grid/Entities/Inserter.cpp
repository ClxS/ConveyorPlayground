#include "Inserter.h"
#include "Conveyor.h"
#include "Sequence.h"
#include "SceneContext.h"

#include "Renderer.h"
#include "RenderContext.h"

#include <algorithm> 
#include <array>
#include <random>
#include <chrono>
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "TileAsset.h"
#include "InserterDefinition.h"
#include "InserterRegistry.h"
#include "EntityGrid.h"
#include "ItemRegistry.h"
#include "ItemDefinition.h"

cpp_conv::Inserter::Inserter(Vector3 position, Vector3 size, Direction direction, InserterId inserterId)
    : Entity(position, size, EntityKind::Inserter)
    , m_inserterId(inserterId)
    , m_direction(direction)
    , m_transitTime(9999999)
    , m_cooldownTime(9999999)
    , m_supportsStacks(false)    
    , m_currentItem(ItemIds::None)
    , m_currentStackSize(0)
    , m_uiTicksRemainingInState(0)
    , m_eCurrentState(State::WaitingPickup)
{
    const cpp_conv::resources::AssetPtr<cpp_conv::InserterDefinition> pFactory = cpp_conv::resources::getInserterDefinition(inserterId);
    if (!pFactory)
    {
        return;
    }

    m_transitTime = pFactory->GetTransitTime();
    m_cooldownTime = pFactory->GetCooldownTime();
    m_supportsStacks = pFactory->IsStackingInserter();
}

void cpp_conv::Inserter::Tick(const SceneContext& kContext)
{
    switch (m_eCurrentState)
    {
    case State::WaitingPickup:
        if (TryGrabItem(kContext))
        {
            m_eCurrentState = State::Transfering;
            m_uiTicksRemainingInState = m_transitTime;
        }
        break; 
    case State::Transfering:
        if (m_uiTicksRemainingInState > 0)
        {
            m_uiTicksRemainingInState--;
        }

        if (m_uiTicksRemainingInState == 0 && TryInsertItem(kContext))
        {
            m_eCurrentState = State::Cooldown;
            m_uiTicksRemainingInState = m_cooldownTime;
        }
        break;
    case State::Cooldown:
        if (m_uiTicksRemainingInState > 0)
        {
            m_uiTicksRemainingInState--;
        }

        if (m_uiTicksRemainingInState == 0)
        {
            m_eCurrentState = State::WaitingPickup;
        }
        break;
    }
}

void cpp_conv::Inserter::Draw(RenderContext& kRenderContext) const
{
    auto pFactory = cpp_conv::resources::getInserterDefinition(m_inserterId);
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
            (float)m_position.m_x * cpp_conv::renderer::c_gridScale,
            (float)m_position.m_y * cpp_conv::renderer::c_gridScale,
            cpp_conv::rotationFromDirection(m_direction)
        },
        { 0xFF0000FF });
}

std::string cpp_conv::Inserter::GetDescription() const
{
    switch (m_eCurrentState)
    {
    case State::WaitingPickup:
        return "Waiting for item to pickup";
        break;
    case State::Transfering:
    {
        cpp_conv::resources::AssetPtr<cpp_conv::ItemDefinition> pItem = cpp_conv::resources::getItemDefinition(m_currentItem);
        return std::format("Transfering {}", pItem == nullptr ? "Unknown item" : pItem->GetName());
        break;
    }
    case State::Cooldown:
        return "Cooling down";
        break;
    default:
        break;

    }
    return "";
}

bool cpp_conv::Inserter::TryGrabItem(const SceneContext& kContext)
{
    auto pSourceEntity = kContext.m_rMap.GetEntity(grid::GetBackwardsPosition(*this));
    if (!pSourceEntity || !pSourceEntity->SupportsProvidingItem())
    {
        return false;
    }

    std::tuple<ItemId, uint32_t> outItem;
    if (!pSourceEntity->TryGrab(kContext, !m_supportsStacks, outItem))
    {
        return false;
    }

    std::tie(m_currentItem, m_currentStackSize) = outItem;
    return true;
}

bool cpp_conv::Inserter::TryInsertItem(const SceneContext& kContext)
{
    auto pTargetEntity = kContext.m_rMap.GetEntity(grid::GetForwardPosition(*this));
    if (!pTargetEntity || !pTargetEntity->SupportsInsertion())
    {
        return false;
    }

    if (!pTargetEntity->TryInsert(kContext, *this, m_currentItem, 0) && !pTargetEntity->TryInsert(kContext, *this, m_currentItem, 1))
    {
        return false;
    }

    m_currentStackSize--;
    if (m_currentStackSize > 0)
    {
        return false;
    }

    m_currentItem = ItemIds::None;
    return true;
}
