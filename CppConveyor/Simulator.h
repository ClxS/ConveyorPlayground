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
                    if (rNode == sequence.GetHeadConveyor())
                    {
                        // Following node is empty, we can just move there
                        if (rNode->m_pItem && !pForwardNode->m_pItem && !pForwardNode->m_pPendingItem)
                        {
                            pForwardNode->m_pPendingItem = rNode->m_pItem;
                            rNode->m_pItem = nullptr;
                        }
                        else if (!pForwardNode->m_pPendingItem)
                        {
                            // Otherwise we need to check if we're in a circular segment
                            if (IsCircular(grid, sequences, &sequence))
                            {
                                pForwardNode->m_pPendingItem = rNode->m_pItem;
                                rNode->m_pItem = nullptr;
                            }
                        }
                    }
                    else
                    {
                        if (rNode->m_pItem && !pForwardNode->m_pItem && !pForwardNode->m_pPendingItem)
                        {
                            pForwardNode->m_pPendingItem = rNode->m_pItem;
                            rNode->m_pItem = nullptr;
                        }
                    }
                }
            }

            for (cpp_conv::Conveyor* pConveyor : conveyors)
            {
                if (pConveyor->m_pPendingItem)
                {
                    pConveyor->m_pItem = pConveyor->m_pPendingItem;
                    pConveyor->m_pPendingItem = nullptr;
                }
            }
        }
    }
}