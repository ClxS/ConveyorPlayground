#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include "Entity.h"
#include "AssetPtr.h"
#include "Conveyor.h"

namespace cpp_conv { namespace resources { class Map; } }

namespace cpp_conv
{
    class WorldMap
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
            using GridRow = std::array<std::unique_ptr<Entity>, c_uiCellSize>;
            using EntityGrid = std::array<GridRow, c_uiCellSize>;
            using CellFloors = std::array<std::unique_ptr<EntityGrid>, c_uiMaximumLevel>;

            CellFloors m_CellGrid;
            bool m_bHasFloors = false;

            [[nodiscard]] bool HasFloor(uint32_t uiFloor) const;
            bool CreateFloor(uint32_t uiFloor);
            EntityGrid& GetFloor(uint32_t uiFloor);
            [[nodiscard]] const EntityGrid& GetFloor(uint32_t uiFloor) const;
            Entity* GetEntity(CellCoordinate coord);
            [[nodiscard]] const Entity* GetEntity(CellCoordinate coord) const;
            bool SetEntity(CellCoordinate coord, Entity* pEntity);
        };

    public:
        static CellCoordinate ToCellSpace(Vector3 position);

        [[nodiscard]] Entity* GetEntity(Vector3 position) const;
        [[nodiscard]] Cell* GetCell(CellCoordinate coord) const;

        bool PlaceEntity(Vector3 position, Entity* pEntity);

        template<typename T>
        const T* GetEntity(Vector3 position, EntityKind kind) const;

        template<typename T>
        T* GetEntity(Vector3 position, EntityKind kind);

        void Consume(const resources::AssetPtr<resources::Map>& map);

        void PopulateCorners();

        [[nodiscard]] std::vector<Conveyor*> GetConveyors() const { return m_vConveyors; }
        [[nodiscard]] std::vector<Conveyor*> GetCornerConveyors() const { return m_vCornerConveyors; }
        [[nodiscard]] std::vector<Entity*> GetOtherEntities() const { return m_vOtherEntities; }
    private:
        using CellPtr = std::unique_ptr<Cell>;
        using WorldMapRow = std::array<CellPtr, c_uiMaximumMapSize>;
        using WorldMapStore = std::array<WorldMapRow, c_uiMaximumMapSize>;

        Cell* GetOrCreateCell(CellCoordinate coord);
        bool ValidateCanPlaceEntity(Vector3 position, Entity* pEntity) const;

         bool m_bSuppressAssess = false;

        WorldMapStore m_WorldMap;
        std::vector<Conveyor*> m_vConveyors;
        std::vector<Conveyor*> m_vCornerConveyors;
        std::vector<Entity*> m_vOtherEntities;
    };

    template<typename T>
    const T* WorldMap::GetEntity(Vector3 position, EntityKind kind) const
    {
        auto pEntity = GetEntity(position);
        if (!pEntity || pEntity->m_eEntityKind != kind)
        {
            return nullptr;
        }

        return reinterpret_cast<const T*>(pEntity);
    }

    template<typename T>
    T* WorldMap::GetEntity(Vector3 position, EntityKind kind)
    {
        return const_cast<T*>(const_cast<const WorldMap*>(this)->GetEntity<T>(position, kind));
    }
}
