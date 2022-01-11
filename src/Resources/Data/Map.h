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
        ~Map();

        std::vector<Conveyor*>& GetConveyors();
        std::vector<Entity*>& GetOtherEntities();

        const std::vector<Conveyor*>& GetConveyors() const;
        const std::vector<Entity*>& GetOtherEntities() const;

    private:
        std::vector<Conveyor*> m_vConveyors;
        std::vector<Entity*> m_vOtherEntities;
    };
}
