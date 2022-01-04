#pragma once

#include <vector>
#include "Grid.h"
#include "Sequence.h"
#include "Conveyor.h"

namespace cpp_conv
{
    namespace simulation
    {
        void Simulate(cpp_conv::grid::EntityGrid& grid, std::vector<cpp_conv::Sequence>& sequences, std::vector<cpp_conv::Conveyor*>& conveyors)
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
                        Item*& currentItem = rNode->m_pChannels[iChannelIdx].m_pItems[Conveyor::Channel::Slot::LastSlot];
                        Item*& forwardTargetItem = pForwardNode->m_pChannels[iChannelIdx].m_pItems[Conveyor::Channel::Slot::FirstSlot];
                        Item*& forwardPendingItem = pForwardNode->m_pChannels[iChannelIdx].m_pItems[Conveyor::Channel::Slot::FirstSlot];

                        if (rNode == sequence.GetHeadConveyor())
                        {
                            // Following node is empty, we can just move there
                            if (currentItem &&
                                !forwardTargetItem &&
                                !forwardPendingItem)
                            {
                                forwardPendingItem = currentItem;
                                currentItem = nullptr;
                            }
                            else if (!pForwardNode->m_pChannels[iChannelIdx].m_pPendingItems[Conveyor::Channel::Slot::FirstSlot])
                            {
                                // Otherwise we need to check if we're in a circular segment
                                if (IsCircular(grid, sequences, &sequence))
                                {
                                    forwardPendingItem = currentItem;
                                    currentItem = nullptr;
                                }
                            }
                        }
                        else
                        {
                            if (currentItem && !forwardTargetItem && !forwardPendingItem)
                            {
                                forwardPendingItem = currentItem;
                                currentItem = nullptr;
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
        }
    }
}