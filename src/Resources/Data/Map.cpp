#include "Map.h"
#include <vector>
#include "Conveyor.h"

using cpp_conv::Entity;
using cpp_conv::Conveyor;
using cpp_conv::resources::Map;

cpp_conv::resources::Map::Map()
{
}

const std::vector<Entity*>& cpp_conv::resources::Map::GetOtherEntities() const
{
    return m_vOtherEntities;
}

std::vector<Entity*>& cpp_conv::resources::Map::GetOtherEntities()
{
    return m_vOtherEntities;
}

const std::vector<Conveyor*>& cpp_conv::resources::Map::GetConveyors() const
{
    return m_vConveyors;
}

std::vector<Conveyor*>& cpp_conv::resources::Map::GetConveyors()
{
    return m_vConveyors;
}

cpp_conv::resources::Map::~Map()
{
    for (Conveyor* pConveyor : m_vConveyors)
    {
        delete pConveyor;
    }

    for (Entity* pEntity : m_vOtherEntities)
    {
        delete pEntity;
    }

    m_vConveyors.clear();
    m_vOtherEntities.clear();
}
