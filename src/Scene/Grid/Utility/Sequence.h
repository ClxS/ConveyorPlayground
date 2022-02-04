#pragma once

#include <vector>
#include "vector_set.h"
#include "Conveyor.h"
#include "EntityGrid.h"
#include "FixedCircularBuffer.h"

namespace cpp_conv { class WorldMap; }

constexpr int c_maxCircularCheckDepth = 64;
namespace cpp_conv
{
    class Sequence;
    struct SceneContext;

    class Sequence
    {
    public:
        Sequence(Conveyor* pHead, uint8_t uiLength, Vector2F laneOnePosition, Vector2F laneTwoPosition, Vector2F unitDirection)
            : m_InitializationState{ { LaneVisual(laneOnePosition, unitDirection), LaneVisual(laneTwoPosition, unitDirection) } }
            , m_RealizedState{ {0, 0}, {0, 0} }
            , m_PendingState{ {0, 0}, {0, 0} }
            , m_pHeadConveyor(pHead)
            , m_Length(uiLength)
        {
        } 

        void Tick(SceneContext& kContext);
        void Realize();

        const Conveyor* GetHeadConveyor() const { return m_pHeadConveyor; }
         
        inline static constexpr uint32_t c_uiMaxSequenceLength = 32;
        bool HasItemInSlot(uint8_t uiSequenceIndex, int lane, int slot) const;
        void AddItemInSlot(uint8_t uiSequenceIndex, int lane, int slot);
        bool DidItemMoveLastSimulation(uint8_t uiSequenceIndex, int lane, int slot) const;
        bool TryPeakItemInSlot(uint8_t uiSequenceIndex, int lane, int slot, ItemInstance& pItem);
        uint64_t CountItemsOnBelt();

        uint32_t GetMoveTick() const { return m_pHeadConveyor->m_uiMoveTick; }
        uint32_t GetCurrentTick() const { return m_pHeadConveyor->m_uiCurrentTick; }
    private:
        friend class Conveyor;
        struct LaneVisual
        {
            LaneVisual(Vector2F origin, Vector2F direction)
                : m_Origin(origin)
                , m_UnitDirection(direction)
            {
            }

            Vector2F m_Origin;
            Vector2F m_UnitDirection;
        };

        struct PositionVisualOverride
        {
            Vector2F m_Position;
            bool m_bIsSet;
        };

        struct
        {
            std::array<LaneVisual, c_conveyorChannels> m_ConveyorVisualOffsets;
        }
        m_InitializationState;

        struct
        {
            std::array<uint64_t, c_conveyorChannels> m_Lanes;
            std::array<uint64_t, c_conveyorChannels> m_RealizedMovements;
        }
        m_RealizedState;

        struct
        {
            std::array<uint64_t, c_conveyorChannels> m_PendingMoves;
            std::array<uint64_t, c_conveyorChannels> m_PendingClears;
        }
        m_PendingState;

        Conveyor* m_pHeadConveyor;
        const uint8_t m_Length;
    };

    Conveyor* TraceHeadConveyor(WorldMap& map, Conveyor& searchStart);
    const Conveyor* TraceTailConveyor(WorldMap& map, Conveyor& searchStart, Conveyor& head, std::vector<Conveyor*>& vOutConveyors);

    std::vector<Sequence*> InitializeSequences(WorldMap& map, const std::vector<Conveyor*>& conveyors);

    std::tuple<int, Direction> GetInnerMostCornerChannel(const cpp_conv::WorldMap& map, const Conveyor& rConveyor);
}
