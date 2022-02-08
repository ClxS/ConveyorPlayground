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

#include "vector_set.h"
using namespace cpp_conv;

Conveyor* cpp_conv::TraceHeadConveyor(WorldMap& map, Conveyor& searchStart)
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

const Conveyor* cpp_conv::TraceTailConveyor(WorldMap& map, Conveyor& searchStart, Conveyor& head, std::vector<Conveyor*>& vOutConveyors)
{
    Conveyor* pCurrentConveyor = map.GetEntity<Conveyor>(searchStart.m_position, EntityKind::Conveyor);

    assert(pCurrentConveyor != nullptr); // If it is null here, we are misusing this method.

    while (true)
    {
        vOutConveyors.push_back(pCurrentConveyor);

        Entity* pTargetConveyor = targeting_util::FindNextTailConveyor(map, *pCurrentConveyor);
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

    std::ranges::reverse(vOutConveyors);
    return pCurrentConveyor;
}

std::vector<Sequence*> cpp_conv::InitializeSequences(WorldMap& map, const std::vector<Conveyor*>& conveyors)
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
                vSequenceConveyors[i]->SetSequence(sequence, static_cast<uint8_t>(i));
                alreadyProcessedConveyors.insert(vSequenceConveyors[i]);
            }

            chunk_begin = chunk_end;
        }
        while (std::distance(chunk_begin, end) > 0);
    }

    return vSequences;
}

std::tuple<int, Direction> cpp_conv::GetInnerMostCornerChannel(const WorldMap& map, const Conveyor& rConveyor)
{
    PROFILE_FUNC();
    const Entity* pBackConverter = targeting_util::FindNextTailConveyor(map, rConveyor);
    if (pBackConverter == nullptr || pBackConverter->GetDirection() == rConveyor.m_direction)
    {
        return std::make_tuple(-1, Direction::Up);
    }

    Direction selfDirection = rConveyor.m_direction;
    Direction backDirection = pBackConverter->GetDirection();
    while (selfDirection != Direction::Up)
    {
        selfDirection = direction::Rotate90DegreeClockwise(selfDirection);
        backDirection = direction::Rotate90DegreeClockwise(backDirection);
    }

    return std::make_tuple(backDirection == Direction::Right ? 1 : 0, pBackConverter->GetDirection());
}

bool Sequence::MoveItemToForwardsNode(const SceneContext& kContext, const Conveyor& pNode, const int lane) const
{
    const SlotItem item = m_RealizedStates[lane].m_Items.Peek();
    const Vector2F startPosition = GetSlotPosition(0, lane, 1);
    Entity* pForwardEntity = kContext.m_rMap.GetEntity(grid::GetForwardPosition(pNode));
    return (item.HasItem() && pForwardEntity && pForwardEntity->SupportsInsertion() && pForwardEntity->TryInsert(kContext, pNode, Entity::InsertInfo(item.m_Item, static_cast<uint8_t>(lane), startPosition)));
}

void Sequence::Tick(const SceneContext& kContext)
{
    m_pHeadConveyor->m_uiCurrentTick++;
    if (m_pHeadConveyor->m_uiCurrentTick < m_pHeadConveyor->m_uiMoveTick)
    {
        return;
    }

    m_pHeadConveyor->m_uiCurrentTick = 0;

    for (uint8_t uiLane = 0; uiLane < c_conveyorChannels; uiLane++)
    {
        RealizedState& realizedState = m_RealizedStates[uiLane];
        PendingState& pendingState = m_PendingStates[uiLane];
        realizedState.m_RealizedMovements = 0;
        realizedState.m_HasOverridePosition = 0;

        bool bIsLeadItemFull = (realizedState.m_Lanes & 0b1) == 1;
        if (bIsLeadItemFull)
        {
            if (MoveItemToForwardsNode(kContext, *m_pHeadConveyor, (int)uiLane))
            {
                pendingState.m_PendingClears = 0b1;
                realizedState.m_Items.Pop();
                bIsLeadItemFull = false;
            }
        }

        if (bIsLeadItemFull)
        {
            const uint64_t uiMaxMask = (1ULL << std::countr_one(realizedState.m_Lanes)) - 1ULL;
            pendingState.m_PendingClears = ~uiMaxMask;

            const uint64_t uiNewPositions = realizedState.m_Lanes >> 1;
            const uint64_t uiOverlaps = uiNewPositions & pendingState.m_PendingMoves;
            if (uiOverlaps == 0)
            {
                pendingState.m_PendingMoves |= realizedState.m_Lanes >> 1;
                pendingState.m_PendingMoves &= ~uiMaxMask;
            }
        }
        else
        {
            uint64_t uiNewPositions = realizedState.m_Lanes >> 1;
            uint64_t uiOverlaps = uiNewPositions & pendingState.m_PendingMoves;
            if (uiOverlaps == 0)
            {
                // No mid-insert collision fast path
                pendingState.m_PendingClears = realizedState.m_Lanes;
                pendingState.m_PendingMoves |= uiNewPositions;
            }
            else
            {
                pendingState.m_PendingClears = realizedState.m_Lanes;
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
                        pendingState.m_PendingMoves |= uiNewPositions & safeRegionMask;

                        // We can't clear our the original position
                        pendingState.m_PendingClears &= ~(1ULL << uiCollisionBit);

                        // Zero out the safe region so we know what remains
                        uiNewPositions &= ~safeRegionMask;
                    }

                    //
                    {
                        // We can't move anything else until the following 0 bit
                        const uint64_t uiNextMovableBit = std::countr_zero(uiOverlaps);
                        const uint64_t uiClearRange = (1ULL << uiNextMovableBit) & ~safeRegionMask;

                        pendingState.m_PendingClears &= ~uiClearRange;
                        pendingState.m_PendingMoves &= ~uiClearRange;
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
        RealizedState& realizedState = m_RealizedStates[uiLane];
        PendingState& pendingState = m_PendingStates[uiLane];

        realizedState.m_Lanes &= ~pendingState.m_PendingClears;
        realizedState.m_Lanes |= pendingState.m_PendingMoves;
        realizedState.m_RealizedMovements |= pendingState.m_PendingMoves;
        pendingState.m_PendingClears = 0;
        pendingState.m_PendingMoves = 0;

        uint64_t uiInsertions = pendingState.m_PendingInsertions;
        pendingState.m_PendingInsertions = 0;

        while (uiInsertions != 0)
        {
            const SlotItem item = pendingState.m_NewItems.Pop();
            const uint32_t uiCurrentInsertIndex = 1U << std::countr_zero(uiInsertions);
            if (item.m_bHasPosition)
            {
                realizedState.m_HasOverridePosition |= uiCurrentInsertIndex;
            }

            const uint32_t uiEarlierItemsMask = uiCurrentInsertIndex - 1;
            uiInsertions &= ~uiCurrentInsertIndex;
            const uint64_t previousItemCount = std::popcount(realizedState.m_Lanes & uiEarlierItemsMask);
            realizedState.m_Items.Insert(previousItemCount, item);
        }
    }
}

uint64_t Sequence::CountItemsOnBelt() const
{
    return std::popcount(m_RealizedStates[0].m_Lanes) + std::popcount(m_RealizedStates[1].m_Lanes);
}

void Sequence::AddItemInSlot(const uint8_t uiSequenceIndex, const int lane, const int slot, ItemId item, const Vector2F* origin)
{
    PendingState& pendingState = m_PendingStates[lane];

    const uint64_t uiSetMask = 1ULL << (m_Length * 2 - uiSequenceIndex * 2 - slot - 1);
    pendingState.m_PendingInsertions |= uiSetMask;
    pendingState.m_PendingMoves |= uiSetMask;

    // Gets all bits below the current insertion point
    const uint64_t uiPreviousInsertMask = pendingState.m_PendingInsertions & (uiSetMask - 1);
    const int uiPreviousCount = std::popcount(uiPreviousInsertMask);
    if (origin == nullptr)
    {
        pendingState.m_NewItems.Insert(uiPreviousCount, SlotItem(item));
    }
    else
    {
        pendingState.m_NewItems.Insert(uiPreviousCount, SlotItem(item, *origin));
    }
}

bool Sequence::HasItemInSlot(const uint8_t uiSequenceIndex, const int lane, const int slot) const
{
    return (((m_RealizedStates[lane].m_Lanes | m_PendingStates[lane].m_PendingMoves) >> (m_Length * 2 - uiSequenceIndex * 2 - slot - 1)) & 0b1) == 1;
}

bool Sequence::DidItemMoveLastSimulation(const uint8_t uiSequenceIndex, const int lane, const int slot) const
{
    return ((m_RealizedStates[lane].m_RealizedMovements >> (m_Length * 2 - uiSequenceIndex * 2 - slot - 1)) & 0b1) == 1;
}

Vector2F Sequence::GetSlotPosition(const uint8_t uiSequenceIndex, const int lane, const int slot) const
{
    const LaneVisual& visual = m_InitializationState.m_ConveyorVisualOffsets[lane];
    return visual.m_Origin + visual.m_UnitDirection * ((uiSequenceIndex * 2) + slot);
}

bool Sequence::TryPeakItemInSlot(const uint8_t uiSequenceIndex, const int lane, const int slot, ItemInstance& pItem) const
{
    if (!HasItemInSlot(uiSequenceIndex, lane, slot))
    {
        return false;
    }

    const RealizedState& realizedState = m_RealizedStates[lane];
    const uint64_t uiSetMask = 1ULL << (m_Length * 2 - uiSequenceIndex * 2 - slot - 1);
    const uint64_t uiPreviousInsertMask = realizedState.m_Lanes & (uiSetMask - 1);
    const int uiPreviousCount = std::popcount(uiPreviousInsertMask);
    const SlotItem item = realizedState.m_Items.Peek(uiPreviousCount);

    if (!item.HasItem())
    {
        return false;
    }

    Vector2F startPosition;
    const bool bHasOverridePosition = ((realizedState.m_HasOverridePosition >> (m_Length * 2 - uiSequenceIndex * 2 - slot - 1)) & 0b1) == 1;

    if (bHasOverridePosition)
    {
        startPosition = item.m_Position;
    }
    else
    {
        startPosition = GetSlotPosition(uiSequenceIndex, lane, slot - 1);
    }

    pItem = { item.m_Item, startPosition.GetX(), startPosition.GetY(), DidItemMoveLastSimulation(uiSequenceIndex, lane, slot) };
    return true;
}
