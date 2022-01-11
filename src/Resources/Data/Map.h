#pragma once

#include "ResourceAsset.h"
#include "EntityGrid.h"
#include <vector>

namespace cpp_conv::resources
{
    class Map : public ResourceAsset
    {
    public:
        Map();
        Map(cpp_conv::grid::EntityGrid& kGrid, std::vector<Conveyor*> vConveyors, std::vector<Entity*> vOtherEntities);

        cpp_conv::grid::EntityGrid& GetGrid();
        std::vector<Conveyor*>& GetConveyors();
        std::vector<Entity*>& GetOtherEntities();

        const cpp_conv::grid::EntityGrid& GetGrid() const;
        const std::vector<Conveyor*>& GetConveyors() const;
        const std::vector<Entity*>& GetOtherEntities() const;

    private:
        cpp_conv::grid::EntityGrid m_kEntityGrid;
        std::vector<Conveyor*> m_vConveyors;
        std::vector<Entity*> m_vOtherEntities;
    };
}
