#include "Map.h"
#include <vector>

using cpp_conv::Entity;
using cpp_conv::Conveyor;
using cpp_conv::resources::Map;

cpp_conv::resources::Map::Map()
    : m_kEntityGrid({})
{
}

cpp_conv::resources::Map::Map(cpp_conv::grid::EntityGrid& kGrid, std::vector<Conveyor*> vConveyors, std::vector<Entity*> vOtherEntities)
    : m_kEntityGrid(std::move(kGrid))
    , m_vConveyors(std::move(vConveyors))
    , m_vOtherEntities(std::move(vOtherEntities))
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

const cpp_conv::grid::EntityGrid& cpp_conv::resources::Map::GetGrid() const
{
    return m_kEntityGrid;
}

cpp_conv::grid::EntityGrid& cpp_conv::resources::Map::GetGrid()
{
    return m_kEntityGrid;
}
