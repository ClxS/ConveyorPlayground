#include "Sequence.h"

#include "TargetingUtility.h"
#include "Junction.h"
#include "Underground.h"
#include "SceneContext.h"
#include "Direction.h"
#include <cassert>
#include "Profiler.h"

using namespace cpp_conv;
using cpp_conv::SequenceIterator;

SequenceIterator::IterateNode::IterateNode(Conveyor* pCurrent, const Conveyor* pEnd, grid::EntityGrid& grid, bool bEndSentinel)
    : m_pCurrent(pCurrent)
    , m_pEnd(pEnd)
    , m_grid(grid)
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
        m_pCurrent = grid::SafeGetEntity<Conveyor>(m_grid, grid::GetForwardPosition(*m_pCurrent), EntityKind::Conveyor);
    }

    return *this;
}

SequenceIterator::IterateNode SequenceIterator::IterateNode::operator++(int)
{
    IterateNode tmp = *this; ++(*this);
    return tmp;
}

SequenceIterator::SequenceIterator(Sequence& sequence, grid::EntityGrid& grid)
    : m_sequence(sequence)
    , m_grid(grid)
{
}

SequenceIterator::IterateNode SequenceIterator::begin()
{
    return { m_sequence.GetTailConveyor(), m_sequence.GetHeadConveyor(), m_grid, false };
}

SequenceIterator::IterateNode SequenceIterator::end()
{
    return { m_sequence.GetHeadConveyor(), m_sequence.GetHeadConveyor(), m_grid, true };
}

Conveyor* cpp_conv::TraceHeadConveyor(const grid::EntityGrid& grid, const Conveyor& searchStart)
{
    static RelativeDirection directionPriority[] =
    {
        RelativeDirection::Backwards,
        RelativeDirection::Right,
        RelativeDirection::Left,
    };

    Conveyor* pCurrentConveyor = grid::SafeGetEntity<Conveyor>(grid, searchStart.m_position, EntityKind::Conveyor);
    while (true)
    {
        Position forwardPosition = grid::GetForwardPosition(*pCurrentConveyor);
        Entity* pTarget = grid::SafeGetEntity(grid, forwardPosition);
        if (pTarget == nullptr || pTarget->m_eEntityKind != EntityKind::Conveyor)
        {
            break;
        }

        Conveyor* pTargetConveyor = reinterpret_cast<Conveyor*>(pTarget);

        Entity* vPotentialNeighbours[4];
        vPotentialNeighbours[(int)RelativeDirection::Backwards] = grid::SafeGetEntity<Conveyor>(grid, grid::GetBackwardsPosition(*pTargetConveyor), EntityKind::Conveyor);
        vPotentialNeighbours[(int)RelativeDirection::Right] = grid::SafeGetEntity<Conveyor>(grid, grid::GetRightPosition(*pTargetConveyor), EntityKind::Conveyor);
        vPotentialNeighbours[(int)RelativeDirection::Left] = grid::SafeGetEntity<Conveyor>(grid, grid::GetLeftPosition(*pTargetConveyor), EntityKind::Conveyor);

        for (auto direction : directionPriority)
        {
            Entity* pDirectionEntity = vPotentialNeighbours[(int)direction];
            if (pDirectionEntity == nullptr || pDirectionEntity->m_eEntityKind != EntityKind::Conveyor)
            {
                continue;
            }

            if (pDirectionEntity == pCurrentConveyor)
            {
                break;
            }
            else if (grid::GetForwardPosition(*reinterpret_cast<Conveyor*>(pDirectionEntity)) == forwardPosition)
            {
                return pCurrentConveyor;
            }
        }

        if (pTargetConveyor == &searchStart)
        {
            break;
        }

        pCurrentConveyor = pTargetConveyor;
    }

    return pCurrentConveyor;
}

Conveyor* FindNextTailConveyor(const grid::EntityGrid& grid, const Conveyor& rCurrentConveyor)
{
    static RelativeDirection directionPriority[] =
    {
        RelativeDirection::Backwards,
        RelativeDirection::Right,
        RelativeDirection::Left,
    };

    Conveyor* vPotentialNeighbours[4];
    vPotentialNeighbours[(int)RelativeDirection::Backwards] = grid::SafeGetEntity<Conveyor>(grid, grid::GetBackwardsPosition(rCurrentConveyor), EntityKind::Conveyor);
    vPotentialNeighbours[(int)RelativeDirection::Right] = grid::SafeGetEntity<Conveyor>(grid, grid::GetRightPosition(rCurrentConveyor), EntityKind::Conveyor);
    vPotentialNeighbours[(int)RelativeDirection::Left] = grid::SafeGetEntity<Conveyor>(grid, grid::GetLeftPosition(rCurrentConveyor), EntityKind::Conveyor);

    Conveyor* pTargetConveyor = nullptr;
    for (auto direction : directionPriority)
    {
        Conveyor* pDirectionEntity = vPotentialNeighbours[(int)direction];
        if (pDirectionEntity == nullptr || pDirectionEntity->m_eEntityKind != EntityKind::Conveyor)
        {
            continue;
        }

        Conveyor* pDirectionConveyor = reinterpret_cast<Conveyor*>(pDirectionEntity);
        if (grid::GetForwardPosition(*pDirectionConveyor) == rCurrentConveyor.m_position)
        {
            pTargetConveyor = pDirectionConveyor;
            break;
        }
    }

    return pTargetConveyor;
}

Conveyor* cpp_conv::TraceTailConveyor(const grid::EntityGrid& grid, Conveyor& searchStart, Conveyor& head)
{
    Conveyor* pCurrentConveyor = reinterpret_cast<Conveyor*>(grid::SafeGetEntity(grid, searchStart.m_position));

    assert(pCurrentConveyor != nullptr); // If it is null here, we are misusing this method.

    while (true)
    {
        Conveyor* pTargetConveyor = FindNextTailConveyor(grid, *pCurrentConveyor);
        if (!pTargetConveyor || pTargetConveyor == &searchStart || pTargetConveyor == &head)
        {
            break;
        }

        pCurrentConveyor = pTargetConveyor;
    }

    return pCurrentConveyor;
}

std::vector<Sequence> cpp_conv::InitializeSequences(grid::EntityGrid& grid, std::vector<Conveyor*>& conveyors)
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
        Conveyor* pHeadConveyor = TraceHeadConveyor(grid, *conveyor);
        Conveyor* pTailConveyor = TraceTailConveyor(grid, *conveyor, *pHeadConveyor);

        vSequences.emplace_back(pHeadConveyor, pTailConveyor, iId);
        Sequence& sequence = vSequences.back();
        for (auto& rNode : sequence.IterateSequence(grid))
        {
            rNode.m_pSequenceId = iId;
            alreadyProcessedConveyors.insert(&rNode);
        }
    }

    return vSequences;
}

bool cpp_conv::IsCircular(const grid::EntityGrid& grid, std::vector<Sequence>& sequences, Sequence* pStartSequence)
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

        Conveyor* pHead = pCurrentSequence->GetHeadConveyor();
        Entity* pForwardEntity = grid::SafeGetEntity(grid, grid::GetForwardPosition(*pHead));
        if (!grid::IsConveyor(pForwardEntity))
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

bool cpp_conv::IsCornerConveyor(const grid::EntityGrid& grid, const Conveyor& rConveyor)
{
    PROFILE_FUNC();
    Conveyor* pBackConverter = FindNextTailConveyor(grid, rConveyor);
    if (pBackConverter == nullptr)
    {
        return false;
    }

    return pBackConverter->m_direction != rConveyor.m_direction;
}

bool cpp_conv::IsClockwiseCorner(const grid::EntityGrid& grid, const Conveyor& rConveyor)
{
	PROFILE_FUNC();
	Conveyor* pBackConverter = FindNextTailConveyor(grid, rConveyor);
	if (pBackConverter == nullptr || pBackConverter->m_direction == rConveyor.m_direction)
	{
		return false;
	}

	Direction selfDirection = rConveyor.m_direction;
	Direction backDirection = pBackConverter->m_direction;
	while (selfDirection != Direction::Up)
	{
		selfDirection = cpp_conv::direction::Rotate90DegreeClockwise(selfDirection);
		backDirection = cpp_conv::direction::Rotate90DegreeClockwise(backDirection);
	}

	return backDirection == Direction::Right;
}

std::tuple<int, Direction> cpp_conv::GetInnerMostCornerChannel(const grid::EntityGrid& grid, const Conveyor& rConveyor)
{
	PROFILE_FUNC();
	Conveyor* pBackConverter = FindNextTailConveyor(grid, rConveyor);
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

void cpp_conv::Sequence::Tick(const SceneContext& kContext)
{
    std::vector<cpp_conv::Conveyor*> sequencePoints;
    for (auto& rNode : IterateSequence(kContext.m_grid))
    {
        sequencePoints.push_back(&rNode);
    }

    std::reverse(sequencePoints.begin(), sequencePoints.end());
    for (cpp_conv::Conveyor* rNode : sequencePoints)
    {
        bool bIsCornerConveyor = IsCornerConveyor(kContext.m_grid, *rNode);
        int iInnerMostChannel;
        Direction eCornerDirection;
        std::tie(iInnerMostChannel, eCornerDirection) = GetInnerMostCornerChannel(kContext.m_grid, *rNode);

        for (int iChannelIdx = 0; iChannelIdx < cpp_conv::c_conveyorChannels; iChannelIdx++)
        {
            int iChannelLength = cpp_conv::c_conveyorChannelSlots;
            if (bIsCornerConveyor)
            {
                iChannelLength += iInnerMostChannel == iChannelIdx ? -1 : 1;
            }

            cpp_conv::Entity* pForwardEntity = cpp_conv::grid::SafeGetEntity(kContext.m_grid, cpp_conv::grid::GetForwardPosition(*rNode));
            Item*& frontMostItem = rNode->m_pChannels[iChannelIdx].m_pItems[iChannelLength - 1];
            if (frontMostItem)
            {
                if (pForwardEntity && pForwardEntity->m_eEntityKind == EntityKind::Conveyor)
                {
                    cpp_conv::Conveyor* pForwardNode = reinterpret_cast<cpp_conv::Conveyor*>(pForwardEntity);

                    // Move the head slot items to the following conveyor
                    if (rNode == GetHeadConveyor())
                    {
                        cpp_conv::Conveyor::Channel* pTargetChannel = cpp_conv::targeting_util::GetTargetChannel(*rNode, *pForwardNode, iChannelIdx);
                        if (!pTargetChannel)
                        {
                            continue;
                        }

                        int forwardTargetItemSlot = cpp_conv::targeting_util::GetChannelTargetSlot(kContext.m_grid, *rNode, *pForwardNode, iChannelIdx);

                        Item*& forwardTargetItem = pTargetChannel->m_pItems[forwardTargetItemSlot];
                        Item*& forwardPendingItem = pTargetChannel->m_pPendingItems[forwardTargetItemSlot];

                        // Following node is empty, we can just move there
                        if (!forwardTargetItem && !forwardPendingItem)
                        {
                            forwardPendingItem = frontMostItem;
                            frontMostItem = nullptr;
                        }
                        else if (!pForwardNode->m_pChannels[iChannelIdx].m_pPendingItems[0])
                        {
                            // Otherwise we need to check if we're in a circular segment
                            if (IsCircular(kContext.m_grid, kContext.m_sequences, this))
                            {
                                forwardPendingItem = frontMostItem;
                                frontMostItem = nullptr;
                            }
                        }
                    }
                    else
                    {
                        Item*& forwardTargetItem = pForwardNode->m_pChannels[iChannelIdx].m_pItems[0];
                        Item*& forwardPendingItem = pForwardNode->m_pChannels[iChannelIdx].m_pPendingItems[0];
                        if (!forwardTargetItem && !forwardPendingItem)
                        {
                            forwardPendingItem = frontMostItem;
                            frontMostItem = nullptr;
                        }
                    }
                }
                else if (pForwardEntity && pForwardEntity->m_eEntityKind == EntityKind::Junction)
                {
                    cpp_conv::Junction* pJunction = reinterpret_cast<cpp_conv::Junction*>(pForwardEntity);
                    if (pJunction->AddItem(frontMostItem))
                    {
                        frontMostItem = nullptr;
                    }
                }
                else if (pForwardEntity && pForwardEntity->m_eEntityKind == EntityKind::Underground)
                {
                    cpp_conv::Underground* pUnderground = reinterpret_cast<cpp_conv::Underground*>(pForwardEntity);
                    if (pUnderground->AddItem(kContext.m_grid, frontMostItem, iChannelIdx))
                    {
                        frontMostItem = nullptr;
                    }
                }
            }

            // Move inner items forwards
            for (int iChannelSlot = iChannelLength - 2; iChannelSlot >= 0; iChannelSlot--)
            {
                Item*& currentItem = rNode->m_pChannels[iChannelIdx].m_pItems[iChannelSlot];
                Item*& forwardTargetItem = rNode->m_pChannels[iChannelIdx].m_pItems[iChannelSlot + 1];
                Item*& forwardPendingItem = rNode->m_pChannels[iChannelIdx].m_pItems[iChannelSlot + 1];

                if (currentItem && !forwardTargetItem && !forwardPendingItem)
                {
                    forwardPendingItem = currentItem;
                    currentItem = nullptr;
                }
            }
        }
    }
}
