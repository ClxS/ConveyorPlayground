#pragma once

#include "Entity.h"

#include <vector>
#include "GeneralItemContainer.h"

namespace cpp_conv
{
    class Storage : public Entity
    {
    public:
        Storage(Eigen::Vector3i position, Eigen::Vector3i size, uint32_t uiMaxCapacity, uint32_t uiMaxStackSize)
            : Entity(position, size, EntityKind::Storage)
              , m_ItemContainer(uiMaxCapacity, uiMaxStackSize, false)
        {
        }

        [[nodiscard]] const char* GetName() const { return "Storage"; }
        [[nodiscard]] std::string GetDescription() const { return m_ItemContainer.GetDescription(); }

        [[nodiscard]] const GeneralItemContainer& GetContainer() const { return m_ItemContainer; }
    private:
        GeneralItemContainer m_ItemContainer;
    };
}
