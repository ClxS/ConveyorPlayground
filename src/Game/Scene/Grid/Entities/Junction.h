#pragma once

#include "Entity.h"

namespace cpp_conv
{
    struct SceneContext;
    struct RenderContext;

    class Junction : public Entity
    {
    public:
        Junction(Eigen::Vector3i position, Eigen::Vector3i size)
            : Entity(position, size, EntityKind::Junction)
              , m_pItem(ItemIds::None)
              , m_uiTick(0)
        {
        }

        [[nodiscard]] const char* GetName() const { return "Junction"; }
        [[nodiscard]] std::string GetDescription() const { return ""; }

    private:
        ItemId m_pItem;
        uint64_t m_uiTick;
    };
}
