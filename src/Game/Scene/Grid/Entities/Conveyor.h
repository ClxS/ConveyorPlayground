#pragma once

#include <array>

#include "AtlasResource/AssetPtr.h"
#include "DataId.h"
#include "Entity.h"
#include "Enums.h"
#include "Renderer.h"
#include "TileAsset.h"

namespace cpp_conv
{
    class WorldMap;
}

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
        Conveyor(Eigen::Vector3i position, Eigen::Vector3i size, Direction direction, ItemId pItem = {})
            : Entity(position, size, EntityKind::Conveyor, direction)
        {
        }


        [[nodiscard]] bool SupportsInsertion() const { return true; }
        [[nodiscard]] const char* GetName() const { return "Conveyor"; }
        [[nodiscard]] bool IsCorner() const { return m_bIsCorner; }
        [[nodiscard]] bool IsClockwiseCorner() const { return m_bIsClockwise; }
        [[nodiscard]] bool IsCapped() const { return m_bIsCapped; }
        [[nodiscard]] int GetInnerMostChannel() const { return m_iInnerMostChannel; }
        [[nodiscard]] uint32_t GetMoveTick() const { return m_uiMoveTick; }
    private:
        uint32_t m_uiMoveTick = 10;
        bool m_bIsCorner;
        bool m_bIsClockwise;
        bool m_bIsCapped;
        int m_iInnerMostChannel;

    };
}
