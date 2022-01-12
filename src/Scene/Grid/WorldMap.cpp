#include "WorldMap.h"
#include "Map.h"
#include "Conveyor.h"
#include <cassert>

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

cpp_conv::WorldMap::CellCoordinate cpp_conv::WorldMap::ToCellSpace(Vector3 position)
{
    constexpr int32_t worldToGridSpaceValue = (c_uiMaximumMapSize / 2) * c_uiCellSize;
    constexpr Vector3 worldToGridSpaceTransform = { worldToGridSpaceValue, worldToGridSpaceValue, 0 };
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

const cpp_conv::Entity* cpp_conv::WorldMap::GetEntity(Vector3 position) const
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
        if (PlaceEntity(pConveyor->m_position, pConveyor))
        {
            m_vConveyors.push_back(pConveyor);
        }
        else
        {
            delete pConveyor;
        }
    }
     
    for (Entity* pEntity : map->GetOtherEntities())
    {
        if (PlaceEntity(pEntity->m_position, pEntity))
        {
            m_vOtherEntities.push_back(pEntity);
        }
        else
        {
            delete pEntity;
        }
    }

    map->GetConveyors().clear();
    map->GetOtherEntities().clear();
}

cpp_conv::Entity* cpp_conv::WorldMap::GetEntity(Vector3 position)
{
    return const_cast<cpp_conv::Entity*>(const_cast<const cpp_conv::WorldMap*>(this)->GetEntity(position));
}

bool cpp_conv::WorldMap::PlaceEntity(Vector3 position, Entity* pEntity)
{
    if (!ValidateCanPlaceEntity(position, pEntity))
    {
        return false;
    }

    for (int32_t iXPosition = position.m_x; iXPosition < (position.m_x + pEntity->m_size.m_x); ++iXPosition)
    {
        for (int32_t iYPosition = position.m_y; iYPosition < (position.m_y + pEntity->m_size.m_y); ++iYPosition)
        {
            for (int32_t iDepthPosition = position.m_depth; iDepthPosition < (position.m_depth + pEntity->m_size.m_depth); ++iDepthPosition)
            {
                CellCoordinate coord = ToCellSpace({ iXPosition, iYPosition, iDepthPosition });
                assert(!coord.IsInvalid());

                Cell* pCell = GetOrCreateCell(coord);
                assert(pCell != nullptr);

                assert(pCell->SetEntity(coord, pEntity));
            }
        }
    }

    if (pEntity->m_eEntityKind == EntityKind::Conveyor)
    {
        m_vConveyors.push_back(reinterpret_cast<Conveyor*>(pEntity));
    }
    else
    {
        m_vOtherEntities.push_back(pEntity);
    }

    return true;
}

bool cpp_conv::WorldMap::ValidateCanPlaceEntity(Vector3 position, Entity* pEntity) const
{
    for (int32_t iXPosition = position.m_x; iXPosition < (position.m_x + pEntity->m_size.m_x); ++iXPosition)
    {
        for (int32_t iYPosition = position.m_y; iYPosition < (position.m_y + pEntity->m_size.m_y); ++iYPosition)
        {
            for (int32_t iDepthPosition = position.m_depth; iDepthPosition < (position.m_depth + pEntity->m_size.m_depth); ++iDepthPosition)
            {
                Vector3 checkPosition = { iXPosition, iYPosition, iDepthPosition };
                CellCoordinate coord = ToCellSpace(checkPosition);
                if (coord.IsInvalid())
                {
                    return false;
                }

                Cell* pCell = GetCell(coord);
                if (!pCell)
                {
                    continue;
                }

                if (pCell->GetEntity(coord))
                {
                    return false;
                }
            }
        }
    }

    return true;
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
