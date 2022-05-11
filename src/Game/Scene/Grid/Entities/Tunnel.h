#pragma once

#include "Conveyor.h"
#include "Entity.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    constexpr int c_maxUndergroundLength = 6;

    class Tunnel final : public Entity
    {
    public:
        Tunnel(Eigen::Vector3i position, Eigen::Vector3i size, Direction direction)
            : Entity(position, size, EntityKind::Tunnel)
              , m_pOtherSide{nullptr}
              , m_pSequence{nullptr}
              , m_Direction(direction)
        {
        }

        [[nodiscard]] Direction GetDirection() const { return m_Direction; }

        [[nodiscard]] const char* GetName() const { return "Tunnel"; }
        [[nodiscard]] std::string GetDescription() const { return ""; }

    private:
        static inline constexpr uint32_t c_uiMaxLength = 7;

        Tunnel* m_pOtherSide;
        Sequence* m_pSequence;
        Direction m_Direction;

        const uint32_t m_uiMoveTick = 10;
    };
}
