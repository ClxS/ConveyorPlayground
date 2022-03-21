#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include "Conveyor.h"
#include "AtlasScene/ECS/Entity.h"
#include "Eigen/Core"

namespace cpp_conv
{
    class EntityLookupGrid
    {
    public:
        static inline constexpr int32_t c_uiMaximumMapSize = 128;
        static inline constexpr int32_t c_uiCellSize = 64;
        static inline constexpr int32_t c_uiMaximumLevel = 4;

        struct CellCoordinate
        {
            int32_t m_CellX;
            int32_t m_CellY;
            int32_t m_CellSlotX;
            int32_t m_CellSlotY;
            int32_t m_Depth;

            [[nodiscard]] bool IsInvalid() const
            {
                return
                    m_CellX < 0 || m_CellY < 0 ||
                    m_CellX >= c_uiMaximumMapSize || m_CellY >= c_uiMaximumMapSize ||
                    m_CellSlotX < 0 || m_CellSlotY < 0 ||
                    m_CellSlotX >= c_uiCellSize || m_CellSlotY >= c_uiCellSize ||
                    m_Depth < 0 || m_Depth >= c_uiMaximumLevel;
            }
        };

        struct Cell
        {
            using GridRow = std::array<atlas::scene::EntityId, c_uiCellSize>;
            using EntityGrid = std::array<GridRow, c_uiCellSize>;
            using CellFloors = std::array<std::unique_ptr<EntityGrid>, c_uiMaximumLevel>;

            CellFloors m_CellGrid;
            bool m_bHasFloors = false;

            [[nodiscard]] bool HasFloor(uint32_t uiFloor) const;
            bool CreateFloor(uint32_t uiFloor);
            EntityGrid& GetFloor(uint32_t uiFloor);
            [[nodiscard]] const EntityGrid& GetFloor(uint32_t uiFloor) const;
            [[nodiscard]] atlas::scene::EntityId GetEntity(CellCoordinate coord) const;
            bool SetEntity(CellCoordinate coord, atlas::scene::EntityId entity);
        };

    public:
        static CellCoordinate ToCellSpace(Eigen::Vector3i position);

        [[nodiscard]] atlas::scene::EntityId GetEntity(Eigen::Vector3i position) const;
        [[nodiscard]] Cell* GetCell(CellCoordinate coord) const;

        bool PlaceEntity(Eigen::Vector3i position, atlas::scene::EntityId entity);

    private:
        using CellPtr = std::unique_ptr<Cell>;
        using EntityLookupGridRow = std::array<CellPtr, c_uiMaximumMapSize>;
        using EntityLookupGridStore = std::array<EntityLookupGridRow, c_uiMaximumMapSize>;

        Cell* GetOrCreateCell(CellCoordinate coord);
        bool ValidateCanPlaceEntity(Eigen::Vector3i position, atlas::scene::EntityId entity) const;

         bool m_bSuppressAssess = false;

        EntityLookupGridStore m_EntityLookupGrid;
    };
}
