#include "Tunnel.h"
#include "Conveyor.h"
#include "SceneContext.h"

#include "RenderContext.h"
#include "Renderer.h"

#include <array>
#include <random>
#include "Direction.h"
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

cpp_conv::Tunnel::Tunnel(Vector3 position, Vector3 size, Direction direction)
    : Entity(position, size, EntityKind::Underground)
    , m_Direction(direction)
    , m_ArrInternalConveyors
    {
        Conveyor(position, size, direction, ItemIds::None),
        Conveyor(position, size, direction, ItemIds::None),
        Conveyor(position, size, direction, ItemIds::None),
        Conveyor(position, size, direction, ItemIds::None),
        Conveyor(position, size, direction, ItemIds::None),
        Conveyor(position, size, direction, ItemIds::None)
    }
    , m_Tick(0)
{
    for (int i = 0; i < c_maxUndergroundLength; i++)
    {
        m_ArrInternalConveyors[i].m_position = m_position;
        position = grid::getForwardPosition(position, direction);
    }
}

void cpp_conv::Tunnel::Tick(const SceneContext& kContext)
{
    auto [iUndergroundLength, undergroundEnd] = GetUndergroundLength(kContext.m_rMap, this, m_Direction);
    if (iUndergroundLength == -1)
    {
        return;
    }

    Entity* pExitEntity = kContext.m_rMap.GetEntity(undergroundEnd);
    Vector3 position(grid::getForwardPosition(m_position, m_Direction));

    for (int i = 0; i < iUndergroundLength; i++)
    {
        m_ArrInternalConveyors[i].m_position = position;
        position = grid::getForwardPosition(position, m_Direction);
    }

    for (int i = 0; i < iUndergroundLength; i++)
    {
        const bool bIsHead = (i >= iUndergroundLength - 1);
        Conveyor* pNode = &m_ArrInternalConveyors[i];
        for (int iChannelIdx = 0; iChannelIdx < c_conveyorChannels; iChannelIdx++)
        {
            constexpr int iChannelLength = c_conveyorChannelSlots;

            Entity* pForwardEntity = bIsHead ? pExitEntity : &m_ArrInternalConveyors[i + 1];
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
                    const auto pForwardNode = static_cast<Conveyor*>(pForwardEntity);
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
        m_ArrInternalConveyors[i].Tick(kContext);
    }
}

void cpp_conv::Tunnel::Draw(RenderContext& kRenderContext) const
{
    const auto pTile = cpp_conv::resources::resource_manager::loadAsset<resources::TileAsset>(resources::registry::assets::conveyors::c_Tunnel);
    if (!pTile)
    {
        return;
    }

    auto [iUndergroundLength, undergroundEnd] = GetUndergroundLength(kRenderContext.m_rMap, this, m_Direction);

    Rotation eDirection = rotationFromDirection(m_Direction);
    if (iUndergroundLength == -1)
    {
        eDirection = rotationFromDirection(
            direction::rotate90DegreeClockwise(
                direction::rotate90DegreeClockwise(m_Direction)));
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

bool cpp_conv::Tunnel::TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, const InsertInfo insertInfo)
{
    bool bProduced = false;
    const ItemInstance& forwardTargetItem = m_ArrInternalConveyors[0].m_pChannels[insertInfo.GetSourceChannel()].m_pSlots[0].m_Item;
    ItemInstance& forwardPendingItem = m_ArrInternalConveyors[0].m_pChannels[insertInfo.GetSourceChannel()].m_pSlots[0].m_Item;
    if (forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
    {
        forwardPendingItem = { insertInfo.GetItem(), 0.0f, 0.0f, false };
        return true;
    }

    return false;
}
