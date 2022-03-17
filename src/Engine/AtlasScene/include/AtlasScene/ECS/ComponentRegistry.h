#pragma once

#include <map>
#include <memory>
#include <unordered_map>

#include "Pools.h"

namespace atlas::scene
{
    class ComponentRegistry
    {
    public:
        using PoolFactory = scene::PoolBase* (*)(void);

        ComponentRegistry() = delete;
        ~ComponentRegistry() = delete;
        ComponentRegistry(const ComponentRegistry&) = delete;
        ComponentRegistry(const ComponentRegistry&&) = delete;
        ComponentRegistry operator=(const ComponentRegistry&) = delete;
        ComponentRegistry operator=(const ComponentRegistry&&) = delete;

        template<typename TComponent>
        static void RegisterComponent()
        {
            ComponentIndexAdapter<TComponent>::m_Index = static_cast<int32_t>(m_ComponentPoolFactory.size());
            m_ComponentPoolFactory.push_back([]() { return static_cast<PoolBase*>(new ComponentPool<TComponent>()); });
        }

        static const std::vector<PoolFactory>& GetComponentPoolFactories()
        {
            return m_ComponentPoolFactory;
        }

        static const PoolFactory& GetFactoryForPoolWithMask(const uint64_t poolIndex)
        {
            return m_ComponentPoolFactory[poolIndex];
        }

        template<typename TComponent>
        static uint64_t GetComponentMask()
        {
            assert(ComponentIndexAdapter<TComponent>::m_Index != -1); // Component has not been registered
            return 1ULL << ComponentIndexAdapter<TComponent>::m_Index;
        }

    private:
        template<typename TComponent>
        struct ComponentIndexAdapter
        {
            inline static int32_t m_Index = -1;
        };

        inline static std::vector<PoolFactory> m_ComponentPoolFactory;
    };
}
