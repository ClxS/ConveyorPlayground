#include "WorldMap.h"
#include "Map.h"
#include "Conveyor.h"

bool cpp_conv::WorldMap::Cell::HasFloor(uint32_t uiFloor) const
{
    if (uiFloor >= m_CellGrid.size())
    {
        return false;
    }

    return m_CellGrid[uiFloor] != nullptr;
}

cpp_conv::WorldMap::Cell::EntityGrid& cpp_conv::WorldMap::Cell::GetFloor(uint32_t uiFloor)
{
    return *m_CellGrid[uiFloor];
}

const cpp_conv::WorldMap::Cell::EntityGrid& cpp_conv::WorldMap::Cell::GetFloor(uint32_t uiFloor) const
{
    return *m_CellGrid[uiFloor];
}

bool cpp_conv::WorldMap::Cell::CreateFloor(uint32_t uiFloor)
{
    if (uiFloor >= m_CellGrid.size())
    {
        return false;
    }

    if (m_CellGrid[uiFloor] != nullptr)
    {
        return true;
    }

    m_CellGrid[uiFloor] = std::make_unique<EntityGrid>();
    return true;
}

const cpp_conv::Entity* cpp_conv::WorldMap::Cell::GetEntity(CellCoordinate coord) const
{
    if (coord.IsInvalid())
    {
        return nullptr;
    }

    if (!HasFloor(coord.m_Depth))
    {
        return nullptr;
    }

    const EntityGrid& rFloor = GetFloor(coord.m_Depth);
    return rFloor[coord.m_CellSlotY][coord.m_CellSlotX].get();
}

cpp_conv::Entity* cpp_conv::WorldMap::Cell::GetEntity(CellCoordinate coord)
{
    return const_cast<cpp_conv::Entity*>(const_cast<const cpp_conv::WorldMap::Cell*>(this)->GetEntity(coord));
}

bool cpp_conv::WorldMap::Cell::SetEntity(CellCoordinate coord, Entity* pEntity)
{
    if (coord.IsInvalid())
    {
        return false;
    }

    if (!CreateFloor(coord.m_Depth))
    {
        return false;
    }

    EntityGrid& rFloor = GetFloor(coord.m_Depth);
    if (rFloor[coord.m_CellSlotY][coord.m_CellSlotX])
    {
        return false;
    }

    rFloor[coord.m_CellSlotY][coord.m_CellSlotX].reset(pEntity);
    return true;
}

cpp_conv::WorldMap::CellCoordinate cpp_conv::WorldMap::ToCellSpace(Position position)
{
    constexpr int32_t worldToGridSpaceValue = (c_uiMaximumMapSize / 2) * c_uiCellSize;
    constexpr Position worldToGridSpaceTransform = { worldToGridSpaceValue, worldToGridSpaceValue, 0 };
    position += worldToGridSpaceTransform;

    constexpr int32_t axisSize = c_uiMaximumMapSize * c_uiCellSize;
    if (position.m_x < 0 || position.m_y < 0 || position.m_x >= axisSize || position.m_y >= axisSize)
    {
        return { -1, -1, -1, -1 };
    }

    int32_t iHorizontalCell = position.m_x / c_uiCellSize;
    int32_t iHorizontalSlot = position.m_x % c_uiCellSize;

    int32_t iVerticalCell = position.m_y / c_uiCellSize;
    int32_t iVerticalSlot = position.m_y % c_uiCellSize;

    return { iHorizontalCell, iVerticalCell, iHorizontalSlot, iVerticalSlot, position.m_depth };
}

const cpp_conv::Entity* cpp_conv::WorldMap::GetEntity(Position position) const
{
    CellCoordinate coord = ToCellSpace(position);
    if (coord.IsInvalid())
    {
        return nullptr;
    }

    Cell* pCell = GetCell(coord);
    if (!pCell)
    {
        return nullptr;
    }

    return pCell->GetEntity(coord);
}

void cpp_conv::WorldMap::Consume(cpp_conv::resources::AssetPtr<cpp_conv::resources::Map> map)
{
    for (Conveyor* pConveyor : map->GetConveyors())
    {
        PlaceEntity(pConveyor->m_position, pConveyor);
        m_vConveyors.push_back(pConveyor);
    }

    for (Entity* pEntity : map->GetOtherEntities())
    {
        PlaceEntity(pEntity->m_position, pEntity);
        m_vOtherEntities.push_back(pEntity);
    }

    map->GetConveyors().clear();
    map->GetOtherEntities().clear();
}

cpp_conv::Entity* cpp_conv::WorldMap::GetEntity(Position position)
{
    return const_cast<cpp_conv::Entity*>(const_cast<const cpp_conv::WorldMap*>(this)->GetEntity(position));
}

bool cpp_conv::WorldMap::PlaceEntity(Position position, Entity* pEntity)
{
    CellCoordinate coord = ToCellSpace(position);
    if (coord.IsInvalid())
    {
        return false;
    }

    Cell* pCell = GetOrCreateCell(coord);
    if (!pCell)
    {
        return false;
    }

    return pCell->SetEntity(coord, pEntity);
}

cpp_conv::WorldMap::Cell* cpp_conv::WorldMap::GetCell(CellCoordinate coord) const
{
    if (coord.IsInvalid())
    {
        return nullptr;
    }

    return m_WorldMap[coord.m_CellY][coord.m_CellX].get();
}

cpp_conv::WorldMap::Cell* cpp_conv::WorldMap::GetOrCreateCell(CellCoordinate coord)
{
    if (coord.IsInvalid())
    {
        return nullptr;
    }

    if (!m_WorldMap[coord.m_CellY][coord.m_CellX])
    {
        m_WorldMap[coord.m_CellY][coord.m_CellX] = std::make_unique<Cell>();
    }

    return m_WorldMap[coord.m_CellY][coord.m_CellX].get();
}
