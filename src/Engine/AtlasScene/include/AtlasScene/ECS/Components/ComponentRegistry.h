#pragma once

#include <map>
#include <memory>
#include <unordered_map>

#include "Pools.h"
#include "AtlasCore/TemplatedUniquenessCounter.h"

namespace atlas::scene
{
    class ComponentRegistry
    {
    public:
        using PoolFactory = PoolBase* (*)(void);

        ComponentRegistry() = delete;
        ~ComponentRegistry() = delete;
        ComponentRegistry(const ComponentRegistry&) = delete;
        ComponentRegistry(const ComponentRegistry&&) = delete;
        ComponentRegistry operator=(const ComponentRegistry&) = delete;
        ComponentRegistry operator=(const ComponentRegistry&&) = delete;

        template <typename TComponent>
        static void RegisterComponent()
        {
            core::TemplatedUniquenessCounter<TComponent, ComponentRegistry>::Ensure();
            m_ComponentPoolFactory.push_back([]() { return static_cast<PoolBase*>(new ComponentPool<TComponent>()); });
        }

        static const std::vector<PoolFactory>& GetComponentPoolFactories()
        {
            return m_ComponentPoolFactory;
        }

        static const PoolFactory& GetFactoryForPoolWithMask(const uint64_t mask)
        {
            auto index = std::countr_zero(mask);
            return m_ComponentPoolFactory[index];
        }

        template <typename TComponent>
        static uint64_t GetComponentMask()
        {
            const uint64_t index = core::TemplatedUniquenessCounter<TComponent, ComponentRegistry>::GetTypeValue();
            assert(index != -1); // Component has not been registered
            return 1ULL << index;
        }

    private:
        inline static std::vector<PoolFactory> m_ComponentPoolFactory;
    };
}
