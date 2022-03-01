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
        Sequence(
            const Entity* pHead,
            const uint8_t uiLength,
            const Vector2F laneOnePosition,
            const Vector2F laneTwoPosition,
            const Vector2F unitDirection,
            const uint32_t uiMoveTick)
            : m_InitializationState{ { LaneVisual(laneOnePosition, unitDirection), LaneVisual(laneTwoPosition, unitDirection) } }
            , m_RealizedStates { RealizedState(uiLength * 2), RealizedState(uiLength * 2) }
            , m_PendingStates{ PendingState(uiLength * 2), PendingState(uiLength * 2) }
            , m_pHeadEntity(pHead)
            , m_Length(uiLength)
            , m_uiCurrentTick{0}
            , m_uiMoveTick{uiMoveTick}
        {
        }

        void Tick(const SceneContext& kContext);

        void Realize();

        inline static constexpr uint32_t c_uiMaxSequenceLength = 32;
        [[nodiscard]] bool HasItemInSlot(uint8_t uiSequenceIndex, int lane, int slot) const;

        void AddItemInSlot(uint8_t uiSequenceIndex, int lane, int slot, ItemId item, const Vector2F* origin = nullptr);
        bool RemoveItemFromSlot(uint8_t uiSequenceIndex, int iChannelLane, int iChannelSlot, ItemId& outItem, Vector2F& outOrigin);

        [[nodiscard]] const Entity* GetHeadEntity() const { return m_pHeadEntity; }

        [[nodiscard]] bool DidItemMoveLastSimulation(uint8_t uiSequenceIndex, int lane, int slot) const;
        [[nodiscard]] Vector2F GetSlotPosition(uint8_t uiSequenceIndex, const int lane, int slot) const;
        bool TryPeakItemInSlot(uint8_t uiSequenceIndex, int lane, int slot, ItemInstance& pItem) const;
        [[nodiscard]] uint64_t CountItemsOnBelt() const;

        [[nodiscard]] uint32_t GetMoveTick() const { return m_uiMoveTick; }
        [[nodiscard]] uint32_t GetCurrentTick() const { return m_uiCurrentTick; }

        [[nodiscard]] uint32_t NeedsRealization() const { return m_bNeedsRealization; }

    private:
        [[nodiscard]] bool MoveItemToForwardsNode(const SceneContext& kContext, const Entity& pNode, int lane) const;

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
            Vector2F m_Position{};
            bool m_bHasPosition = false;

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
                , m_Position(position)
                , m_bHasPosition{true}
            {
            }

            [[nodiscard]] bool HasItem() const { return m_Item.IsValid(); }
        };

        struct
        {
            std::array<LaneVisual, c_conveyorChannels> m_ConveyorVisualOffsets;
        }
        m_InitializationState;

        struct RealizedState
        {
            explicit RealizedState(const uint32_t uiConveyorLength) : m_Items{(uiConveyorLength)}
            {}

            uint64_t m_Lanes = {};
            uint64_t m_RealizedMovements = {};
            uint64_t m_HasOverridePosition = {};
            FixedCircularBuffer<SlotItem> m_Items;
        };
        struct PendingState
        {
            explicit PendingState(const uint32_t uiConveyorLength) : m_NewItems{(uiConveyorLength)}
            {}

            uint64_t m_PendingInsertions = {};
            uint64_t m_PendingMoves = {};
            uint64_t m_PendingClears = {};
            uint64_t m_PendingRemovals = {};
            FixedCircularBuffer<SlotItem> m_NewItems;
        };

        std::array<RealizedState, c_conveyorChannels> m_RealizedStates;
        std::array<PendingState, c_conveyorChannels> m_PendingStates;

        const Entity* m_pHeadEntity;
        const uint8_t m_Length;

        uint32_t m_uiCurrentTick;
        uint32_t m_uiMoveTick;
        bool m_bNeedsRealization = false;

        const RealizedState& GetFreshRealizedStateForTick(uint8_t uiLane);
    };

    Conveyor* traceHeadConveyor(WorldMap& map, const Conveyor& searchStart);
    const Conveyor* traceTailConveyor(WorldMap& map, const Conveyor& searchStart, const Conveyor& head, std::vector<Conveyor*>& vOutConveyors);
    std::vector<Sequence*> initializeSequences(WorldMap& map, const std::vector<Conveyor*>& conveyors);
    std::tuple<int, Direction> getInnerMostCornerChannel(const WorldMap& map, const Conveyor& rConveyor);
}
