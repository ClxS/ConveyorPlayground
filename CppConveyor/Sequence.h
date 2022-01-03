#pragma once

#include "Conveyor.h"
#include "Grid.h"

constexpr int c_maxCircularCheckDepth = 64;
namespace cpp_conv
{
    class SequenceIterator
    {
    public:
        class IterateNode
        {
        public:
            IterateNode(Conveyor* pCurrent, const Conveyor* pEnd, grid::EntityGrid& grid, bool bEndSentinel)
                : m_pCurrent(pCurrent)
                , m_pEnd(pEnd)
                , m_grid(grid)
                , m_bEndSentinel(bEndSentinel)
            {
            }

            Conveyor& operator*() const { return *m_pCurrent; }
            Conveyor* operator->() { return m_pCurrent; }

            // Prefix increment
            IterateNode& operator++()
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

            // Postfix increment
            IterateNode operator++(int)
            {
                IterateNode tmp = *this; ++(*this);
                return tmp;
            }

            friend bool operator== (const IterateNode& a, const IterateNode& b) { return a.m_pCurrent == b.m_pCurrent && a.m_bEndSentinel == b.m_bEndSentinel; };
            friend bool operator!= (const IterateNode& a, const IterateNode& b) { return a.m_pCurrent != b.m_pCurrent || a.m_bEndSentinel != b.m_bEndSentinel; };

        private:
            Conveyor* m_pCurrent;
            const Conveyor* m_pEnd;
            grid::EntityGrid& m_grid;
            bool m_bEndSentinel;
        };


        SequenceIterator(Sequence& sequence, grid::EntityGrid& grid)
            : m_sequence(sequence)
            , m_grid(grid)
        {
        }

        IterateNode begin()
        {
            return { m_sequence.GetTailConveyor(), m_sequence.GetHeadConveyor(), m_grid, false };
        }

        IterateNode end()
        {
            return { m_sequence.GetHeadConveyor(), m_sequence.GetHeadConveyor(), m_grid, true };
        }


    private:
        Sequence& m_sequence;
        grid::EntityGrid& m_grid;
    };

    class Sequence
    {
    public:
        Sequence(Conveyor* pHead, Conveyor* pTail)
            : m_pHeadConveyor(pHead)
            , m_pTailConveyor(pTail)
        {
        }

        const Conveyor* GetHeadConveyor() const { return m_pHeadConveyor; }
        const Conveyor* GetTailConveyor() const { return m_pTailConveyor; }

        Conveyor* GetHeadConveyor() { return m_pHeadConveyor; }
        Conveyor* GetTailConveyor() { return m_pTailConveyor; }

        SequenceIterator IterateSequence(grid::EntityGrid& grid)
        {
            return SequenceIterator(this, grid);
        }

    private:
        Conveyor* m_pHeadConveyor;
        Conveyor* m_pTailConveyor;
    };

    Conveyor* TraceHeadConveyor(const grid::EntityGrid& grid, const Conveyor& searchStart)
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

    Conveyor* TraceTailConveyor(grid::EntityGrid& grid, Conveyor& searchStart, Conveyor& head)
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


    std::vector<Sequence> InitializeSequences(grid::EntityGrid& grid, std::vector<Conveyor*>& conveyors)
    {
        std::vector<Sequence> vSequences;
        cpp_conveyor::vector_set<const Conveyor*> alreadyProcessedConveyors(conveyors.size());
        for (auto& conveyor : conveyors)
        {
            if (alreadyProcessedConveyors.contains(conveyor))
            {
                continue;
            }

            Conveyor* pHeadConveyor = TraceHeadConveyor(grid, *conveyor);
            Conveyor* pTailConveyor = TraceTailConveyor(grid, *conveyor, *pHeadConveyor);

            vSequences.emplace_back(pHeadConveyor, pTailConveyor);
            for (auto& rNode : vSequences.back().IterateSequence(grid))
            {
                alreadyProcessedConveyors.insert(&rNode);
            }
        }

        return vSequences;
    }

    bool IsCircular(grid::EntityGrid& grid, std::vector<Sequence>& sequences, Sequence* pStartSequence)
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
}
