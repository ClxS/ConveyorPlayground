#include "EntityLookupGrid.h"
#include "Map.h"
#include "Conveyor.h"
#include <cassert>

bool cpp_conv::EntityLookupGrid::Cell::HasFloor(uint32_t uiFloor) const
{
    if (uiFloor >= m_CellGrid.size())
    {
        return false;
    }

    return m_CellGrid[uiFloor] != nullptr;
}

cpp_conv::EntityLookupGrid::Cell::EntityGrid& cpp_conv::EntityLookupGrid::Cell::GetFloor(uint32_t uiFloor)
{
    return *m_CellGrid[uiFloor];
}

const cpp_conv::EntityLookupGrid::Cell::EntityGrid& cpp_conv::EntityLookupGrid::Cell::GetFloor(uint32_t uiFloor) const
{
    return *m_CellGrid[uiFloor];
}

bool cpp_conv::EntityLookupGrid::Cell::CreateFloor(uint32_t uiFloor)
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

atlas::scene::EntityId cpp_conv::EntityLookupGrid::Cell::GetEntity(CellCoordinate coord) const
{
    if (coord.IsInvalid())
    {
        return atlas::scene::EntityId::Invalid();
    }

    if (!HasFloor(coord.m_Depth))
    {
        return atlas::scene::EntityId::Invalid();
    }

    const EntityGrid& rFloor = GetFloor(coord.m_Depth);
    return rFloor[coord.m_CellSlotY][coord.m_CellSlotX];
}

bool cpp_conv::EntityLookupGrid::Cell::SetEntity(const CellCoordinate coord, atlas::scene::EntityId entity)
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
    if (rFloor[coord.m_CellSlotY][coord.m_CellSlotX] != atlas::scene::EntityId::Invalid())
    {
        return false;
    }

    rFloor[coord.m_CellSlotY][coord.m_CellSlotX] = entity;
    return true;
}

cpp_conv::EntityLookupGrid::CellCoordinate cpp_conv::EntityLookupGrid::ToCellSpace(Eigen::Vector3i position)
{
    constexpr int32_t worldToGridSpaceValue = (c_uiMaximumMapSize / 2) * c_uiCellSize;
    const Eigen::Vector3i worldToGridSpaceTransform = { worldToGridSpaceValue, worldToGridSpaceValue, 0 };
    position += worldToGridSpaceTransform;

    constexpr int32_t axisSize = c_uiMaximumMapSize * c_uiCellSize;
    if (position.x() < 0 || position.y() < 0 || position.x() >= axisSize || position.y() >= axisSize)
    {
        return { -1, -1, -1, -1, position.z() };
    }

    const auto iHorizontalCell = position.x() / c_uiCellSize;
    const auto iHorizontalSlot = position.x() % c_uiCellSize;

    const auto iVerticalCell = position.y() / c_uiCellSize;
    const auto iVerticalSlot = position.y() % c_uiCellSize;

    return { iHorizontalCell, iVerticalCell, iHorizontalSlot, iVerticalSlot, position.z() };
}

atlas::scene::EntityId cpp_conv::EntityLookupGrid::GetEntity(const Eigen::Vector3i position) const
{
    const CellCoordinate coord = ToCellSpace(position);
    if (coord.IsInvalid())
    {
        return atlas::scene::EntityId::Invalid();
    }

    const Cell* pCell = GetCell(coord);
    if (!pCell)
    {
        return atlas::scene::EntityId::Invalid();
    }

    return pCell->GetEntity(coord);
}

bool cpp_conv::EntityLookupGrid::PlaceEntity(Eigen::Vector3i position, atlas::scene::EntityId entity)
{
    if (!ValidateCanPlaceEntity(position, entity))
    {
        return false;
    }

    const CellCoordinate coord = ToCellSpace(position);
    assert(!coord.IsInvalid());

    Cell* pCell = GetOrCreateCell(coord);
    assert(pCell != nullptr);

    assert(pCell->SetEntity(coord, entity));

    /*for (int32_t iXPosition = position.GetX(); iXPosition < (position.GetX() + entity->m_size.GetX()); ++iXPosition)
    {
        for (int32_t iYPosition = position.GetY(); iYPosition < (position.GetY() + entity->m_size.GetY()); ++iYPosition)
        {
            for (int32_t iDepthPosition = position.GetZ(); iDepthPosition < (position.GetZ() + entity->m_size.GetZ()); ++iDepthPosition)
            {
                CellCoordinate coord = ToCellSpace({ iXPosition, iYPosition, iDepthPosition });
                assert(!coord.IsInvalid());

                Cell* pCell = GetOrCreateCell(coord);
                assert(pCell != nullptr);

                assert(pCell->SetEntity(coord, entity));
            }
        }
    }*/

    return true;
}

bool cpp_conv::EntityLookupGrid::ValidateCanPlaceEntity(Eigen::Vector3i position, atlas::scene::EntityId pEntity) const
{
    const CellCoordinate coord = ToCellSpace(position);
    if (coord.IsInvalid())
    {
        return false;
    }

    const Cell* pCell = GetCell(coord);
    if (!pCell)
    {
        return true;
    }

    if (pCell->GetEntity(coord) != atlas::scene::EntityId::Invalid())
    {
        return false;
    }

    /*for (int32_t iXPosition = position.GetX(); iXPosition < (position.GetX() + pEntity->m_size.GetX()); ++iXPosition)
    {
        for (int32_t iYPosition = position.GetY(); iYPosition < (position.GetY() + pEntity->m_size.GetY()); ++iYPosition)
        {
            for (int32_t iDepthPosition = position.GetZ(); iDepthPosition < (position.GetZ() + pEntity->m_size.GetZ()); ++iDepthPosition)
            {
                const Vector3 checkPosition = { iXPosition, iYPosition, iDepthPosition };
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
    }*/

    return true;
}

cpp_conv::EntityLookupGrid::Cell* cpp_conv::EntityLookupGrid::GetCell(const CellCoordinate coord) const
{
    if (coord.IsInvalid())
    {
        return nullptr;
    }

    return m_EntityLookupGrid[coord.m_CellY][coord.m_CellX].get();
}

cpp_conv::EntityLookupGrid::Cell* cpp_conv::EntityLookupGrid::GetOrCreateCell(const CellCoordinate coord)
{
    if (coord.IsInvalid())
    {
        return nullptr;
    }

    if (!m_EntityLookupGrid[coord.m_CellY][coord.m_CellX])
    {
        m_EntityLookupGrid[coord.m_CellY][coord.m_CellX] = std::make_unique<Cell>();
    }

    return m_EntityLookupGrid[coord.m_CellY][coord.m_CellX].get();
}
