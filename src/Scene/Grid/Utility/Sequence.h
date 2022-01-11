#pragma once

#include <vector>
#include "vector_set.h"
#include "Conveyor.h"
#include "EntityGrid.h"

namespace cpp_conv { class WorldMap; }

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
            IterateNode(Conveyor* pCurrent, const Conveyor* pEnd, cpp_conv::WorldMap& map, bool bEndSentinel);

            Conveyor& operator*() const { return *m_pCurrent; }
            Conveyor* operator->() { return m_pCurrent; }

            IterateNode& operator++();
            IterateNode operator++(int);

            friend bool operator== (const IterateNode& a, const IterateNode& b) { return a.m_pCurrent == b.m_pCurrent && a.m_bEndSentinel == b.m_bEndSentinel; };
            friend bool operator!= (const IterateNode& a, const IterateNode& b) { return a.m_pCurrent != b.m_pCurrent || a.m_bEndSentinel != b.m_bEndSentinel; };

        private:
            Conveyor* m_pCurrent;
            const Conveyor* m_pEnd;
            cpp_conv::WorldMap& m_rMap;
            bool m_bEndSentinel;
        };


        SequenceIterator(Sequence& sequence, cpp_conv::WorldMap& map);
        IterateNode begin();
        IterateNode end();

    private:
        Sequence& m_sequence;
        cpp_conv::WorldMap& m_rMap;
    };

    class Sequence
    {
    public:
        Sequence(const Conveyor* pHead, const Conveyor* pTail, int iSequenceId)
            : m_pHeadConveyor(pHead)
            , m_pTailConveyor(pTail)
            , m_iSequenceId(iSequenceId)
        {
        }

        void Tick(SceneContext& kContext);

        const Conveyor* GetHeadConveyor() const { return m_pHeadConveyor; }
        const Conveyor* GetTailConveyor() const { return m_pTailConveyor; }

        SequenceIterator IterateSequence(cpp_conv::WorldMap& map)
        {
            return SequenceIterator(*this, map);
        }

        int GetSequenceId() const
        {
            return m_iSequenceId;
        }

    private:
        const Conveyor* m_pHeadConveyor;
        const Conveyor* m_pTailConveyor;
        int m_iSequenceId;
    };

    const Conveyor* TraceHeadConveyor(const WorldMap& map, const Conveyor& searchStart);
    const Conveyor* TraceTailConveyor(const WorldMap& map, const Conveyor& searchStart, const Conveyor& head);

    std::vector<Sequence> InitializeSequences(WorldMap& map, const std::vector<Conveyor*>& conveyors);

    bool IsCircular(const cpp_conv::WorldMap& map, std::vector<Sequence>& sequences, Sequence* pStartSequence);

    std::tuple<int, Direction> GetInnerMostCornerChannel(const cpp_conv::WorldMap& map, const Conveyor& rConveyor);
}
