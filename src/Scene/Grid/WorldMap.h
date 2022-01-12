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
        static inline constexpr uint32_t c_uiMaximumMapSize = 128;
        static inline constexpr uint32_t c_uiCellSize = 64;
        static inline constexpr uint32_t c_uiMaximumLevel = 2;

    private:
        struct CellCoordinate
        {
            int32_t m_CellX;
            int32_t m_CellY;
            int32_t m_CellSlotX;
            int32_t m_CellSlotY;
            int32_t m_Depth;

            bool IsInvalid() const
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

            bool HasFloor(uint32_t uiFloor) const;
            bool CreateFloor(uint32_t uiFloor);
            EntityGrid& GetFloor(uint32_t uiFloor);
            const EntityGrid& GetFloor(uint32_t uiFloor) const;
            Entity* GetEntity(CellCoordinate coord);
            const Entity* GetEntity(CellCoordinate coord) const;
            bool SetEntity(CellCoordinate coord, Entity* pEntity);
        };

    public:
        Entity* GetEntity(Vector3 position);
        const Entity* GetEntity(Vector3 position) const;

        bool PlaceEntity(Vector3 position, Entity* pEntity);

        template<typename T>
        const T* GetEntity(Vector3 position, EntityKind kind) const;

        template<typename T>
        T* GetEntity(Vector3 position, EntityKind kind);

        void Consume(cpp_conv::resources::AssetPtr<cpp_conv::resources::Map> map);

        const std::vector<cpp_conv::Conveyor*> GetConveyors() const { return m_vConveyors; }
        const std::vector<cpp_conv::Entity*> GetOtherEntities() const { return m_vOtherEntities; }
    private:
        using CellPtr = std::unique_ptr<Cell>;
        using WorldMapRow = std::array<CellPtr, c_uiMaximumMapSize>;
        using WorldMapStore = std::array<WorldMapRow, c_uiMaximumMapSize>;

        static CellCoordinate ToCellSpace(Vector3 position);
        Cell* GetCell(CellCoordinate coord) const;
        Cell* GetOrCreateCell(CellCoordinate coord);
        bool ValidateCanPlaceEntity(Vector3 position, Entity* pEntity) const;

        WorldMapStore m_WorldMap;
        std::vector<cpp_conv::Conveyor*> m_vConveyors;
        std::vector<cpp_conv::Entity*> m_vOtherEntities;
    };

    template<typename T>
    const T* cpp_conv::WorldMap::GetEntity(Vector3 position, EntityKind kind) const
    {
        auto pEntity = GetEntity(position);
        if (!pEntity || pEntity->m_eEntityKind != kind)
        {
            return nullptr;
        }

        return reinterpret_cast<const T*>(pEntity);
    }

    template<typename T>
    T* cpp_conv::WorldMap::GetEntity(Vector3 position, EntityKind kind)
    {
        return const_cast<T*>(const_cast<const cpp_conv::WorldMap*>(this)->GetEntity<T>(position, kind));
    }
}
