#pragma once

#include "Entity.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    class Stairs : public Entity
    {
    public:
        Stairs(Eigen::Vector3i position, Eigen::Vector3i size, Direction direction, bool bIsUp)
            : Entity(position, size, EntityKind::Stairs)
              , m_pItem(ItemIds::None)
              , m_uiTick(0)
              , m_direction(direction)
              , m_bIsUp(bIsUp)
        {
        }

        [[nodiscard]] const char* GetName() const { return m_bIsUp ? "Stairs (Up)" : "Stairs (Down)"; }
        [[nodiscard]] std::string GetDescription() const { return ""; }

        [[nodiscard]] Direction GetDirection() const { return m_direction; }

    private:
        ItemId m_pItem;
        uint64_t m_uiTick;
        Direction m_direction;

        bool m_bIsUp;
    };
}
