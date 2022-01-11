#pragma once

#include <array>

#include "Renderer.h"
#include "Entity.h"
#include "Enums.h"
#include "DataId.h"

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
        struct Channel
        {
            Channel(); 

            std::array<ItemId, c_conveyorChannelSlots + 1> m_pItems;
            std::array<ItemId, c_conveyorChannelSlots + 1> m_pPendingItems;
        };

        Conveyor(Vector3 position, Vector3 size, Direction direction, ItemId pItem = {});

        Direction m_direction;
        int m_pSequenceId;

        std::array<Channel, c_conveyorChannels> m_pChannels;

        void Tick(const SceneContext& kContext) override;
        void Draw(RenderContext& kContext) const override;
        bool SupportsInsertion() const override { return true; }
        bool TryInsert(const SceneContext& kContext, const Entity& pSourceEntity, ItemId pItem, int iSourceChannel) override;

        bool SupportsProvidingItem() const override { return true; }
        bool TryGrab(const SceneContext& kContext, bool bSingle, std::tuple<ItemId, uint32_t>& outItem) override;

        Direction GetDirection() const override { return m_direction; }

        const char* GetName() const override { return "Conveyor"; }

        std::string GetDescription() const override;

        static_assert(c_conveyorChannels >= 1, "Conveyors must have at least once channel");
        static_assert(c_conveyorChannelSlots >= 1, "Conveyors channels must have at least once slot");
    };
}
