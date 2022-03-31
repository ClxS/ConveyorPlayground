#pragma once

#include <functional>
#include <map>
#include <memory>
#include <tuple>

#include "ComponentRegistry.h"
#include "Pools.h"
#include "AtlasScene/ECS/Entity.h"

namespace atlas::scene
{
    class EcsManager
    {
    public:
        EcsManager();
        EcsManager(const EcsManager&) = delete;
        EcsManager(const EcsManager&&) = delete;
        EcsManager operator=(const EcsManager&) = delete;
        EcsManager operator=(const EcsManager&&) = delete;

        ~EcsManager();

        EntityId AddEntity();

        void RemoveEntity(EntityId entity);

        template <typename TComponent, typename... TArgs>
        TComponent& AddComponent(EntityId entity, TArgs&& ...args);

        template <typename TComponent>
        TComponent& AddComponent(EntityId entity, TComponent&& value);

        template <typename TComponent>
        void RemoveComponent(const EntityId entity);

        template <typename TComponent>
        [[nodiscard]] bool DoesEntityHaveComponent(EntityId entity) const;

        template <typename TComponent, typename... TOtherComponents>
        bool DoesEntityHaveComponents(EntityId entity) const;


        [[nodiscard]] std::vector<EntityId> GetEmptyEntities() const
        {
            return m_ArchetypePools[0].m_EntityPool.GetData();
        }

        template <typename TComponent, typename... TOtherComponents>
        [[nodiscard]] std::vector<EntityId> GetEntitiesWithComponents() const;

        template <typename TComponent>
        [[nodiscard]] TComponent& GetComponent(EntityId entityId);

        template <typename TComponent>
        [[nodiscard]] const TComponent& GetComponent(EntityId entityId) const;

        template <typename... TComponent>
        [[nodiscard]] std::tuple<TComponent&...> GetComponents(EntityId entityId);

        template <typename... TComponent>
        [[nodiscard]] std::tuple<const TComponent&...> GetComponents(EntityId entityId) const;

        template <typename... TComponent>
        void ForEachComponents(std::function<void(EntityId, TComponent&...)> callback);

        template <typename... TComponent>
        void ForEachComponents(std::function<bool(EntityId, const TComponent&...)> callback) const;

    private:
        struct MaskedComponentPool
        {
            uint64_t m_PoolMask;
            PoolBase* m_Pool;
        };

        struct ArchetypePool
        {
            explicit ArchetypePool(const uint64_t archetypeComponentMask)
                : m_ArchetypeComponentMask{archetypeComponentMask}
            {
            }

            ArchetypePool(const ArchetypePool& other) = delete;
            ArchetypePool(ArchetypePool&& other) = default;
            ArchetypePool operator=(const ArchetypePool& other) = delete;
            ArchetypePool operator=(ArchetypePool&& other) = delete;

            uint64_t m_ArchetypeComponentMask = 0;
            EntityPool m_EntityPool{};
            std::vector<MaskedComponentPool> m_Components;

            MaskedComponentPool& GetComponentPool(const uint64_t componentMask)
            {
                const uint64_t componentIndex = std::popcount(m_ArchetypeComponentMask & (componentMask - 1));
                return m_Components[componentIndex];
            }

            const MaskedComponentPool& GetComponentPool(const uint64_t componentMask) const
            {
                const uint64_t componentIndex = std::popcount(m_ArchetypeComponentMask & (componentMask - 1));
                return m_Components[componentIndex];
            }
        };

        template <typename...>
        struct MaskLookup;

        template <typename TComponent, typename... TOtherComponents>
        struct MaskLookup<TComponent, TOtherComponents...>
        {
            static uint64_t GetComponentMask();
        };

        template <>
        struct MaskLookup<>
        {
            static uint64_t GetComponentMask() { return 0; }
        };

        template <typename TComponent, typename ... TArgs>
        TComponent& DoAddComponent(EntityId entity,
                                   std::function<TComponent*(ComponentPool<TComponent>*)> setInputEntity);

        ArchetypeIndex GetOrCreateArchetype(uint64_t archetypeMask);

        ArchetypePool& GetPool(const ArchetypeIndex index)
        {
            return m_ArchetypePools[index.m_ArchetypeIndex];
        }

        const ArchetypePool& GetPool(const ArchetypeIndex index) const
        {
            return m_ArchetypePools[index.m_ArchetypeIndex];
        }

        EntityIndicesPool m_EntityIndices{};
        std::vector<ArchetypePool> m_ArchetypePools;
    };

    template <typename TComponent, typename ... TArgs>
    TComponent& EcsManager::DoAddComponent(const EntityId entity,
                                           std::function<TComponent*(ComponentPool<TComponent>*)> setInputEntity)
    {
        const auto [oldEntityIndex, oldArchetypeIndex] = m_EntityIndices.GetCopy(entity.m_Value);
        assert(oldEntityIndex >= 0);
        assert(oldArchetypeIndex.m_ArchetypeIndex >= 0);

        ArchetypeIndex newArchetypeIndex = ArchetypeIndex::Empty();
        {
            const uint64_t newMask = m_ArchetypePools[oldArchetypeIndex.m_ArchetypeIndex].m_ArchetypeComponentMask |
                ComponentRegistry::GetComponentMask<TComponent>();
            newArchetypeIndex = GetOrCreateArchetype(newMask);
        }

        // Important! We need to re-obtain the old pool in case the GetOrCreateArchetype caused an expansion (and thus moved it)
        ArchetypePool& oldPool = GetPool(oldArchetypeIndex);

        auto& [newArchetypeMask, newEntityPool, newComponentPools] = GetPool(newArchetypeIndex);

        m_EntityIndices.Set(entity.m_Value, EntityIndex{newEntityPool.Size(), newArchetypeIndex});

        TComponent* returnValue = nullptr;

        for (auto [poolMask, componentPool] : newComponentPools)
        {
            if (oldPool.m_ArchetypeComponentMask & poolMask)
            {
                componentPool->ClaimFromOtherPool(oldPool.GetComponentPool(poolMask).m_Pool, oldEntityIndex);
            }
            else
            {
                auto* typedComponentPool = static_cast<ComponentPool<TComponent>*>(componentPool);
                returnValue = setInputEntity(typedComponentPool);
            }
        }

        newEntityPool.Push({entity});
        if (oldPool.m_EntityPool.Size() > 1 && oldEntityIndex != oldPool.m_EntityPool.Size() - 1)
        {
            oldPool.m_EntityPool.SwapAndPop(oldEntityIndex);
            m_EntityIndices.Set(
                oldPool.m_EntityPool.GetCopy(oldEntityIndex).m_Value,
            {
                    oldEntityIndex,
                    oldArchetypeIndex
                });
        }
        else
        {
            oldPool.m_EntityPool.Pop();
        }

        assert(returnValue);
        return *returnValue;
    }

    template <typename TComponent, typename ... TArgs>
    TComponent& EcsManager::AddComponent(const EntityId entity, TArgs&&... args)
    {
        return DoAddComponent<TComponent>(entity, [&](ComponentPool<TComponent>* pool)
        {
            return &pool->Push(std::forward<TArgs&&>(args)...);
        });
    }

    template <typename TComponent>
    TComponent& EcsManager::AddComponent(const EntityId entity, TComponent&& value)
    {
        return DoAddComponent<TComponent>(entity, [&](ComponentPool<TComponent>* pool)
        {
            return &pool->Push(value);
        });
    }

    template <typename TComponent>
    void EcsManager::RemoveComponent(const EntityId entity)
    {
        const auto [oldEntityIndex, oldArchetypeIndex] = m_EntityIndices.GetCopy(entity.m_Value);
        assert(oldEntityIndex >= 0);
        assert(oldArchetypeIndex.m_ArchetypeIndex >= 0);

        const uint64_t removedComponentMask = ComponentRegistry::GetComponentMask<TComponent>();
        ArchetypeIndex newArchetypeIndex = ArchetypeIndex::Empty();
        {
            const uint64_t currentMask = m_ArchetypePools[oldArchetypeIndex.m_ArchetypeIndex].m_ArchetypeComponentMask;
            assert((currentMask & removedComponentMask) != 0);

            const uint64_t newMask = currentMask & (~removedComponentMask);
            newArchetypeIndex = GetOrCreateArchetype(newMask);
        }

        // Important! We need to re-obtain the old pool in case the GetOrCreateArchetype caused an expansion (and thus moved it)
        ArchetypePool& oldPool = GetPool(oldArchetypeIndex);

        auto& [newArchetypeMask, newEntityPool, newComponentPools] = GetPool(newArchetypeIndex);

        m_EntityIndices.Set(entity.m_Value, EntityIndex{newEntityPool.Size(), newArchetypeIndex});

        // Move components over to new archetype
        for (auto [poolMask, componentPool] : newComponentPools)
        {
            assert((oldPool.m_ArchetypeComponentMask & poolMask) != 0);
            componentPool->ClaimFromOtherPool(oldPool.GetComponentPool(poolMask).m_Pool, oldEntityIndex);
        }

        // Remove from the removed component type, as the above wont have migrated it.
        {
            auto* pOtherPool = oldPool.GetComponentPool(removedComponentMask).m_Pool;
            if (pOtherPool->Size() > 1 || oldEntityIndex != pOtherPool->Size() - 1)
            {
                pOtherPool->SwapAndPop(oldEntityIndex);
            }
            else
            {
                pOtherPool->Pop();
            }
        }

        newEntityPool.Push({entity});
        if (oldPool.m_EntityPool.Size() > 1 && oldEntityIndex != oldPool.m_EntityPool.Size() - 1)
        {
            oldPool.m_EntityPool.SwapAndPop(oldEntityIndex);
            m_EntityIndices.Set(
                oldPool.m_EntityPool.GetCopy(oldEntityIndex).m_Value,
            {
                    oldEntityIndex,
                    oldArchetypeIndex
                });
        }
        else
        {
            oldPool.m_EntityPool.Pop();
        }
    }

    template <typename TComponent>
    bool EcsManager::DoesEntityHaveComponent(const EntityId entity) const
    {
        const auto [_, oldArchetypeIndex] = m_EntityIndices.GetCopy(entity.m_Value);
        assert(oldArchetypeIndex.m_ArchetypeIndex >= 0);

        const ArchetypePool& pool = GetPool(oldArchetypeIndex);

        const uint64_t targetMask = ComponentRegistry::GetComponentMask<TComponent>();
        return pool.m_ArchetypeComponentMask & targetMask;
    }


    template <typename TComponent, typename ... TOtherComponents>
    bool EcsManager::DoesEntityHaveComponents(const EntityId entity) const
    {
        const auto [_, oldArchetypeIndex] = m_EntityIndices.GetCopy(entity.m_Value);
        assert(oldArchetypeIndex.m_ArchetypeIndex >= 0);
        const ArchetypePool& pool = GetPool(oldArchetypeIndex);

        const uint64_t uiMask = MaskLookup<TComponent, TOtherComponents...>::GetComponentMask();
        return (pool.m_ArchetypeComponentMask & uiMask) == uiMask;
    }

    template <typename TComponent, typename ... TOtherComponents>
    std::vector<EntityId> EcsManager::GetEntitiesWithComponents() const
    {
        std::vector<EntityId> entities{};
        const uint64_t uiMask = MaskLookup<TComponent, TOtherComponents...>::GetComponentMask();
        for (const auto& archetype : m_ArchetypePools)
        {
            if ((archetype.m_ArchetypeComponentMask & uiMask) == uiMask)
            {
                auto& data = archetype.m_EntityPool.GetData();
                entities.insert(entities.end(), data.begin(), data.end());
            }
        }

        return entities;
    }

    template <typename TComponent>
    TComponent& EcsManager::GetComponent(const EntityId entityId)
    {
        const auto [entityIndex, oldArchetypeIndex] = m_EntityIndices.GetCopy(entityId.m_Value);
        assert(oldArchetypeIndex.m_ArchetypeIndex >= 0);

        auto& archetypePool = GetPool(oldArchetypeIndex);
        auto& componentPool = archetypePool.GetComponentPool(ComponentRegistry::GetComponentMask<TComponent>());
        ComponentPool<TComponent>* pTypedPool = static_cast<ComponentPool<TComponent>*>(componentPool.m_Pool);
        return pTypedPool->GetReference(entityIndex);
    }

    template <typename TComponent>
    const TComponent& EcsManager::GetComponent(const EntityId entityId) const
    {
        const auto [_, oldArchetypeIndex] = m_EntityIndices.GetCopy(entityId.m_Value);
        assert(oldArchetypeIndex.m_ArchetypeIndex >= 0);

        const auto& archetypePool = GetPool(oldArchetypeIndex);
        const auto& componentPool = archetypePool.GetComponentPool(ComponentRegistry::GetComponentMask<TComponent>());
        ComponentPool<TComponent>* pTypedPool = static_cast<ComponentPool<TComponent>*>(componentPool.m_Pool);
        return pTypedPool->GetReference(m_EntityIndices.GetCopy(entityId.m_Value).m_EntityIndex);
    }

    template <typename ... TComponent>
    std::tuple<TComponent&...> EcsManager::GetComponents(const EntityId entityId)
    {
        return {GetComponent<TComponent>(entityId)...};
    }

    template <typename ... TComponent>
    std::tuple<const TComponent&...> EcsManager::GetComponents(const EntityId entityId) const
    {
        return {GetComponent<TComponent>(entityId)...};
    }

    template <typename ... TComponent>
    void EcsManager::ForEachComponents(std::function<void(EntityId, TComponent&...)> callback)
    {
        for (auto entity : GetEntitiesWithComponents<TComponent...>())
        {
            callback(entity, GetComponent<TComponent>(entity)...);
        }
    }

    template <typename ... TComponent>
    void EcsManager::ForEachComponents(std::function<bool(EntityId, const TComponent&...)> callback) const
    {
        for (auto entity : GetEntitiesWithComponents<TComponent...>())
        {
            if (!callback(entity, GetComponent<const TComponent>(entity)...))
            {
                break;
            }
        }
    }

    template <typename TComponent, typename ... TOtherComponents>
    uint64_t EcsManager::MaskLookup<TComponent, TOtherComponents...>::GetComponentMask()
    {
        return ComponentRegistry::GetComponentMask<TComponent>() | MaskLookup<TOtherComponents...>::GetComponentMask();
    }
}
