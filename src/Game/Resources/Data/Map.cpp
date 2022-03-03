#include "Map.h"
#include <vector>
#include "Conveyor.h"

using cpp_conv::Entity;
using cpp_conv::Conveyor;
using cpp_conv::resources::Map;

Map::Map()
{
}

const std::vector<Entity*>& Map::GetOtherEntities() const
{
    return m_vOtherEntities;
}

std::vector<Entity*>& Map::GetOtherEntities()
{
    return m_vOtherEntities;
}

const std::vector<Conveyor*>& Map::GetConveyors() const
{
    return m_vConveyors;
}

std::vector<Conveyor*>& Map::GetConveyors()
{
    return m_vConveyors;
}

Map::~Map()
{
    for (const Conveyor* pConveyor : m_vConveyors)
    {
        delete pConveyor;
    }

    for (const Entity* pEntity : m_vOtherEntities)
    {
        delete pEntity;
    }

    m_vConveyors.clear();
    m_vOtherEntities.clear();
}
