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

            [[nodiscard]] bool IsInvalid() const;
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

        bool PlaceEntity(Eigen::Vector3i position, Eigen::Vector3i size, atlas::scene::EntityId entity);
        bool ValidateCanPlaceEntity(Eigen::Vector3i position, Eigen::Vector3i size,
                                    atlas::scene::EntityId entity) const;

    private:
        using CellPtr = std::unique_ptr<Cell>;
        using EntityLookupGridRow = std::array<CellPtr, c_uiMaximumMapSize>;
        using EntityLookupGridStore = std::array<EntityLookupGridRow, c_uiMaximumMapSize>;

        Cell* GetOrCreateCell(CellCoordinate coord);

        bool m_bSuppressAssess = false;

        EntityLookupGridStore m_EntityLookupGrid;
    };
}
