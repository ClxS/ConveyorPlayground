#pragma once

#include <vector>
#include "vector_set.h"
#include "Conveyor.h"
#include "EntityGrid.h"

constexpr int c_maxCircularCheckDepth = 64;
namespace cpp_conv
{
    class Sequence;
    struct SceneContext;

    class SequenceIterator
    {
    public:
        class IterateNode
        {
        public:
            IterateNode(Conveyor* pCurrent, const Conveyor* pEnd, grid::EntityGrid& grid, bool bEndSentinel);

            Conveyor& operator*() const { return *m_pCurrent; }
            Conveyor* operator->() { return m_pCurrent; }

            IterateNode& operator++();
            IterateNode operator++(int);

            friend bool operator== (const IterateNode& a, const IterateNode& b) { return a.m_pCurrent == b.m_pCurrent && a.m_bEndSentinel == b.m_bEndSentinel; };
            friend bool operator!= (const IterateNode& a, const IterateNode& b) { return a.m_pCurrent != b.m_pCurrent || a.m_bEndSentinel != b.m_bEndSentinel; };

        private:
            Conveyor* m_pCurrent;
            const Conveyor* m_pEnd;
            grid::EntityGrid& m_grid;
            bool m_bEndSentinel;
        };


        SequenceIterator(Sequence& sequence, grid::EntityGrid& grid);
        IterateNode begin();
        IterateNode end();

    private:
        Sequence& m_sequence;
        grid::EntityGrid& m_grid;
    };

    class Sequence
    {
    public:
        Sequence(Conveyor* pHead, Conveyor* pTail, int iSequenceId)
            : m_pHeadConveyor(pHead)
            , m_pTailConveyor(pTail)
            , m_iSequenceId(iSequenceId)
        {
        }

        void Tick(SceneContext& kContext);

        const Conveyor* GetHeadConveyor() const { return m_pHeadConveyor; }
        const Conveyor* GetTailConveyor() const { return m_pTailConveyor; }

        Conveyor* GetHeadConveyor() { return m_pHeadConveyor; }
        Conveyor* GetTailConveyor() { return m_pTailConveyor; }

        SequenceIterator IterateSequence(grid::EntityGrid& grid)
        {
            return SequenceIterator(*this, grid);
        }

        int GetSequenceId() const
        {
            return m_iSequenceId;
        }

    private:
        Conveyor* m_pHeadConveyor;
        Conveyor* m_pTailConveyor;
        int m_iSequenceId;
    };

    Conveyor* TraceHeadConveyor(const grid::EntityGrid& grid, const Conveyor& searchStart);
    Conveyor* TraceTailConveyor(const grid::EntityGrid& grid, Conveyor& searchStart, Conveyor& head);

    std::vector<Sequence> InitializeSequences(grid::EntityGrid& grid, std::vector<Conveyor*>& conveyors);

    bool IsCircular(const grid::EntityGrid& grid, std::vector<Sequence>& sequences, Sequence* pStartSequence);

    std::tuple<int, Direction> GetInnerMostCornerChannel(const grid::EntityGrid& grid, const Conveyor& rConveyor);
}
