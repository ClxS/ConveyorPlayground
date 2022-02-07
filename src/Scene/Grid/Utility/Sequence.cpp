#include "Sequence.h"

#include "TargetingUtility.h"
#include "Junction.h"
#include "Underground.h"
#include "SceneContext.h"
#include "Direction.h"
#include "Profiler.h"
#include "WorldMap.h"
#include <cassert>
#include <xutility>
#include <bitset>
#include <bit>
using namespace cpp_conv;

Conveyor* cpp_conv::TraceHeadConveyor(cpp_conv::WorldMap& map, Conveyor& searchStart)
{
    static RelativeDirection directionPriority[] =
    {
        RelativeDirection::Backwards,
        RelativeDirection::Right,
        RelativeDirection::Left,
    };

    Conveyor* pCurrentConveyor = map.GetEntity<Conveyor>(searchStart.m_position, EntityKind::Conveyor);
    while (true)
    {
        Vector3 forwardPosition = grid::GetForwardPosition(*pCurrentConveyor);
        Conveyor* pTargetConveyor = map.GetEntity<Conveyor>(forwardPosition, EntityKind::Conveyor);
        if (pTargetConveyor == nullptr || pTargetConveyor->IsCorner())
        {
            break;
        }

        const Entity* vPotentialNeighbours[4];
        vPotentialNeighbours[(int)RelativeDirection::Backwards] = map.GetEntity<Conveyor>(grid::GetBackwardsPosition(*pTargetConveyor), EntityKind::Conveyor);
        vPotentialNeighbours[(int)RelativeDirection::Right] = map.GetEntity<Conveyor>(grid::GetRightPosition(*pTargetConveyor), EntityKind::Conveyor);
        vPotentialNeighbours[(int)RelativeDirection::Left] = map.GetEntity<Conveyor>(grid::GetLeftPosition(*pTargetConveyor), EntityKind::Conveyor);

        for (auto direction : directionPriority)
        {
            const Entity* pDirectionEntity = vPotentialNeighbours[(int)direction];
            if (pDirectionEntity == nullptr || pDirectionEntity->m_eEntityKind != EntityKind::Conveyor)
            {
                continue;
            }

            if (pDirectionEntity == pCurrentConveyor)
            {
                continue;
            }

            if (grid::GetForwardPosition(*pDirectionEntity) == forwardPosition)
            {
                return pCurrentConveyor;
            }
        }

        if (pTargetConveyor == &searchStart || pTargetConveyor->IsCorner())
        {
            break;
        }

        pCurrentConveyor = reinterpret_cast<Conveyor*>(pTargetConveyor);
    }

    return pCurrentConveyor;
}

const Conveyor* cpp_conv::TraceTailConveyor(cpp_conv::WorldMap& map, Conveyor& searchStart, Conveyor& head, std::vector<Conveyor*>& vOutConveyors)
{
    Conveyor* pCurrentConveyor = map.GetEntity<Conveyor>(searchStart.m_position, EntityKind::Conveyor);

    assert(pCurrentConveyor != nullptr); // If it is null here, we are misusing this method.

    while (true)
    {
        vOutConveyors.push_back(pCurrentConveyor);

        Entity* pTargetConveyor = cpp_conv::targeting_util::FindNextTailConveyor(map, *pCurrentConveyor);
        if (!pTargetConveyor ||
            pTargetConveyor->m_eEntityKind != EntityKind::Conveyor ||
            reinterpret_cast<Conveyor*>(pTargetConveyor)->IsCorner() ||
            pTargetConveyor == &searchStart ||
            pTargetConveyor == &head)
        {
            break;
        }

        pCurrentConveyor = reinterpret_cast<Conveyor*>(pTargetConveyor);
    }

    std::reverse(vOutConveyors.begin(), vOutConveyors.end());
    return pCurrentConveyor;
}

std::vector<Sequence*> cpp_conv::InitializeSequences(cpp_conv::WorldMap& map, const std::vector<Conveyor*>& conveyors)
{
    std::vector<Sequence*> vSequences;
    cpp_conveyor::vector_set<const Conveyor*> alreadyProcessedConveyors(conveyors.size());

    int iId = 0;
    for (auto& conveyor : conveyors)
    {
        if (alreadyProcessedConveyors.contains(conveyor))
        {
            continue;
        }

        if (conveyor->IsCorner())
        {
            conveyor->ClearSequence();
            continue;
        }

        std::vector<Conveyor*> vConveyors;
        Conveyor* pHeadConveyor = TraceHeadConveyor(map, *conveyor);
        const Conveyor* pTailConveyor = TraceTailConveyor(map, *pHeadConveyor, *pHeadConveyor, vConveyors);

        const auto end = vConveyors.end();
        std::vector<Conveyor*>::iterator chunk_begin;
        std::vector<Conveyor*>::iterator chunk_end;
        chunk_end = chunk_begin = vConveyors.begin();
        do
        {
            if (std::distance(chunk_end, end) < Sequence::c_uiMaxSequenceLength)
            {
                chunk_end = end;
            }
            else
            {
                std::advance(chunk_end, Sequence::c_uiMaxSequenceLength);
            }

            std::vector<Conveyor*> vSequenceConveyors(chunk_begin, chunk_end);
            pTailConveyor = vSequenceConveyors.front();

            vSequences.push_back(new Sequence(
                vSequenceConveyors[vSequenceConveyors.size() - 1],
                (uint8_t)vSequenceConveyors.size(),
                pTailConveyor->m_pChannels[0].m_pSlots[0].m_VisualPosition,
                pTailConveyor->m_pChannels[1].m_pSlots[0].m_VisualPosition,
                pTailConveyor->m_pChannels[0].m_pSlots[1].m_VisualPosition - pTailConveyor->m_pChannels[0].m_pSlots[0].m_VisualPosition
            ));

            Sequence* sequence = vSequences.back();
            for(size_t i = 0; i < vSequenceConveyors.size(); ++i)
            {
                vSequenceConveyors[i]->SetSequence(sequence, i);
                alreadyProcessedConveyors.insert(vSequenceConveyors[i]);
            }

            chunk_begin = chunk_end;
        }
        while (std::distance(chunk_begin, end) > 0);
    }

    return vSequences;
}

std::tuple<int, Direction> cpp_conv::GetInnerMostCornerChannel(const cpp_conv::WorldMap& map, const Conveyor& rConveyor)
{
    PROFILE_FUNC();
    const Entity* pBackConverter = cpp_conv::targeting_util::FindNextTailConveyor(map, rConveyor);
    if (pBackConverter == nullptr || pBackConverter->GetDirection() == rConveyor.m_direction)
    {
        return std::make_tuple(-1, Direction::Up);
    }

    Direction selfDirection = rConveyor.m_direction;
    Direction backDirection = pBackConverter->GetDirection();
    while (selfDirection != Direction::Up)
    {
        selfDirection = cpp_conv::direction::Rotate90DegreeClockwise(selfDirection);
        backDirection = cpp_conv::direction::Rotate90DegreeClockwise(backDirection);
    }

    return std::make_tuple(backDirection == Direction::Right ? 1 : 0, pBackConverter->GetDirection());
}

bool MoveItemToForwardsNode(cpp_conv::SceneContext& kContext, const cpp_conv::Conveyor& pNode, int lane)
{
    static ItemId s_Item = cpp_conv::ItemId::FromStringId("ITEM_COPPER_ORE");

    cpp_conv::Entity* pForwardEntity = kContext.m_rMap.GetEntity(grid::GetForwardPosition(pNode));
    return (pForwardEntity && pForwardEntity->SupportsInsertion() && pForwardEntity->TryInsert(kContext, pNode, s_Item, lane));
}

void cpp_conv::Sequence::Tick(SceneContext& kContext)
{
    m_pHeadConveyor->m_uiCurrentTick++;
    if (m_pHeadConveyor->m_uiCurrentTick < m_pHeadConveyor->m_uiMoveTick)
    {
        return;
    }

    m_RealizedState.m_RealizedMovements[0] = 0;
    m_RealizedState.m_RealizedMovements[1] = 0;
    m_pHeadConveyor->m_uiCurrentTick = 0;

    for (uint8_t uiLane = 0; uiLane < c_conveyorChannels; uiLane++)
    {
        bool bIsLeadItemFull = (m_RealizedState.m_Lanes[uiLane] & 0b1) == 1;
        if (bIsLeadItemFull)
        {
            if (MoveItemToForwardsNode(kContext, *m_pHeadConveyor, (int)uiLane))
            {
                m_PendingState.m_PendingClears[uiLane] = 0b1;
                m_RealizedState.m_Items[uiLane].Pop();
                bIsLeadItemFull = false;
            }
        }

        if (bIsLeadItemFull)
        {
            uint64_t uiMaxMask = (1ULL << std::countr_one(m_RealizedState.m_Lanes[uiLane])) - 1ULL;
            m_PendingState.m_PendingClears[uiLane] = ~uiMaxMask;

            uint64_t uiNewPositions = m_RealizedState.m_Lanes[uiLane] >> 1;
            uint64_t uiOverlaps = uiNewPositions & m_PendingState.m_PendingMoves[uiLane];
            if (uiOverlaps == 0)
            {
                m_PendingState.m_PendingMoves[uiLane] |= m_RealizedState.m_Lanes[uiLane] >> 1;
                m_PendingState.m_PendingMoves[uiLane] &= ~uiMaxMask;
            }
        }
        else
        {
            uint64_t uiNewPositions = m_RealizedState.m_Lanes[uiLane] >> 1;
            uint64_t uiOverlaps = uiNewPositions & m_PendingState.m_PendingMoves[uiLane];
            if (uiOverlaps == 0)
            {
                // No mid-insert collision fast path
                m_PendingState.m_PendingClears[uiLane] = m_RealizedState.m_Lanes[uiLane];
                m_PendingState.m_PendingMoves[uiLane] |= uiNewPositions;
            }
            else
            {
                m_PendingState.m_PendingClears[uiLane] = m_RealizedState.m_Lanes[uiLane];
                do
                {
                    // Determine save area
                    uint64_t uiCollisionBit;
                    uint64_t safeRegionMask;
                    {
                        uiCollisionBit = std::countr_zero(uiOverlaps);
                        uiOverlaps &= ~(1ULL << uiCollisionBit);

                        // Everything below our collision bit is safe to move
                        safeRegionMask = (1ULL << uiCollisionBit) - 1;
                    }

                    // Perform safe move area
                    {
                        m_PendingState.m_PendingMoves[uiLane] |= uiNewPositions & safeRegionMask;

                        // We can't clear our the original position
                        m_PendingState.m_PendingClears[uiLane] &= ~(1ULL << uiCollisionBit);

                        // Zero out the safe region so we know what remains
                        uiNewPositions &= ~safeRegionMask;
                    }

                    //
                    {
                        // We can't move anything else until the following 0 bit
                        uint64_t uiNextMovableBit = std::countr_zero(uiOverlaps);
                        uint64_t uiClearRange = (1ULL << uiNextMovableBit) & ~safeRegionMask;

                        m_PendingState.m_PendingClears[uiLane] &= ~uiClearRange;
                        m_PendingState.m_PendingMoves[uiLane] &= ~uiClearRange;
                    }
                }
                while (uiOverlaps != 0);
            }
        }
    }
}

void Sequence::Realize()
{
    for (uint8_t uiLane = 0; uiLane < c_conveyorChannels; uiLane++)
    {
        m_RealizedState.m_Lanes[uiLane] &= ~m_PendingState.m_PendingClears[uiLane];
        m_RealizedState.m_Lanes[uiLane] |= m_PendingState.m_PendingMoves[uiLane];
        m_RealizedState.m_RealizedMovements[uiLane] |= m_PendingState.m_PendingMoves[uiLane];
        m_PendingState.m_PendingClears[uiLane] = 0;
        m_PendingState.m_PendingMoves[uiLane] = 0;

        uint64_t uiInsertions = m_PendingState.m_pPendingInsertions[uiLane];
        m_PendingState.m_pPendingInsertions[uiLane] = 0;

         while (uiInsertions != 0)
        {
            const uint32_t uiCurrentInsertIndex = 1U << std::countr_zero(uiInsertions);
            const uint32_t uiEarlierItemsMask = uiCurrentInsertIndex - 1;
            uiInsertions &= ~uiCurrentInsertIndex;
            const uint64_t previousItemCount = std::popcount(m_RealizedState.m_Lanes[uiLane] & uiEarlierItemsMask);
            m_RealizedState.m_Items[uiLane].Insert(previousItemCount, m_PendingState.m_NewItems[uiLane].Pop());
        }
    }
}

uint64_t Sequence::CountItemsOnBelt()
{
    return std::popcount(m_RealizedState.m_Lanes[0]) + std::popcount(m_RealizedState.m_Lanes[1]);
}

void Sequence::AddItemInSlot(uint8_t uiSequenceIndex, int lane, int slot, ItemId item, const Vector2F* origin)
{
    uint64_t uiSetMask = 1ULL << (m_Length * 2 - uiSequenceIndex * 2 - slot - 1);
    m_PendingState.m_pPendingInsertions[lane] |= uiSetMask;
    m_PendingState.m_PendingMoves[lane] |= uiSetMask;

    // Gets all bits below the current insertion point
    uint64_t uiPreviousInsertMask = m_PendingState.m_pPendingInsertions[lane] & (uiSetMask - 1);
    uint8_t uiPreviousCount = std::popcount(uiPreviousInsertMask);
    if (origin == nullptr)
    {
        m_PendingState.m_NewItems[lane].Insert(uiPreviousCount, { item });
    }
    else
    {
        m_PendingState.m_NewItems[lane].Insert(uiPreviousCount, { item, *origin });
    }
}

bool Sequence::HasItemInSlot(uint8_t uiSequenceIndex, int lane, int slot) const
{
    return (((m_RealizedState.m_Lanes[lane] | m_PendingState.m_PendingMoves[lane]) >> (m_Length * 2 - uiSequenceIndex * 2 - slot - 1)) & 0b1) == 1;
}

bool Sequence::DidItemMoveLastSimulation(uint8_t uiSequenceIndex, int lane, int slot) const
{
    return ((m_RealizedState.m_RealizedMovements[lane] >> (m_Length * 2 - uiSequenceIndex * 2 - slot - 1)) & 0b1) == 1;
}

bool Sequence::TryPeakItemInSlot(uint8_t uiSequenceIndex, int lane, int slot, ItemInstance& pItem)
{
    if (!HasItemInSlot(uiSequenceIndex, lane, slot))
    {
        return false;
    }

    LaneVisual& visual = m_InitializationState.m_ConveyorVisualOffsets[lane];
    Vector2F startPosition = visual.m_Origin + visual.m_UnitDirection * ((uiSequenceIndex * 2) + slot - 1);

    static ItemId s_Item = cpp_conv::ItemId::FromStringId("ITEM_COPPER_ORE");
    pItem = { s_Item, startPosition.GetX(), startPosition.GetY(), DidItemMoveLastSimulation(uiSequenceIndex, lane, slot) };
    return true;
}
