#include "Underground.h"
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
#include "WorldMap.h"

template <class RandomAccessIterator, class URNG>
void shuffle(RandomAccessIterator first, RandomAccessIterator last, URNG&& g)
{
    for (auto i = (last - first) - 1; i > 0; --i)
    {
        std::uniform_int_distribution<decltype(i)> d(0, i);
        swap(first[i], first[d(g)]);
    }
}

std::tuple<int, Vector3> GetUndergroundLength(const cpp_conv::WorldMap& map, cpp_conv::Entity* pStart, Direction direction)
{
    Vector3 kTargetPosition;
    int iTargetUnderground = -1;
    Vector3 kTmpPosition = pStart->m_position;
    for (int i = 0; i < cpp_conv::c_maxUndergroundLength; i++)
    {
        Vector3 kForwardPosition = cpp_conv::grid::GetForwardPosition(kTmpPosition, direction);
        const cpp_conv::Entity* pForwardEntity = map.GetEntity(kForwardPosition);
        if (pForwardEntity == nullptr || pForwardEntity->m_eEntityKind != EntityKind::Underground)
        {
            kTmpPosition = kForwardPosition;
            continue;
        }

        iTargetUnderground = i + 1; // + 1 for the start point

        // Move forward an extra one to skip the underground block
        kTmpPosition = cpp_conv::grid::GetForwardPosition(kTmpPosition, direction);
        kTargetPosition = cpp_conv::grid::GetForwardPosition(kTmpPosition, direction);
        break;
    }

    return std::make_tuple(iTargetUnderground, kTargetPosition);
}

cpp_conv::Underground::Underground(Vector3 position, Vector3 size, Direction direction)
    : Entity(position, size, EntityKind::Underground)
    , m_direction(direction)
    , m_arrInternalConveyors
    {
        cpp_conv::Conveyor(position, size, direction, cpp_conv::ItemIds::None),
        cpp_conv::Conveyor(position, size, direction, cpp_conv::ItemIds::None),
        cpp_conv::Conveyor(position, size, direction, cpp_conv::ItemIds::None),
        cpp_conv::Conveyor(position, size, direction, cpp_conv::ItemIds::None),
        cpp_conv::Conveyor(position, size, direction, cpp_conv::ItemIds::None),
        cpp_conv::Conveyor(position, size, direction, cpp_conv::ItemIds::None)
    }    
    , m_uiTick(0)
{
    for (int i = 0; i < cpp_conv::c_maxUndergroundLength; i++)
    {
        m_arrInternalConveyors[i].m_position = m_position;
        position = cpp_conv::grid::GetForwardPosition(position, direction);
    }
}

void cpp_conv::Underground::Tick(const SceneContext& kContext)
{
    int iUndergroundLength;
    Vector3 undergroundEnd;
    
    std::tie(iUndergroundLength, undergroundEnd) = GetUndergroundLength(kContext.m_rMap, this, m_direction);
    if (iUndergroundLength == -1)
    {
        return;
    }

    Entity* pExitEntity = kContext.m_rMap.GetEntity(undergroundEnd);
    Vector3 position(cpp_conv::grid::GetForwardPosition(m_position, m_direction));

    for (int i = 0; i < iUndergroundLength; i++)
    {
        m_arrInternalConveyors[i].m_position = position;
        position = cpp_conv::grid::GetForwardPosition(position, m_direction);
    }

    for (int i = 0; i < iUndergroundLength; i++)
    {
        bool bIsHead = (i >= iUndergroundLength - 1);
        Conveyor* pNode = &m_arrInternalConveyors[i];
        for (int iChannelIdx = 0; iChannelIdx < cpp_conv::c_conveyorChannels; iChannelIdx++)
        {
            int iChannelLength = cpp_conv::c_conveyorChannelSlots;

            cpp_conv::Entity* pForwardEntity = bIsHead ? pExitEntity : &m_arrInternalConveyors[i + 1];
            ItemId& frontMostItem = pNode->m_pChannels[iChannelIdx].m_pItems[iChannelLength - 1];
            if (!frontMostItem.IsEmpty())
            {
                if (bIsHead)
                {
                    if (pForwardEntity && pForwardEntity->SupportsInsertion() && pForwardEntity->TryInsert(kContext, *pNode, frontMostItem, iChannelIdx))
                    {
                        frontMostItem = cpp_conv::ItemIds::None;
                    }
                }
                else
                {
                    cpp_conv::Conveyor* pForwardNode = reinterpret_cast<cpp_conv::Conveyor*>(pForwardEntity);
                    ItemId& forwardTargetItem = pForwardNode->m_pChannels[iChannelIdx].m_pItems[0];
                    ItemId& forwardPendingItem = pForwardNode->m_pChannels[iChannelIdx].m_pPendingItems[0];
                    if (forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
                    {
                        forwardPendingItem = frontMostItem;
                        frontMostItem = cpp_conv::ItemIds::None;
                    }
                }
            }

            // Move inner items forwards
            for (int iChannelSlot = iChannelLength - 2; iChannelSlot >= 0; iChannelSlot--)
            {
                ItemId& currentItem = pNode->m_pChannels[iChannelIdx].m_pItems[iChannelSlot];
                ItemId& forwardTargetItem = pNode->m_pChannels[iChannelIdx].m_pItems[iChannelSlot + 1];
                ItemId& forwardPendingItem = pNode->m_pChannels[iChannelIdx].m_pItems[iChannelSlot + 1];

                if (currentItem.IsValid() && forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
                {
                    forwardPendingItem = currentItem;
                    currentItem = cpp_conv::ItemIds::None;
                }
            }
        }
    }

    for (int i = 0; i < iUndergroundLength; i++)
    {
        m_arrInternalConveyors[i].Tick(kContext);
    }
}

void cpp_conv::Underground::Draw(RenderContext& kRenderContext) const
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
        { 0xFF0000FF });
}

bool cpp_conv::Underground::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel)
{
    bool bProduced = false;
    ItemId& forwardTargetItem = m_arrInternalConveyors[0].m_pChannels[iSourceChannel].m_pItems[0];
    ItemId& forwardPendingItem = m_arrInternalConveyors[0].m_pChannels[iSourceChannel].m_pItems[0];
    if (forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
    {
        forwardPendingItem = pItem;
        return true;
    }

    return false;
}
