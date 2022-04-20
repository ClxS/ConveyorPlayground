#pragma once

#include <algorithm>
#include <bitset>
#include <format>
#include <iostream>
#include <memory>
#include <ranges>
#include <vector>

#include "SystemBase.h"
#include "AtlasCore/TemplatedUniquenessCounter.h"

namespace atlas::scene
{
    class SystemsBuilder;

    class SystemsBuilder
    {
    public:
        ~SystemsBuilder()
        {
            for (const auto& system : m_Systems)
            {
                delete system.m_pSystem;
            }

            m_Systems.clear();
        }

        struct GroupId
        {
            int32_t m_GroupId = 0;
        };

        template <typename TSystem, typename ... DependsOn, typename... TArgs>
        void RegisterSystem(std::vector<GroupId> dependentGroups, TArgs&&...);

        template <typename TSystem, typename ... DependsOn, typename... TArgs>
        void RegisterSystem(TArgs&&...);

        template <typename ... DependsOn>
        GroupId RegisterGroup(std::string, std::function<void(SystemsBuilder&)> callback);

        template <typename ... DependsOn>
        GroupId RegisterGroup(std::string, std::vector<GroupId> dependentGroups,
                              std::function<void(SystemsBuilder&)> callback);


    private:
        friend class SystemsManager;

        template <typename TSystem>
        int32_t GetSystemIndex();
        int32_t GetMinimumDependencyIndex(int32_t uniquenessValue) const;
        int32_t GetMaximumGroupIndex(std::vector<GroupId> dependentGroups) const;

        struct SystemRegistration
        {
            SystemRegistration(std::string systemName, SystemBase* pSystem, GroupId groupId, int32_t value,
                               std::vector<int32_t> dependencies);

            std::string m_SystemName;
            SystemBase* m_pSystem;
            GroupId m_GroupId;
            int32_t m_UniquenessValue;
            std::vector<int32_t> m_Dependencies;
        };

        std::vector<SystemRegistration> m_Systems;
        int32_t m_LatestGroupId = 0;
    };

    class SystemsManager
    {
    public:
        ~SystemsManager()
        {
            for (const auto& system : m_Systems)
            {
                delete system;
            }

            m_Systems.clear();
        }

        void Initialise(SystemsBuilder& builder, EcsManager& ecsManager)
        {
            std::cout << "Systems Order:";
            for (auto& system : builder.m_Systems)
            {
                std::cout << std::format("\t{}", system.m_SystemName);
                m_Systems.push_back(system.m_pSystem);
            }

            builder.m_Systems.clear();
            for (const auto& system : m_Systems)
            {
                system->Initialise(ecsManager);
            }
        }

        void Update(EcsManager& ecsManager) const
        {
            for (const auto& system : m_Systems)
            {
                Update(ecsManager, system);
            }
        }

        static void Update(EcsManager& ecsManager, SystemBase* system)
        {
            system->Update(ecsManager);
        }

    private:
        std::vector<SystemBase*> m_Systems;
    };

    template <typename TSystem>
    int32_t SystemsBuilder::GetSystemIndex()
    {
        // Slow but infrequently used
        const int32_t uniquenessValue = core::TemplatedUniquenessCounter<TSystem, SystemsBuilder>::GetTypeValue();
        for (size_t i = 0; i < m_Systems.size(); ++i)
        {
            if (m_Systems[i].m_UniquenessValue == uniquenessValue)
            {
                return static_cast<int32_t>(i);
            }
        }

        return -1;
    }

    inline int32_t SystemsBuilder::GetMinimumDependencyIndex(const int32_t uniquenessValue) const
    {
        for (size_t i = 0; i < m_Systems.size(); ++i)
        {
            for (const auto dependencyValue : m_Systems[i].m_Dependencies)
            {
                if (dependencyValue == uniquenessValue)
                {
                    return static_cast<int32_t>(i);
                }
            }
        }

        return static_cast<int32_t>(m_Systems.size());
    }

    inline int32_t SystemsBuilder::GetMaximumGroupIndex(const std::vector<GroupId> dependentGroups) const
    {
        int32_t outValue = -1;
        for (const auto& group : dependentGroups)
        {
            for (int32_t i = static_cast<int32_t>(m_Systems.size()) - 1; i >= 0; i--)
            {
                if (m_Systems[i].m_GroupId.m_GroupId == group.m_GroupId)
                {
                    outValue = std::max(outValue, i);
                }
            }
        }
        return outValue;
    }

    template <typename TSystem, typename ... DependsOn, typename... TArgs>
    void SystemsBuilder::RegisterSystem(TArgs&&... args)
    {
        RegisterSystem<TSystem, DependsOn..., TArgs...>({}, std::forward<TArgs&&>(args)...);
    }

    template <typename TSystem, typename ... DependsOn, typename... TArgs>
    void SystemsBuilder::RegisterSystem(std::vector<GroupId> dependentGroups, TArgs&&... args)
    {
        core::TemplatedUniquenessCounter<TSystem, SystemsBuilder>::Ensure();

        std::vector<int32_t> dependencyIndices{GetSystemIndex<DependsOn>()...};
        std::erase(dependencyIndices, -1);

        int32_t minimumIndex = dependencyIndices.empty() ? -1 : *std::ranges::max_element(dependencyIndices);
        minimumIndex = std::max(minimumIndex, GetMaximumGroupIndex(dependentGroups));

        const int32_t maximumIndex = GetMinimumDependencyIndex(
            core::TemplatedUniquenessCounter<TSystem, SystemsBuilder>::GetTypeValue());

        assert(minimumIndex < maximumIndex); // If we hit this, we have a circular dependency issue

        m_Systems.insert(
            m_Systems.begin() + maximumIndex,
            SystemRegistration(
                typeid(TSystem).name(),
                new TSystem(std::forward<TArgs&&>(args)...),
                {0},
                core::TemplatedUniquenessCounter<TSystem, SystemsManager>::Ensure(),
                {core::TemplatedUniquenessCounter<DependsOn, SystemsManager>::GetTypeValue()...}));
    }

    template <typename ... DependsOn>
    SystemsBuilder::GroupId SystemsBuilder::RegisterGroup(std::string groupName,
                                                          std::function<void(SystemsBuilder&)> callback)
    {
        return RegisterGroup<DependsOn...>(groupName, {}, callback);
    }

    template <typename ... DependsOn>
    SystemsBuilder::GroupId SystemsBuilder::RegisterGroup(std::string, std::vector<GroupId> dependentGroups,
                                                          std::function<void(SystemsBuilder&)> callback)
    {
        SystemsBuilder groupBuilder;
        callback(groupBuilder);
        if (groupBuilder.m_Systems.empty())
        {
            return {};
        }

        std::vector<int32_t> dependencyIndices{GetSystemIndex<DependsOn>()...};
        std::erase(dependencyIndices, -1);

        int32_t minimumIndex = dependencyIndices.empty() ? -1 : *std::ranges::max_element(dependencyIndices);
        minimumIndex = std::max(minimumIndex, GetMaximumGroupIndex(dependentGroups));

        int32_t maximumIndex = 0;
        for (const auto& registration : groupBuilder.m_Systems)
        {
            maximumIndex = std::max(maximumIndex, GetMinimumDependencyIndex(registration.m_UniquenessValue));
        }

        assert(minimumIndex < maximumIndex); // If we hit this, we have a circular dependency issue

        const GroupId groupId = {++m_LatestGroupId};
        for (const auto& registration : groupBuilder.m_Systems | std::views::reverse)
        {
            m_Systems.insert(
                m_Systems.begin() + maximumIndex,
                SystemRegistration(
                    registration.m_SystemName,
                    registration.m_pSystem,
                    groupId,
                    registration.m_UniquenessValue,
                    registration.m_Dependencies));
        }

        groupBuilder.m_Systems.clear();
        return groupId;
    }

    inline SystemsBuilder::SystemRegistration::SystemRegistration(
        std::string systemName,
        SystemBase* pSystem,
        GroupId groupId,
        const int32_t value,
        std::vector<int32_t> dependencies)
        : m_SystemName{std::move(systemName)}
          , m_pSystem{pSystem}
          , m_GroupId{groupId}
          , m_UniquenessValue{value}
          , m_Dependencies{std::move(dependencies)}
    {
    }
}
