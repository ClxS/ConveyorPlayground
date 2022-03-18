#pragma once

#include <algorithm>
#include <bitset>
#include <memory>
#include <ranges>
#include <vector>

#include "AtlasScene/ECS/Utilities/TemplatedUniquenessCounter.h"
#include "SystemBase.h"

namespace atlas::scene
{
    class SystemsBuilder;

    class SystemsBuilder
    {
        template<typename TSystem>
        int32_t GetSystemIndex();

        template<typename TSystem>
        int32_t GetMinimumDependencyIndex();

    public:
        ~SystemsBuilder()
        {
            for(const auto& system : m_Systems)
            {
                delete system.m_pSystem;
            }

            m_Systems.clear();
        }

        template<typename TSystem, typename ... DependsOn, typename... TArgs>
        void RegisterSystem(TArgs&&...);

    private:
        friend class SystemsManager;

        struct SystemRegistration
        {
            SystemRegistration(SystemBase* pSystem, const int32_t value, std::vector<int32_t> dependencies);

            SystemBase* m_pSystem;
            int32_t m_UniquenessValue;
            std::vector<int32_t> m_Dependencies;
        };

        std::vector<SystemRegistration> m_Systems;
    };

    class SystemsManager
    {
    public:
        ~SystemsManager()
        {
            for(const auto& system : m_Systems)
            {
                delete system;
            }

            m_Systems.clear();
        }

        void Initialise(SystemsBuilder& builder, EcsManager& ecsManager)
        {
            for(auto& system : builder.m_Systems)
            {
                m_Systems.push_back(system.m_pSystem);
            }

            builder.m_Systems.clear();
            for(const auto& system : m_Systems)
            {
                system->Initialise(ecsManager);
            }
        }

        void Update(EcsManager& ecsManager) const
        {
            for(const auto& system : m_Systems)
            {
                system->Update(ecsManager);
            }
        }

    private:
        std::vector<SystemBase*> m_Systems;
    };

    template <typename TSystem>
    int32_t SystemsBuilder::GetSystemIndex()
    {
        // Slow but infrequently used
        const int32_t uniquenessValue = utils::TemplatedUniquenessCounter<TSystem, SystemsBuilder>::GetTypeValue();
        for (size_t i = 0; i < m_Systems.size(); ++i)
        {
            if (m_Systems[i].m_UniquenessValue == uniquenessValue)
            {
                return static_cast<int32_t>(i);
            }
        }

        return -1;
    }

    template <typename TSystem>
    int32_t SystemsBuilder::GetMinimumDependencyIndex()
    {
        const int32_t uniquenessValue = utils::TemplatedUniquenessCounter<TSystem, SystemsBuilder>::GetTypeValue();

        for (size_t i = 0; i < m_Systems.size(); ++i)
        {
            for(const auto dependencyValue : m_Systems[i].m_Dependencies)
            {
                if (dependencyValue == uniquenessValue)
                {
                    return static_cast<int32_t>(i);
                }
            }
        }

        return static_cast<int32_t>(m_Systems.size());
    }

    template <typename TSystem, typename ... DependsOn, typename... TArgs>
    void SystemsBuilder::RegisterSystem(TArgs&&... args)
    {
        utils::TemplatedUniquenessCounter<TSystem, SystemsBuilder>::Ensure();

        std::vector<int32_t> dependencyIndices { GetSystemIndex<DependsOn>()... };
        std::erase(dependencyIndices, -1);

        const int32_t minimumIndex = dependencyIndices.empty() ? -1 : *std::ranges::min_element(dependencyIndices);
        const int32_t maximumIndex = GetMinimumDependencyIndex<TSystem>();

        assert(minimumIndex < maximumIndex); // If we hit this, we have a circular dependency issue

        m_Systems.insert(
            m_Systems.begin() + maximumIndex,
            SystemRegistration(
                new TSystem(std::forward<TArgs&&>(args)...),
                utils::TemplatedUniquenessCounter<TSystem, SystemsManager>::Ensure(),
                { utils::TemplatedUniquenessCounter<DependsOn, SystemsManager>::GetTypeValue()... }));
    }

    inline SystemsBuilder::SystemRegistration::SystemRegistration(SystemBase* pSystem, const int32_t value,
        std::vector<int32_t> dependencies)
        : m_pSystem{pSystem}
        , m_UniquenessValue{value}
        , m_Dependencies{std::move(dependencies)}
    {
    }
}
