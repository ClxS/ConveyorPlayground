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
#include "Direction.h"

template <class RandomAccessIterator, class URNG>
void shuffle(RandomAccessIterator first, RandomAccessIterator last, URNG&& g)
{
    for (auto i = (last - first) - 1; i > 0; --i)
    {
        std::uniform_int_distribution<decltype(i)> d(0, i);
        swap(first[i], first[d(g)]);
    }
}

std::tuple<int, Vector3> GetUndergroundLength(const cpp_conv::WorldMap& map, const cpp_conv::Entity* pStart, Direction direction)
{
    Vector3 kTargetPosition;
    int iTargetUnderground = -1;
    Vector3 kTmpPosition = pStart->m_position;
    for (int i = 0; i < cpp_conv::c_maxUndergroundLength; i++)
    {
        const Vector3 kForwardPosition = cpp_conv::grid::getForwardPosition(kTmpPosition, direction);
        const cpp_conv::Entity* pForwardEntity = map.GetEntity(kForwardPosition);
        if (pForwardEntity == nullptr || pForwardEntity->m_eEntityKind != EntityKind::Underground)
        {
            kTmpPosition = kForwardPosition;
            continue;
        }

        iTargetUnderground = i + 1; // + 1 for the start point

        // Move forward an extra one to skip the underground block
        kTmpPosition = cpp_conv::grid::getForwardPosition(kTmpPosition, direction);
        kTargetPosition = cpp_conv::grid::getForwardPosition(kTmpPosition, direction);
        break;
    }

    return std::make_tuple(iTargetUnderground, kTargetPosition);
}

cpp_conv::Underground::Underground(Vector3 position, Vector3 size, Direction direction)
    : Entity(position, size, EntityKind::Underground)
    , m_direction(direction)
    , m_arrInternalConveyors
    {
        Conveyor(position, size, direction, ItemIds::None),
        Conveyor(position, size, direction, ItemIds::None),
        Conveyor(position, size, direction, ItemIds::None),
        Conveyor(position, size, direction, ItemIds::None),
        Conveyor(position, size, direction, ItemIds::None),
        Conveyor(position, size, direction, ItemIds::None)
    }
    , m_uiTick(0)
{
    for (int i = 0; i < c_maxUndergroundLength; i++)
    {
        m_arrInternalConveyors[i].m_position = m_position;
        position = grid::getForwardPosition(position, direction);
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
    Vector3 position(grid::getForwardPosition(m_position, m_direction));

    for (int i = 0; i < iUndergroundLength; i++)
    {
        m_arrInternalConveyors[i].m_position = position;
        position = grid::getForwardPosition(position, m_direction);
    }

    for (int i = 0; i < iUndergroundLength; i++)
    {
        const bool bIsHead = (i >= iUndergroundLength - 1);
        Conveyor* pNode = &m_arrInternalConveyors[i];
        for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; iChannelIdx++)
        {
            const int iChannelLength = c_conveyorChannelSlots;

            Entity* pForwardEntity = bIsHead ? pExitEntity : &m_arrInternalConveyors[i + 1];
            ItemInstance& frontMostItem = pNode->m_pChannels[iChannelIdx].m_pSlots[iChannelLength - 1].m_Item;
            if (!frontMostItem.IsEmpty())
            {
                if (bIsHead)
                {
                    if (pForwardEntity && pForwardEntity->SupportsInsertion() && pForwardEntity->TryInsert(kContext, *pNode, InsertInfo(frontMostItem.m_Item, iChannelIdx)))
                    {
                        frontMostItem = ItemInstance::Empty();
                    }
                }
                else
                {
                    Conveyor* pForwardNode = reinterpret_cast<Conveyor*>(pForwardEntity);
                    ItemInstance& forwardTargetItem = pForwardNode->m_pChannels[iChannelIdx].m_pSlots[0].m_Item;
                    ItemInstance& forwardPendingItem = pForwardNode->m_pChannels[iChannelIdx].m_pPendingItems[0];
                    if (forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
                    {
                        forwardPendingItem = frontMostItem;
                        frontMostItem = ItemInstance::Empty();
                    }
                }
            }

            // Move inner items forwards
            for (int iChannelSlot = iChannelLength - 2; iChannelSlot >= 0; iChannelSlot--)
            {
                ItemInstance& currentItem = pNode->m_pChannels[iChannelIdx].m_pSlots[iChannelSlot].m_Item;
                ItemInstance& forwardTargetItem = pNode->m_pChannels[iChannelIdx].m_pSlots[iChannelSlot + 1].m_Item;
                ItemInstance& forwardPendingItem = pNode->m_pChannels[iChannelIdx].m_pSlots[iChannelSlot + 1].m_Item;

                if (!currentItem.IsEmpty() && forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
                {
                    forwardPendingItem = currentItem;
                    currentItem = ItemInstance::Empty();
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
    const auto pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(resources::registry::visual::Tunnel);
    if (!pTile)
    {
        return;
    }

    int iUndergroundLength;
    Vector3 undergroundEnd;

    std::tie(iUndergroundLength, undergroundEnd) = GetUndergroundLength(kRenderContext.m_rMap, this, m_direction);

    Rotation eDirection = rotationFromDirection(m_direction);
    if (iUndergroundLength == -1)
    {
        eDirection = rotationFromDirection(
            direction::Rotate90DegreeClockwise(
                direction::Rotate90DegreeClockwise(m_direction)));
    }

    renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            static_cast<float>(m_position.GetX()) * renderer::c_gridScale,
            static_cast<float>(m_position.GetY()) * renderer::c_gridScale,
            eDirection
        },
        { 0xFF0000FF });
}

bool cpp_conv::Underground::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const InsertInfo insertInfo)
{
    bool bProduced = false;
    const ItemInstance& forwardTargetItem = m_arrInternalConveyors[0].m_pChannels[insertInfo.GetSourceChannel()].m_pSlots[0].m_Item;
    ItemInstance& forwardPendingItem = m_arrInternalConveyors[0].m_pChannels[insertInfo.GetSourceChannel()].m_pSlots[0].m_Item;
    if (forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
    {
        forwardPendingItem = { insertInfo.GetItem(), 0.0f, 0.0f, false };
        return true;
    }

    return false;
}
