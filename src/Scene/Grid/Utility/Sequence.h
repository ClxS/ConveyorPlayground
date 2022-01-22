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

    class Sequence
    {
    public:
        Sequence(const Conveyor* pHead, uint8_t uiLength)
            : m_Lanes{ 0, 0 }
            , m_pHeadConveyor(pHead)
            , m_Length(uiLength)
        {
        }

        void Tick(SceneContext& kContext);

        const Conveyor* GetHeadConveyor() const { return m_pHeadConveyor; }

        inline static constexpr uint32_t c_uiMaxSequenceLength = 32;
        bool HasItemInSlot(uint8_t m_uiSequenceIndex, int lane, int slot) const;
        void AddItemInSlot(uint8_t m_uiSequenceIndex, int lane, int slot);
        bool TryPeakItemInSlot(uint8_t m_uiSequenceIndex, int lane, int slot, const ItemInstance*& pItem);
        uint64_t CountItemsOnBelt();
    private:
        friend class Conveyor;

        std::array<uint64_t, c_conveyorChannels> m_Lanes;

        const Conveyor* m_pHeadConveyor;
        const uint8_t m_Length;
    };

    Conveyor* TraceHeadConveyor(WorldMap& map, Conveyor& searchStart);
    const Conveyor* TraceTailConveyor(WorldMap& map, Conveyor& searchStart, Conveyor& head, std::vector<Conveyor*>& vOutConveyors);

    std::vector<Sequence*> InitializeSequences(WorldMap& map, const std::vector<Conveyor*>& conveyors);

    std::tuple<int, Direction> GetInnerMostCornerChannel(const cpp_conv::WorldMap& map, const Conveyor& rConveyor);
}
