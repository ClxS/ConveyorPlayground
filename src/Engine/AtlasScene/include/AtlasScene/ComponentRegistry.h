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
        using PoolBasePtr = std::unique_ptr<PoolBase>;
        typedef PoolBasePtr (*PoolFactory)(void);

        ComponentRegistry() = delete;
        ~ComponentRegistry() = delete;
        ComponentRegistry(const ComponentRegistry&) = delete;
        ComponentRegistry(const ComponentRegistry&&) = delete;
        ComponentRegistry operator=(const ComponentRegistry&) = delete;
        ComponentRegistry operator=(const ComponentRegistry&&) = delete;

        template<typename TComponent>
        static void RegisterComponent()
        {
            m_ComponentPoolFactory[typeid(TComponent).hash_code()] = []() { return std::unique_ptr<PoolBase>(new ComponentPool<TComponent>()); };
        }

        static void Reset()
        {
            m_ComponentPoolFactory.clear();
        }

        static const std::unordered_map<size_t, PoolFactory>& GetComponentPoolFactories()
        {
            return m_ComponentPoolFactory;
        }

    private:

        inline static std::unordered_map<size_t, PoolFactory> m_ComponentPoolFactory;
    };
}
