#pragma once

#include <array>

#include "Renderer.h"
#include "Entity.h"
#include "Enums.h"
#include "DataId.h"
#include "ItemInstance.h"
#include "AssetPtr.h"
#include "TileAsset.h"

namespace cpp_conv { class WorldMap; }

namespace cpp_conv
{
    constexpr int c_conveyorChannels = 2;
    constexpr int c_conveyorChannelSlots = 2;

    class Sequence;
    struct SceneContext;
    struct RenderContext;

    class Conveyor final : public Entity
    {
    public:
        struct Lane
        {
            ItemInstance m_Item;
            Vector2F m_VisualPosition;
        };

        struct Channel
        {
            // ReSharper disable once CppNonExplicitConvertingConstructor
            Channel(int channelLane);

            const int m_ChannelLane;

            int m_LaneLength = 2;
            std::array<Lane, c_conveyorChannelSlots + 1> m_pSlots;
            std::array<ItemInstance, c_conveyorChannelSlots + 1> m_pPendingItems;
        };

        Conveyor(Vector3 position, Vector3 size, Direction direction, ItemId pItem = {});

        Direction m_direction;

        std::array<Channel, c_conveyorChannels> m_pChannels;

        void Tick(const SceneContext& kContext) override;
        void Realize();

        void Draw(RenderContext& kContext) const override;
        [[nodiscard]] bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, InsertInfo insertInfo) override;

        [[nodiscard]] bool SupportsProvidingItem() const override { return true; }
        bool TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem) override;

        [[nodiscard]] Direction GetDirection() const override { return m_direction; }

        [[nodiscard]] const char* GetName() const override { return "Conveyor"; }

        [[nodiscard]] std::string GetDescription() const override;

        [[nodiscard]] uint32_t GetDrawPassCount() const override { return 2; }

        [[nodiscard]] bool IsCorner() const { return m_bIsCorner; }
        [[nodiscard]] bool IsClockwiseCorner() const { return m_bIsClockwise; }
        [[nodiscard]] bool IsCapped() const { return m_bIsCapped; }
        [[nodiscard]] int GetInnerMostChannel() const { return m_iInnerMostChannel; }
        [[nodiscard]] Direction GetCornerDirection() const { return m_eCornerDirection; }

        [[nodiscard]] bool IsPartOfASequence() const { return m_pSequence != nullptr; }

        [[nodiscard]] bool RequiresPlacementLocalityChecks() const override { return true; }
        void OnLocalityUpdate(const WorldMap& map) override;

        void SetSequence(Sequence* pSequence, uint8_t position);
        void ClearSequence();

        uint64_t CountItemsOnBelt();

        [[nodiscard]] resources::AssetPtr<resources::TileAsset> GetTile() const { return m_pTile; }

        static_assert(c_conveyorChannels >= 1, "Conveyors must have at least once channel");
        static_assert(c_conveyorChannelSlots >= 1, "Conveyors channels must have at least once slot");
    private:
        bool HasItemInSlot(int lane, int slot);
        void PlaceItemInSlot(int lane, int slot, InsertInfo insertInfo, bool bDirectItemSet = false);

        friend class Sequence;

        Sequence* m_pSequence = nullptr;
        uint8_t m_uiSequenceIndex;

        uint32_t m_uiCurrentTick = 0;
        uint32_t m_uiMoveTick = 10;
        bool m_bHasWork = false;

        bool m_bIsCorner;
        bool m_bIsClockwise;
        bool m_bIsCapped;
        int m_iInnerMostChannel;
        Direction m_eCornerDirection;
        resources::AssetPtr<resources::TileAsset> m_pTile;

        bool TryPeakItemInSlot(int lane, int slot, ItemInstance& rItem) const;
    };
}
