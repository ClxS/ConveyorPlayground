#include "Inserter.h"
#include "Conveyor.h"
#include "SceneContext.h"
#include "Sequence.h"

#include "RenderContext.h"
#include "Renderer.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <random>
#include "EntityGrid.h"
#include "InserterDefinition.h"
#include "InserterRegistry.h"
#include "ItemDefinition.h"
#include "ItemRegistry.h"
#include "ResourceManager.h"
#include "TileAsset.h"

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
    const resources::AssetPtr<InserterDefinition> pFactory = resources::getInserterDefinition(inserterId);
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
    const auto pFactory = resources::getInserterDefinition(m_inserterId);
    if (!pFactory)
    {
        return;
    }

    const auto pTile = pFactory->GetTile();
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
        {0xFF0000FF});
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
            const resources::AssetPtr<ItemDefinition> pItem = resources::getItemDefinition(m_currentItem);
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
    const auto pSourceEntity = kContext.m_rMap.GetEntity(grid::getBackwardsPosition(*this));
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
    const auto pTargetEntity = kContext.m_rMap.GetEntity(grid::getForwardPosition(*this));
    if (!pTargetEntity || !pTargetEntity->SupportsInsertion())
    {
        return false;
    }

    if (!pTargetEntity->TryInsert(kContext, *this, InsertInfo(m_currentItem, 0)) && !pTargetEntity->TryInsert(
        kContext, *this, InsertInfo(m_currentItem, 1)))
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
