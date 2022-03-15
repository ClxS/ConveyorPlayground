#pragma once

#include <functional>
#include <map>
#include <memory>
#include <ranges>
#include <tuple>

#include "ComponentRegistry.h"
#include "Entity.h"
#include "Pools.h"

namespace atlas::scene
{
    class EcsManager
    {
        class EntityComponentIterator
        {
        public:
            class Iterator
            {
            public:
                Iterator(const EntityPool& entityPool, const uint64_t componentMask, const bool bIsEnd = false)
                    : m_EntityPool{entityPool}
                    , m_ComponentMask{componentMask}
                    , m_CurrentIndex(bIsEnd ? -1 : FindNextMatchingSlot(-1))
                {

                }

                using iterator_category = std::forward_iterator_tag;
                using difference_type   = std::ptrdiff_t;
                using value_type        = EntityId;
                using pointer           = EntityId;
                using reference         = EntityId;

                reference operator*() const { return m_EntityPool.GetCopy(m_CurrentIndex).m_EntityId; }
                pointer operator->() const { return m_EntityPool.GetCopy(m_CurrentIndex).m_EntityId; }

                // Prefix increment
                Iterator& operator++()
                {
                    m_CurrentIndex = FindNextMatchingSlot(m_CurrentIndex);
                    return *this;
                }

                // Postfix increment
                Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

                friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_CurrentIndex == b.m_CurrentIndex; }
                friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_CurrentIndex != b.m_CurrentIndex; }

            private:
                const EntityPool& m_EntityPool;
                const uint64_t m_ComponentMask;
                int32_t m_CurrentIndex = -1;

                [[nodiscard]] int32_t FindNextMatchingSlot(const int32_t current) const
                {
                    for(int i = current + 1; i < m_EntityPool.Size(); ++i)
                    {
                        if ((m_EntityPool.GetCopy(i).m_PossessedComponentMask & m_ComponentMask) == m_ComponentMask)
                        {
                            return i;
                        }
                    }

                    return -1;
                }
            };

            EntityComponentIterator(const EntityPool& entityPool, const uint64_t componentMask)
                : m_EntityPool{entityPool}
                , m_ComponentMask{componentMask}
            {
            }

            Iterator begin() { return { m_EntityPool, m_ComponentMask, false }; }
            Iterator end()   { return { m_EntityPool, m_ComponentMask, true }; }

        private:
            const EntityPool& m_EntityPool;
            uint64_t m_ComponentMask;
        };

    public:
        EcsManager()
        {
            uint64_t uiMapHash = 1;
            for(const auto& [typeHash, poolFactory] : ComponentRegistry::GetComponentPoolFactories())
            {
                m_Components[typeHash].m_MapMask = uiMapHash;
                m_Components[typeHash].m_Pool = poolFactory();

                uiMapHash = uiMapHash << 1;
            }
        }

        EntityId AddEntity()
        {
            const EntityId id { m_EntityIndices.Size() };
            m_EntityIndices.Push(m_EntityPool.Size());
            m_EntityPool.Push({ id, 0 });
            for(const auto& [m_MapMask, m_Pool] : m_Components | std::views::values)
            {
                m_Pool->PushEmpty();
            }

            return id;
        }

        void RemoveEntity(const EntityId entity)
        {
            const auto removedIndex = m_EntityIndices.GetCopy(entity.m_Value);

            if (m_EntityPool.Size() > 1)
            {
                const auto endEntityId = m_EntityPool.GetCopy(m_EntityPool.Size() - 1).m_EntityId;
                if (endEntityId != entity)
                {
                    m_EntityIndices.Remove(entity.m_Value);
                    m_EntityIndices.Set(endEntityId.m_Value, removedIndex);
                    m_EntityPool.SwapAndPop(removedIndex);
                    for(const auto& [m_HashMapIndex, m_Pool] : m_Components | std::views::values)
                    {
                        m_Pool->SwapAndPop(removedIndex);
                    }
                }
                else
                {
                    m_EntityIndices.Remove(entity.m_Value);
                    m_EntityPool.Pop();
                    for(const auto& [m_HashMapIndex, m_Pool] : m_Components | std::views::values)
                    {
                        m_Pool->Pop();
                    }
                }
            }
            else
            {
                m_EntityIndices.Remove(entity.m_Value);
                m_EntityPool.Pop();
                for(const auto& [m_HashMapIndex, m_Pool] : m_Components | std::views::values)
                {
                    m_Pool->Pop();
                }
            }
        }

        template<typename TComponent, typename... TArgs>
        TComponent& AddComponent(const EntityId entity, TArgs&& ...args)
        {
            const auto itComponentPool = m_Components.find(typeid(TComponent).hash_code());
            assert(itComponentPool != m_Components.end());

            auto entityIndex = m_EntityIndices.GetCopy(entity.m_Value);

            auto& [entityId, possessedComponentMask] = m_EntityPool.GetReference(entityIndex);
            const uint64_t uiMask = (*itComponentPool).second.m_MapMask;
            assert((possessedComponentMask & uiMask) == 0);

            auto* typedComponentPool = static_cast<ComponentPool<TComponent>*>(itComponentPool->second.m_Pool.get());

            possessedComponentMask |= uiMask;
            typedComponentPool->PlacementSet(entityIndex, std::forward<TArgs&&>(args)...);

            return typedComponentPool->GetReference(entityIndex);
        }

        template<typename TComponent>
        TComponent& AddComponent(const EntityId entity, TComponent&& value)
        {
            const auto itComponentPool = m_Components.find(typeid(TComponent).hash_code());
            assert(itComponentPool != m_Components.end());

            auto entityIndex = m_EntityIndices.GetCopy(entity.m_Value);

            auto& [entityId, possessedComponentMask] = m_EntityPool.GetReference(entityIndex);
            const uint64_t uiMask = (*itComponentPool).second.m_MapMask;
            assert((possessedComponentMask & uiMask) == 0);

            auto* typedComponentPool = static_cast<ComponentPool<TComponent>*>(itComponentPool->second.m_Pool.get());

            possessedComponentMask |= uiMask;
            typedComponentPool->Set(entityIndex, std::forward<TComponent&&>(value));

            return typedComponentPool->GetReference(entityIndex);
        }

        template<typename TComponent>
        [[nodiscard]] bool DoesEntityHaveComponent(const EntityId entity)
        {
            const auto it = m_Components.find(typeid(TComponent).hash_code());
            assert(it != m_Components.end());

            return (m_EntityPool.GetCopy(m_EntityIndices.GetCopy(entity.m_Value)).m_PossessedComponentMask & it->second.m_MapMask) != 0;
        }

        template<typename TComponent, typename... TOtherComponents>
        [[nodiscard]] EntityComponentIterator GetEntitiesWithComponents()
        {
            const uint64_t componentMask = MaskLookup<TComponent, TOtherComponents...>::GetComponentMask(m_Components);
            return { m_EntityPool, componentMask };
        }

        template<typename TComponent>
        TComponent& GetComponent(const EntityId entityId)
        {
            const auto it = m_Components.find(typeid(TComponent).hash_code());
            assert(it != m_Components.end());

            ComponentPool<TComponent>* pTypedPool = static_cast< ComponentPool<TComponent>*>(it->second.m_Pool.get());
            return pTypedPool->GetReference(m_EntityIndices.GetCopy(entityId.m_Value));
        }

        template<typename TComponent>
        const TComponent& GetComponent(const EntityId entityId) const
        {
            const auto it = m_Components.find(typeid(TComponent).hash_code());
            assert(it != m_Components.end());

            const ComponentPool<TComponent>* pTypedPool = static_cast<const ComponentPool<TComponent>*>(it->second.m_Pool.get());
            return pTypedPool->GetReference(m_EntityIndices.GetCopy(entityId.m_Value));
        }

        template<typename... TComponent>
        std::tuple<TComponent&...> GetComponents(const EntityId entityId)
        {
            return { GetComponent<TComponent>(entityId)... };
        }

        template<typename... TComponent>
        std::tuple<const TComponent&...> GetComponents(const EntityId entityId) const
        {
            return { GetComponent<const TComponent>(entityId)... };
        }

        template<typename... TComponent>
        void ForEachComponents(std::function<void(TComponent&...)> callback)
        {
            for(auto entity : GetEntitiesWithComponents<TComponent...>())
            {
                callback(GetComponent<TComponent>(entity)...);
            }
        }

        template<typename... TComponent>
        void ForEachComponents(std::function<bool(const TComponent&...)> callback) const
        {
            for(auto entity : GetEntitiesWithComponents<TComponent...>())
            {
                if (!callback(GetComponent<const TComponent>(entity)...))
                {
                    break;
                }
            }
        }

    private:
        struct MaskedComponentPool
        {
            uint64_t m_MapMask {0};
            std::unique_ptr<PoolBase> m_Pool;
        };

        template<typename...>
        struct MaskLookup;

        template<typename TComponent, typename... TOtherComponents>
        struct MaskLookup<TComponent, TOtherComponents...>
        {
            static uint64_t GetComponentMask(const std::map<size_t, MaskedComponentPool>& componentSet)
            {
                const auto it = componentSet.find(typeid(TComponent).hash_code());
                assert(it != componentSet.end());

                uint64_t uiMask = it->second.m_MapMask;
                uiMask |= MaskLookup<TOtherComponents...>::GetComponentMask(componentSet);

                return uiMask;
            }
        };

        template<>
        struct MaskLookup<>
        {
            static uint64_t GetComponentMask(const std::map<size_t, MaskedComponentPool>&) { return 0; }
        };

        EntityIndicesPool m_EntityIndices{};
        EntityPool m_EntityPool{};
        std::map<size_t, MaskedComponentPool> m_Components{};
    };
}
