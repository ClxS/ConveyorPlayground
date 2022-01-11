#include "Sequence.h"

#include "TargetingUtility.h"
#include "Junction.h"
#include "Underground.h"
#include "SceneContext.h"
#include "Direction.h"
#include "Profiler.h"
#include "WorldMap.h"
#include <cassert>

using namespace cpp_conv;
using cpp_conv::SequenceIterator;

SequenceIterator::IterateNode::IterateNode(Conveyor* pCurrent, const Conveyor* pEnd, cpp_conv::WorldMap& map, bool bEndSentinel)
    : m_pCurrent(pCurrent)
    , m_pEnd(pEnd)
    , m_rMap(map)
    , m_bEndSentinel(bEndSentinel)
{
}

SequenceIterator::IterateNode& SequenceIterator::IterateNode::operator++()
{
    if (m_pCurrent == m_pEnd)
    {
        m_bEndSentinel = true;
    }
    else
    {
        m_pCurrent = m_rMap.GetEntity<Conveyor>(grid::GetForwardPosition(*m_pCurrent), EntityKind::Conveyor);
    }

    return *this;
}

SequenceIterator::IterateNode SequenceIterator::IterateNode::operator++(int)
{
    IterateNode tmp = *this; ++(*this);
    return tmp;
}

SequenceIterator::SequenceIterator(Sequence& sequence, cpp_conv::WorldMap& map)
    : m_sequence(sequence)
    , m_rMap(map)
{
}

SequenceIterator::IterateNode SequenceIterator::begin()
{
    return { const_cast<Conveyor*>(m_sequence.GetTailConveyor()), m_sequence.GetHeadConveyor(), m_rMap, false };
}

SequenceIterator::IterateNode SequenceIterator::end()
{
    return { const_cast<Conveyor*>(m_sequence.GetHeadConveyor()), m_sequence.GetHeadConveyor(), m_rMap, true };
}

const Conveyor* cpp_conv::TraceHeadConveyor(const cpp_conv::WorldMap& map, const Conveyor& searchStart)
{
    static RelativeDirection directionPriority[] =
    {
        RelativeDirection::Backwards,
        RelativeDirection::Right,
        RelativeDirection::Left,
    };

    const Conveyor* pCurrentConveyor = map.GetEntity<Conveyor>(searchStart.m_position, EntityKind::Conveyor);
    while (true)
    {
        Position forwardPosition = grid::GetForwardPosition(*pCurrentConveyor);
        const Entity* pTargetConveyor = map.GetEntity<Conveyor>(forwardPosition, EntityKind::Conveyor);
        if (pTargetConveyor == nullptr)
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
                break;
            }

            if (grid::GetForwardPosition(*pDirectionEntity) == forwardPosition)
            {
                return pCurrentConveyor;
            }
        }

        if (pTargetConveyor == &searchStart)
        {
            break;
        }

        pCurrentConveyor = reinterpret_cast<const Conveyor*>(pTargetConveyor);
    }

    return pCurrentConveyor;
}

const Conveyor* cpp_conv::TraceTailConveyor(const cpp_conv::WorldMap& map, const Conveyor& searchStart, const Conveyor& head)
{
    const Conveyor* pCurrentConveyor = map.GetEntity<Conveyor>(searchStart.m_position, EntityKind::Conveyor);

    assert(pCurrentConveyor != nullptr); // If it is null here, we are misusing this method.

    while (true)
    {
        const Conveyor* pTargetConveyor = cpp_conv::targeting_util::FindNextTailConveyor(map, *pCurrentConveyor);
        if (!pTargetConveyor || pTargetConveyor == &searchStart || pTargetConveyor == &head)
        {
            break;
        }

        pCurrentConveyor = pTargetConveyor;
    }

    return pCurrentConveyor;
}

std::vector<Sequence> cpp_conv::InitializeSequences(cpp_conv::WorldMap& map, const std::vector<Conveyor*>& conveyors)
{
    std::vector<Sequence> vSequences;
    cpp_conveyor::vector_set<const Conveyor*> alreadyProcessedConveyors(conveyors.size());

    int iId = 0;
    for (auto& conveyor : conveyors)
    {
        if (alreadyProcessedConveyors.contains(conveyor))
        {
            continue;
        }

        iId++;
        const Conveyor* pHeadConveyor = TraceHeadConveyor(map, *conveyor);
        const Conveyor* pTailConveyor = TraceTailConveyor(map, *conveyor, *pHeadConveyor);

        vSequences.emplace_back(pHeadConveyor, pTailConveyor, iId);
        Sequence& sequence = vSequences.back();
        for (auto& rNode : sequence.IterateSequence(map))
        {
            rNode.m_pSequenceId = iId;
            alreadyProcessedConveyors.insert(&rNode);
        }
    }

    return vSequences;
}

bool cpp_conv::IsCircular(const cpp_conv::WorldMap& map, std::vector<Sequence>& sequences, Sequence* pStartSequence)
{
    Sequence* pCurrentSequence = pStartSequence;
    int checkedDepth = 0;
    while (true)
    {
        checkedDepth++;
        if (checkedDepth > c_maxCircularCheckDepth)
        {
            return false;
        }

        const Conveyor* pHead = pCurrentSequence->GetHeadConveyor();
        const Entity* pForwardEntity = map.GetEntity(grid::GetForwardPosition(*pHead));
        if (pForwardEntity->m_eEntityKind != EntityKind::Conveyor)
        {
            return false;
        }

        Sequence* pNextSequence = nullptr;
        for (auto& sequence : sequences)
        {
            if (sequence.GetTailConveyor() == pForwardEntity)
            {
                pNextSequence = &sequence;
                break;
            }
        }

        if (pNextSequence == nullptr)
        {
            return false;
        }

        if (pNextSequence == pStartSequence)
        {
            return true;
        }
    }

    return false;
}

std::tuple<int, Direction> cpp_conv::GetInnerMostCornerChannel(const cpp_conv::WorldMap& map, const Conveyor& rConveyor)
{
    PROFILE_FUNC();
    const Conveyor* pBackConverter = cpp_conv::targeting_util::FindNextTailConveyor(map, rConveyor);
    if (pBackConverter == nullptr || pBackConverter->m_direction == rConveyor.m_direction)
    {
        return std::make_tuple(-1, Direction::Up);
    }

    Direction selfDirection = rConveyor.m_direction;
    Direction backDirection = pBackConverter->m_direction;
    while (selfDirection != Direction::Up)
    {
        selfDirection = cpp_conv::direction::Rotate90DegreeClockwise(selfDirection);
        backDirection = cpp_conv::direction::Rotate90DegreeClockwise(backDirection);
    }

    return std::make_tuple(backDirection == Direction::Right ? 1 : 0, pBackConverter->m_direction);
}

void cpp_conv::Sequence::Tick(SceneContext& kContext)
{
    std::vector<cpp_conv::Conveyor*> sequencePoints;
    for (auto& rNode : IterateSequence(kContext.m_rMap))
    {
        sequencePoints.push_back(&rNode);
    }

    std::reverse(sequencePoints.begin(), sequencePoints.end());
    for (cpp_conv::Conveyor* rNode : sequencePoints)
    {
        bool bIsCornerConveyor = cpp_conv::targeting_util::IsCornerConveyor(kContext.m_rMap, *rNode);
        int iInnerMostChannel;
        Direction eCornerDirection;
        std::tie(iInnerMostChannel, eCornerDirection) = GetInnerMostCornerChannel(kContext.m_rMap, *rNode);

        for (int iChannelIdx = 0; iChannelIdx < cpp_conv::c_conveyorChannels; iChannelIdx++)
        {
            int iChannelLength = cpp_conv::c_conveyorChannelSlots;
            if (bIsCornerConveyor)
            {
                iChannelLength += iInnerMostChannel == iChannelIdx ? -1 : 1;
            }

            cpp_conv::Entity* pForwardEntity = kContext.m_rMap.GetEntity(cpp_conv::grid::GetForwardPosition(*rNode));
            ItemId& frontMostItem = rNode->m_pChannels[iChannelIdx].m_pItems[iChannelLength - 1];
            if (!frontMostItem.IsEmpty())
            {
                if (rNode == GetHeadConveyor())
                {
                    if (pForwardEntity && pForwardEntity->SupportsInsertion() && pForwardEntity->TryInsert(kContext, *rNode, frontMostItem, iChannelIdx))
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
                ItemId& currentItem = rNode->m_pChannels[iChannelIdx].m_pItems[iChannelSlot];
                ItemId& forwardTargetItem = rNode->m_pChannels[iChannelIdx].m_pItems[iChannelSlot + 1];
                ItemId& forwardPendingItem = rNode->m_pChannels[iChannelIdx].m_pItems[iChannelSlot + 1];

                if (currentItem.IsValid() && forwardTargetItem.IsEmpty() && forwardPendingItem.IsEmpty())
                {
                    forwardPendingItem = currentItem;
                    currentItem = cpp_conv::ItemIds::None;
                }
            }
        }
    }
}
