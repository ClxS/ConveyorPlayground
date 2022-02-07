#pragma once

#include <vector>
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
        Sequence(Conveyor* pHead, const uint8_t uiLength, const Vector2F laneOnePosition, const Vector2F laneTwoPosition, const Vector2F unitDirection)
            : m_InitializationState{ { LaneVisual(laneOnePosition, unitDirection), LaneVisual(laneTwoPosition, unitDirection) } }
            , m_RealizedState{ {0, 0}, {0, 0}, {0, 0}, { 64, 64 } }
            , m_PendingState{ {0, 0}, {0, 0}, {0, 0}, { 64, 64 } }
            , m_pHeadConveyor(pHead)
            , m_Length(uiLength)
        {
        }

        void Tick(SceneContext& kContext);
        void Realize();

        [[nodiscard]] const Conveyor* GetHeadConveyor() const { return m_pHeadConveyor; }

        inline static constexpr uint32_t c_uiMaxSequenceLength = 32;
        [[nodiscard]] bool HasItemInSlot(uint8_t uiSequenceIndex, int lane, int slot) const;
        void AddItemInSlot(uint8_t uiSequenceIndex, int lane, int slot, ItemId item, const Vector2F* origin = nullptr);
        [[nodiscard]] bool DidItemMoveLastSimulation(uint8_t uiSequenceIndex, int lane, int slot) const;
        bool TryPeakItemInSlot(uint8_t uiSequenceIndex, int lane, int slot, ItemInstance& pItem) const;
        [[nodiscard]] uint64_t CountItemsOnBelt() const;

        [[nodiscard]] uint32_t GetMoveTick() const { return m_pHeadConveyor->m_uiMoveTick; }
        [[nodiscard]] uint32_t GetCurrentTick() const { return m_pHeadConveyor->m_uiCurrentTick; }
    private:
        friend class Conveyor;
        struct LaneVisual
        {
            LaneVisual(const Vector2F origin, const Vector2F direction)
                : m_Origin(origin)
                , m_UnitDirection(direction)
            {
            }

            Vector2F m_Origin;
            Vector2F m_UnitDirection;
        };
        struct SlotItem
        {
            ItemId m_Item;
            union {
                Vector2F m_Position{};
            };

            // Test assumptions about the above
            static_assert(sizeof(Vector2F) == sizeof(uint64_t), "Vector2F is not expected size");

            SlotItem()
                : m_Item(ItemId::Empty())
            {
            }

            explicit SlotItem(const ItemId uiItemId)
                : m_Item(uiItemId)
            {
            }

            explicit SlotItem(const ItemId uiItemId, const Vector2F position)
                : m_Item(uiItemId)
            {
                m_Position = position;
            }

            [[nodiscard]] bool HasItem() const { return m_Item.IsValid(); }
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
            std::array<uint64_t, c_conveyorChannels> m_HasOverridePosition;
            std::array<FixedCircularBuffer<SlotItem>, c_conveyorChannels> m_Items;
        }
        m_RealizedState;

        struct
        {
            std::array<uint64_t, c_conveyorChannels> m_PendingInsertions;
            std::array<uint64_t, c_conveyorChannels> m_PendingMoves;
            std::array<uint64_t, c_conveyorChannels> m_PendingClears;
            std::array<FixedCircularBuffer<SlotItem>, c_conveyorChannels> m_NewItems;
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
