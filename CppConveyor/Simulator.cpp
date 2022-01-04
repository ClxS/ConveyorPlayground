#include "Simulator.h"
#include "Producer.h"

cpp_conv::Conveyor::Channel* GetTargetChannel(cpp_conv::Conveyor& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel)
{
    if (sourceNode.m_direction == targetNode.m_direction)
    {
        return &targetNode.m_pChannels[iSourceChannel];
    }

    int mask = (int)sourceNode.m_direction | (int)targetNode.m_direction;
    if (mask == 0b11 || (mask >> 2) == 0b11)
    {
        // The nodes are facing either other, no valid path.
        return nullptr;
    }

    switch (sourceNode.m_direction)
    {
    case Direction::Up: return targetNode.m_direction == Direction::Left ? &targetNode.m_pChannels[1] : &targetNode.m_pChannels[0];
    case Direction::Down: return targetNode.m_direction == Direction::Left ? &targetNode.m_pChannels[0] : &targetNode.m_pChannels[1];
    case Direction::Left: return targetNode.m_direction == Direction::Up ? &targetNode.m_pChannels[0] : &targetNode.m_pChannels[1];
    case Direction::Right: return targetNode.m_direction == Direction::Up ? &targetNode.m_pChannels[1] : &targetNode.m_pChannels[0];
    }
}

int GetChannelTargetSlot(cpp_conv::Conveyor& sourceNode, cpp_conv::Conveyor& targetNode, int iSourceChannel)
{
    if (sourceNode.m_direction == targetNode.m_direction)
    {
        return cpp_conv::Conveyor::Channel::Slot::FirstSlot;
    }

    switch (sourceNode.m_direction)
    {
    case Direction::Up: return targetNode.m_direction == Direction::Left ? iSourceChannel : cpp_conv::c_conveyorChannels - 1 - iSourceChannel;
    case Direction::Down: return targetNode.m_direction == Direction::Left ? cpp_conv::c_conveyorChannels - 1 - iSourceChannel : iSourceChannel;
    case Direction::Left: return targetNode.m_direction == Direction::Up ? cpp_conv::c_conveyorChannels - 1 - iSourceChannel : iSourceChannel;
    case Direction::Right: return targetNode.m_direction == Direction::Up ? iSourceChannel : cpp_conv::c_conveyorChannels - 1 - iSourceChannel;
    }
}

void cpp_conv::simulation::simulate(
    cpp_conv::grid::EntityGrid& grid,
    std::vector<cpp_conv::Sequence>& sequences,
    std::vector<cpp_conv::Conveyor*>& conveyors,
    std::vector<cpp_conv::Producer*>& producer)
{
    for (auto& sequence : sequences)
    {
        std::vector<cpp_conv::Conveyor*> sequencePoints;
        for (auto& rNode : sequence.IterateSequence(grid))
        {
            sequencePoints.push_back(&rNode);
        }

        std::reverse(sequencePoints.begin(), sequencePoints.end());
        for (cpp_conv::Conveyor* rNode : sequencePoints)
        {
            cpp_conv::Entity* pForwardEntity = cpp_conv::grid::SafeGetEntity(grid, cpp_conv::grid::GetForwardPosition(*rNode));
            if (!cpp_conv::grid::IsConveyor(pForwardEntity))
            {
                continue;
            }

            cpp_conv::Conveyor* pForwardNode = reinterpret_cast<cpp_conv::Conveyor*>(pForwardEntity);
            for (int iChannelIdx = 0; iChannelIdx < cpp_conv::c_conveyorChannels; iChannelIdx++)
            {
                // Move the head slot items to the following conveyor
                Item*& frontMostItem = rNode->m_pChannels[iChannelIdx].m_pItems[Conveyor::Channel::Slot::LastSlot];
                if (frontMostItem != nullptr)
                {                    
                    if (rNode == sequence.GetHeadConveyor())
                    {
                        cpp_conv::Conveyor::Channel* pTargetChannel = GetTargetChannel(*rNode, *pForwardNode, iChannelIdx);
                        if (!pTargetChannel)
                        {
                            continue;
                        }

                        int forwardTargetItemSlot = GetChannelTargetSlot(*rNode, *pForwardNode, iChannelIdx);

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
                            if (IsCircular(grid, sequences, &sequence))
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

    for (cpp_conv::Conveyor* pConveyor : conveyors)
    {
        for (cpp_conv::Conveyor::Channel& channel : pConveyor->m_pChannels)
        {
            for (int iChannelSlot = Conveyor::Channel::Slot::FirstSlot; iChannelSlot <= Conveyor::Channel::Slot::LastSlot; iChannelSlot++)
            {
                if (channel.m_pPendingItems[iChannelSlot])
                {
                    channel.m_pItems[iChannelSlot] = channel.m_pPendingItems[iChannelSlot];
                    channel.m_pPendingItems[iChannelSlot] = nullptr;
                }
            }
        }
    }

    for (cpp_conv::Producer* pProducer : producer)
    {
        pProducer->Tick();
        if (!pProducer->IsReadyToProduce())
        {
            continue;
        }
     
        Item* pItem = pProducer->ProduceItem();
        if (!pItem)
        {
            continue;
        }

        cpp_conv::Entity* pForwardEntity = cpp_conv::grid::SafeGetEntity(grid, cpp_conv::grid::GetForwardPosition(*pProducer, pProducer->GetDirection()));
        if (!pForwardEntity || pForwardEntity->m_eEntityKind != EntityKind::Conveyor)
        {
            continue;
        }

        cpp_conv::Conveyor* pConveyor = reinterpret_cast<cpp_conv::Conveyor*>(pForwardEntity);
        Item*& forwardTargetItem = pConveyor->m_pChannels[rand() % cpp_conv::c_conveyorChannels].m_pItems[0];
        Item*& forwardPendingItem = pConveyor->m_pChannels[rand() % cpp_conv::c_conveyorChannels].m_pItems[0];
        if (!forwardTargetItem && !forwardPendingItem)
        {
            forwardPendingItem = pItem;
        }
    }
}
