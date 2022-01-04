#include "Sequence.h"

#include "TargetingUtility.h"
#include "Junction.h"
#include "Underground.h"
#include "SceneContext.h"

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
        m_pCurrent = reinterpret_cast<Conveyor*>(grid::SafeGetEntity(m_grid, grid::GetForwardPosition(*m_pCurrent)));
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

    Conveyor* pCurrentConveyor = reinterpret_cast<Conveyor*>(grid::SafeGetEntity(grid, searchStart.m_position));
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
        vPotentialNeighbours[(int)RelativeDirection::Backwards] = grid::SafeGetEntity(grid, grid::GetBackwardsPosition(*pTargetConveyor));
        vPotentialNeighbours[(int)RelativeDirection::Right] = grid::SafeGetEntity(grid, grid::GetRightPosition(*pTargetConveyor));
        vPotentialNeighbours[(int)RelativeDirection::Left] = grid::SafeGetEntity(grid, grid::GetLeftPosition(*pTargetConveyor));

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

Conveyor* cpp_conv::TraceTailConveyor(grid::EntityGrid& grid, Conveyor& searchStart, Conveyor& head)
{
    static RelativeDirection directionPriority[] =
    {
        RelativeDirection::Backwards,
        RelativeDirection::Right,
        RelativeDirection::Left,
    };

    Conveyor* pCurrentConveyor = reinterpret_cast<Conveyor*>(grid::SafeGetEntity(grid, searchStart.m_position));
    while (true)
    {
        Entity* vPotentialNeighbours[4];
        vPotentialNeighbours[(int)RelativeDirection::Backwards] = grid::SafeGetEntity(grid, grid::GetBackwardsPosition(*pCurrentConveyor));
        vPotentialNeighbours[(int)RelativeDirection::Right] = grid::SafeGetEntity(grid, grid::GetRightPosition(*pCurrentConveyor));
        vPotentialNeighbours[(int)RelativeDirection::Left] = grid::SafeGetEntity(grid, grid::GetLeftPosition(*pCurrentConveyor));

        Conveyor* pTargetConveyor = nullptr;
        for (auto direction : directionPriority)
        {
            Entity* pDirectionEntity = vPotentialNeighbours[(int)direction];
            if (pDirectionEntity == nullptr || pDirectionEntity->m_eEntityKind != EntityKind::Conveyor)
            {
                continue;
            }

            Conveyor* pDirectionConveyor = reinterpret_cast<Conveyor*>(pDirectionEntity);
            if (grid::GetForwardPosition(*pDirectionConveyor) == pCurrentConveyor->m_position)
            {
                pTargetConveyor = pDirectionConveyor;
                break;
            }
        }

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

bool cpp_conv::IsCircular(grid::EntityGrid& grid, std::vector<Sequence>& sequences, Sequence* pStartSequence)
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
        for (int iChannelIdx = 0; iChannelIdx < cpp_conv::c_conveyorChannels; iChannelIdx++)
        {
            cpp_conv::Entity* pForwardEntity = cpp_conv::grid::SafeGetEntity(kContext.m_grid, cpp_conv::grid::GetForwardPosition(*rNode));
            Item*& frontMostItem = rNode->m_pChannels[iChannelIdx].m_pItems[Conveyor::Channel::Slot::LastSlot];
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

                        int forwardTargetItemSlot = cpp_conv::targeting_util::GetChannelTargetSlot(*rNode, *pForwardNode, iChannelIdx);

                        Item*& forwardTargetItem = pTargetChannel->m_pItems[forwardTargetItemSlot];
                        Item*& forwardPendingItem = pTargetChannel->m_pPendingItems[forwardTargetItemSlot];

                        // Following node is empty, we can just move there
                        if (!forwardTargetItem && !forwardPendingItem)
                        {
                            forwardPendingItem = frontMostItem;
                            frontMostItem = nullptr;
                        }
                        else if (!pForwardNode->m_pChannels[iChannelIdx].m_pPendingItems[Conveyor::Channel::Slot::FirstSlot])
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
                        Item*& forwardTargetItem = pForwardNode->m_pChannels[iChannelIdx].m_pItems[Conveyor::Channel::Slot::FirstSlot];
                        Item*& forwardPendingItem = pForwardNode->m_pChannels[iChannelIdx].m_pPendingItems[Conveyor::Channel::Slot::FirstSlot];
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
            for (int iChannelSlot = Conveyor::Channel::Slot::LastSlot - 1; iChannelSlot >= Conveyor::Channel::Slot::FirstSlot; iChannelSlot--)
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
