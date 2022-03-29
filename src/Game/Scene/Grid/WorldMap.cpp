#include "WorldMap.h"
#include <cassert>
#include "Conveyor.h"
#include "Map.h"

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

    m_bHasFloors = true;
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

cpp_conv::Entity* cpp_conv::WorldMap::Cell::GetEntity(const CellCoordinate coord)
{
    return const_cast<Entity*>(const_cast<const Cell*>(this)->GetEntity(coord));
}

bool cpp_conv::WorldMap::Cell::SetEntity(const CellCoordinate coord, Entity* pEntity)
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
    const Vector3 worldToGridSpaceTransform = {worldToGridSpaceValue, worldToGridSpaceValue, 0};
    position += worldToGridSpaceTransform;

    constexpr int32_t axisSize = c_uiMaximumMapSize * c_uiCellSize;
    if (position.GetX() < 0 || position.GetY() < 0 || position.GetX() >= axisSize || position.GetY() >= axisSize)
    {
        return {-1, -1, -1, -1, position.GetZ()};
    }

    const auto iHorizontalCell = position.GetX() / c_uiCellSize;
    const auto iHorizontalSlot = position.GetX() % c_uiCellSize;

    const auto iVerticalCell = position.GetY() / c_uiCellSize;
    const auto iVerticalSlot = position.GetY() % c_uiCellSize;

    return {iHorizontalCell, iVerticalCell, iHorizontalSlot, iVerticalSlot, position.GetZ()};
}

cpp_conv::Entity* cpp_conv::WorldMap::GetEntity(const Vector3 position) const
{
    const CellCoordinate coord = ToCellSpace(position);
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

void cpp_conv::WorldMap::Consume(const resources::AssetPtr<resources::Map>& map)
{
    m_bSuppressAssess = true;
    for (Conveyor* pConveyor : map->GetConveyors())
    {
        if (!PlaceEntity(pConveyor->m_position, pConveyor))
        {
            delete pConveyor;
        }
    }

    for (Entity* pEntity : map->GetOtherEntities())
    {
        if (!PlaceEntity(pEntity->m_position, pEntity))
        {
            delete pEntity;
        }
    }

    map->GetConveyors().clear();
    map->GetOtherEntities().clear();
    m_bSuppressAssess = false;

    for (Entity* pEntity : this->GetConveyors())
    {
        if (pEntity->RequiresPlacementLocalityChecks())
        {
            pEntity->OnLocalityUpdate(*this);
        }
    }

    for (Entity* pEntity : this->GetOtherEntities())
    {
        if (pEntity->RequiresPlacementLocalityChecks())
        {
            pEntity->OnLocalityUpdate(*this);
        }
    }
}

void cpp_conv::WorldMap::PopulateCorners()
{
    for (auto pConveyor : m_vConveyors)
    {
        if (pConveyor->IsCorner())
        {
            m_vCornerConveyors.push_back(pConveyor);
        }
    }
}

bool cpp_conv::WorldMap::PlaceEntity(Vector3 position, Entity* pEntity)
{
    if (!ValidateCanPlaceEntity(position, pEntity))
    {
        return false;
    }

    for (int32_t iXPosition = position.GetX(); iXPosition < (position.GetX() + pEntity->m_size.GetX()); ++iXPosition)
    {
        for (int32_t iYPosition = position.GetY(); iYPosition < (position.GetY() + pEntity->m_size.GetY()); ++
             iYPosition)
        {
            for (int32_t iDepthPosition = position.GetZ(); iDepthPosition < (position.GetZ() + pEntity->m_size.GetZ());
                 ++iDepthPosition)
            {
                CellCoordinate coord = ToCellSpace({iXPosition, iYPosition, iDepthPosition});
                assert(!coord.IsInvalid());

                Cell* pCell = GetOrCreateCell(coord);
                assert(pCell != nullptr);

                assert(pCell->SetEntity(coord, pEntity));
            }
        }
    }

    if (pEntity->m_eEntityKind == EntityKind::Conveyor)
    {
        m_vConveyors.push_back(static_cast<Conveyor*>(pEntity));
    }
    else
    {
        m_vOtherEntities.push_back(pEntity);
    }

    for (int y = position.GetY() - 1; y <= position.GetY() + pEntity->m_size.GetY(); y++)
    {
        for (int x = position.GetX() - 1; x <= position.GetX() + pEntity->m_size.GetX(); x++)
        {
            for (int z = position.GetZ() - 1; z <= position.GetZ() + pEntity->m_size.GetZ(); z++)
            {
                CellCoordinate coord = ToCellSpace({x, y, z});
                if (coord.IsInvalid())
                {
                    continue;
                }

                Cell* pCell = GetOrCreateCell(coord);
                if (!pCell)
                {
                    continue;
                }

                Entity* pEntityInSlot = pCell->GetEntity(coord);
                if (!pEntityInSlot || !pEntityInSlot->RequiresPlacementLocalityChecks())
                {
                    continue;
                }

                if (!m_bSuppressAssess)
                {
                    pEntityInSlot->OnLocalityUpdate(*this);
                }
            }
        }
    }

    return true;
}

bool cpp_conv::WorldMap::ValidateCanPlaceEntity(Vector3 position, Entity* pEntity) const
{
    for (int32_t iXPosition = position.GetX(); iXPosition < (position.GetX() + pEntity->m_size.GetX()); ++iXPosition)
    {
        for (int32_t iYPosition = position.GetY(); iYPosition < (position.GetY() + pEntity->m_size.GetY()); ++
             iYPosition)
        {
            for (int32_t iDepthPosition = position.GetZ(); iDepthPosition < (position.GetZ() + pEntity->m_size.GetZ());
                 ++iDepthPosition)
            {
                const Vector3 checkPosition = {iXPosition, iYPosition, iDepthPosition};
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

cpp_conv::WorldMap::Cell* cpp_conv::WorldMap::GetCell(const CellCoordinate coord) const
{
    if (coord.IsInvalid())
    {
        return nullptr;
    }

    return m_WorldMap[coord.m_CellY][coord.m_CellX].get();
}

cpp_conv::WorldMap::Cell* cpp_conv::WorldMap::GetOrCreateCell(const CellCoordinate coord)
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
