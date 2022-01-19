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

    class Conveyor : public Entity
    {
    public:
        struct Lane
        {
            ItemInstance m_Item;
            Vector2F m_VisualPosition;
        };

        struct Channel
        {
            Channel(int channelLane); 

            const int m_ChannelLane;

            int m_LaneLength;
            std::array<Lane, c_conveyorChannelSlots + 1> m_pSlots;
            std::array<ItemInstance, c_conveyorChannelSlots + 1> m_pPendingItems;
        };

        Conveyor(Vector3 position, Vector3 size, Direction direction, ItemId pItem = {});

        Direction m_direction;
        int m_pSequenceId;

        std::array<Channel, c_conveyorChannels> m_pChannels;

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kContext) const override;
        bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel, int iSourceSlot) override;

        bool SupportsProvidingItem() const override { return true; }
        bool TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem) override;

        Direction GetDirection() const override { return m_direction; }

        const char* GetName() const override { return "Conveyor"; }

        std::string GetDescription() const override;

        virtual uint32_t GetDrawPassCount() const { return 2; }

        void AssessPosition(const cpp_conv::WorldMap& map);

        bool IsCorner() const { return m_bIsCorner; }
        bool IsClockwiseCorner() const { return m_bIsClockwise; }
        bool IsCapped() const { return m_bIsCapped; }
        int GetInnerMostChannel() const { return m_iInnerMostChannel; }
        Direction GetCornerDirection() const { return m_eCornerDirection; }

        cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> GetTile() const { return m_pTile; }

        static_assert(c_conveyorChannels >= 1, "Conveyors must have at least once channel");
        static_assert(c_conveyorChannelSlots >= 1, "Conveyors channels must have at least once slot");
    private: 
        friend class Sequence;

        uint32_t m_uiCurrentTick = 0;
        uint32_t m_uiMoveTick = 10;

        bool m_bIsCorner;
        bool m_bIsClockwise;
        bool m_bIsCapped;
        int m_iInnerMostChannel;
        Direction m_eCornerDirection;
        cpp_conv::resources::AssetPtr<cpp_conv::resources::TileAsset> m_pTile;

        void AddItemToSlot(const cpp_conv::WorldMap& map, Channel* pTargetChannel, int forwardTargetItemSlot, const ItemId pItem, const Entity& pSourceEntity, int iSourceChannel, int iSourceSlot);
    };
}
