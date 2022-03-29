#pragma once

#include <vector>
#include "EntityGrid.h"
#include "ResourceAsset.h"

namespace cpp_conv::resources
{
    class Map : public ResourceAsset
    {
    public:
        Map();
        ~Map() override;

        std::vector<Conveyor*>& GetConveyors();
        std::vector<Entity*>& GetOtherEntities();

        [[nodiscard]] const std::vector<Conveyor*>& GetConveyors() const;
        [[nodiscard]] const std::vector<Entity*>& GetOtherEntities() const;

    private:
        std::vector<Conveyor*> m_vConveyors;
        std::vector<Entity*> m_vOtherEntities;
    };
}
