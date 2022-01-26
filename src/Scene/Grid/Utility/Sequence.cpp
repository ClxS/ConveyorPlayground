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

        auto begin = vConveyors.begin();
        auto end = vConveyors.end();
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
            for(int i = 0; i < vSequenceConveyors.size(); ++i)
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

int First0Bit(uint64_t i)
{
    return std::countr_one(i);
}

void cpp_conv::Sequence::Tick(SceneContext& kContext)
{
    m_pHeadConveyor->m_uiCurrentTick++;
    if (m_pHeadConveyor->m_uiCurrentTick < m_pHeadConveyor->m_uiMoveTick)
    {
        return;
    } 

    m_LanesRealizedMovements[0] = 0;
    m_LanesRealizedMovements[1] = 0;
    m_pHeadConveyor->m_uiCurrentTick = 0;

    for (uint8_t uiLane = 0; uiLane < c_conveyorChannels; uiLane++)
    {
        bool bIsLeadItemFull = (m_Lanes[uiLane] & 0b1) == 1;
        if (bIsLeadItemFull)
        {
            if (MoveItemToForwardsNode(kContext, *m_pHeadConveyor, (int)uiLane))
            {
                m_PendingClears[uiLane] = 0b1;
                bIsLeadItemFull = false;
            }
        }

        if (bIsLeadItemFull)
        {
            std::bitset<64> x(m_Lanes[uiLane]);
            uint64_t uiMaxMask = (1ULL << First0Bit(m_Lanes[uiLane])) - 1ULL;
            m_PendingClears[uiLane] = ~uiMaxMask;
            m_PendingMoves[uiLane] = m_Lanes[uiLane] >> 1;
            m_PendingMoves[uiLane] &= ~uiMaxMask;
            std::bitset<64> x2(m_Lanes[uiLane]);
        }
        else
        {
            m_PendingClears[uiLane] = m_Lanes[uiLane];
            m_PendingMoves[uiLane] |= m_Lanes[uiLane] >> 1;
        }
    }
}


void Sequence::Realize()
{
    for (uint8_t uiLane = 0; uiLane < c_conveyorChannels; uiLane++)
    {
        m_Lanes[uiLane] &= ~m_PendingClears[uiLane];
        m_Lanes[uiLane] |= m_PendingMoves[uiLane];
        m_LanesRealizedMovements[uiLane] |= m_PendingMoves[uiLane];
        m_PendingClears[uiLane] = 0;
        m_PendingMoves[uiLane] = 0;
    }
}

uint64_t Sequence::CountItemsOnBelt()
{
    return std::popcount(m_Lanes[0]) + std::popcount(m_Lanes[1]);
}

void Sequence::AddItemInSlot(uint8_t uiSequenceIndex, int lane, int slot)
{ 
    m_PendingMoves[lane] |= 1ULL << (m_Length * 2 - uiSequenceIndex * 2 - slot - 1);
} 

bool Sequence::HasItemInSlot(uint8_t uiSequenceIndex, int lane, int slot) const
{
    bool bMainLane = ((m_Lanes[lane] >> (m_Length * 2 - uiSequenceIndex * 2 - slot - 1)) & 0b1) == 1;
    return bMainLane || ((m_PendingMoves[lane] >> (m_Length * 2 - uiSequenceIndex * 2 - slot - 1)) & 0b1) == 1;
}

bool Sequence::DidItemMoveLastSimulation(uint8_t uiSequenceIndex, int lane, int slot) const
{
    return ((m_LanesRealizedMovements[lane] >> (m_Length * 2 - uiSequenceIndex * 2 - slot - 1)) & 0b1) == 1;
}

bool Sequence::TryPeakItemInSlot(uint8_t uiSequenceIndex, int lane, int slot, ItemInstance& pItem)
{
    if (!HasItemInSlot(uiSequenceIndex, lane, slot))
    {
        return false;
    }

    LaneVisual& visual = m_ConveyorVisualOffsets[lane];
    Vector2F startPosition = visual.m_Origin + visual.m_UnitDirection * ((uiSequenceIndex * 2) + slot - 1);

    static ItemId s_Item = cpp_conv::ItemId::FromStringId("ITEM_COPPER_ORE");
    pItem = { s_Item, startPosition.GetX(), startPosition.GetY(), DidItemMoveLastSimulation(uiSequenceIndex, lane, slot)};
    return true;
}
